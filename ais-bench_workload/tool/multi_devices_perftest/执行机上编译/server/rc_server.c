#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <malloc.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <time.h>
#include <inttypes.h>
#include <sys/mman.h>
// #include <numa.h>
#include <unistd.h>
#include <numa.h>
#include <numaif.h>
#include <unistd.h>
#include <sys/syscall.h>
// #include <linux/mempolicy.h>

#include "server.h"

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (a) < (b) ? (a) : (b)
// #define MAX_SERVERS 10
#define MAX_SERVERS 100000
#define MAX_COMMUNICATION_ITERS 100000
#define NUM_THREADS 128
int global_process_id;

enum {
    PINGPONG_RECV_WRID = 1,
    PINGPONG_SEND_WRID = 2,
};

static int page_size;
static int use_odp;
static int use_ts;
static int validate_buf;
static int send_num = 0;
static int send_cnt = 0;
static int recv_cnt = 1;
struct message {
    uint32_t type;
    uint32_t size;
};

struct pingpong_context {
    struct ibv_context *context;
    struct ibv_comp_channel *channel;
    struct ibv_pd *pd;
    struct ibv_mr **mr;
    struct ibv_cq **cq;
    struct ibv_qp **qp;
    char **buf;
    int size;
    int qp_num;
    int send_flags;
    int rx_depth;
    int pending;
    struct ibv_port_attr portinfo;
    uint64_t completion_timestamp_mask;
};

struct pingpong_dest {
    int lid;
    int qpn;
    int psn;
    union ibv_gid gid;
    int num_qps;
};

typedef struct thread_data {
    struct ibv_qp *qp;
    struct ibv_cq *cq;
    struct ibv_mr *mr;
    char *buf;
    int msg_size;
    int recv_size;
    int recv_num;
    int send_num;
    int send_flags;
    int thread_id;
}thread_data;


static struct ibv_cq *pp_cq(struct pingpong_context *ctx, int i) {
    return ctx->cq[i];
}

static int pp_post_recv(struct pingpong_context *ctx, int n, int qp_index) {
    recv_cnt += 2;
    struct ibv_sge list = {
        .addr = (uintptr_t)(ctx->buf[qp_index]),
        .length = ctx->size,
        .lkey = ctx->mr[qp_index]->lkey
    };
    struct ibv_recv_wr wr = {
        .wr_id = recv_cnt,
        .sg_list = &list,
        .num_sge = 1,
    };
    struct ibv_recv_wr *bad_wr;
    int i;
    for (i = 0; i < n; ++i)
        if(ibv_post_recv(ctx->qp[qp_index], &wr, &bad_wr)) break;
    return i;
}

static int pp_post_send(struct pingpong_context *ctx, int qp_index, int size) {
    send_cnt += 2;
    struct ibv_sge list = {
        .addr = (uintptr_t)(ctx->buf[qp_index]),
        .length = size,
        .lkey = ctx->mr[qp_index]->lkey
    };
    struct ibv_send_wr wr = {
        .wr_id = send_cnt,
        .sg_list = &list,
        .num_sge = 1,
        .opcode = IBV_WR_SEND,
        .send_flags = ctx->send_flags,
    };
    struct ibv_send_wr *bad_wr;
    return ibv_post_send(ctx->qp[qp_index], &wr, &bad_wr);
}


struct pingpong_context *pp_init_ctx(struct ibv_device *ib_dev, int size, int rx_depth, int port, int num_qps) {
   struct pingpong_context *ctx = calloc(1, sizeof *ctx);
    if (!ctx) {
        fprintf(stderr, "Failed to allocate pingpong_context\n");
        return NULL;
    }

    ctx->size = size;
    ctx->rx_depth = rx_depth;
    ctx->qp_num = num_qps;
    ctx->send_flags = IBV_SEND_SIGNALED;
    int alloc_size = max(size, 64);
    // ctx->buf = calloc(num_qps, sizeof(char*));

    ctx->buf = mmap(0, num_qps * sizeof(char*), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 ,0);
    printf("Page Size:%d\n", page_size);
    if (!ctx->buf) {
        fprintf(stderr, "Failed to allocate buffer pointers\n");
        goto clean_ctx;
    }
    int32_t mpolBind = 2;

    unsigned long nodeMask[2] = {0UL, 0UL}; // less than 128


    unsigned long bitsMaxNum = 128UL;
    // nodeMask[0] |= (1UL << (33 % (sizeof(unsigned long) * 8))); // 对于 64 位系统

    for (int node = 32; node <= 40; ++node) {
        // 计算在 nodeMask 中的偏移量
        int index = node / (sizeof(unsigned long) * 8);
        int shift = node % (sizeof(unsigned long) * 8);
        // 设置掩码
        nodeMask[index] |= (1UL << shift);
    }


    unsigned long maxNode = sizeof(unsigned long) * 8; // 每个掩码的位数

    int ret = set_mempolicy(mpolBind, nodeMask, bitsMaxNum);
    // int ret = set_mempolicy(mpolBind, nodeMask, maxNode);
    if (ret < 0) {
        perror("set_mempolicy failed");
        // 处理错误
    }



    printf("Alloc Size : %d\n", alloc_size);

    for (int i = 0; i < num_qps; ++i) {
        ctx->buf[i] = memalign(page_size, alloc_size);
        if (!ctx->buf[i]) {
        fprintf(stderr, "Failed to allocate buffer\n");
        goto clean_ctx;
        }
    }

    for (int i = 0; i < num_qps; ++i) memset(ctx->buf[i], 0x7b, size);

    ctx->context = ibv_open_device(ib_dev);
    if (!ctx->context) {
        fprintf(stderr, "Failed to open device\n");
        goto clean_buffer;
    }

    ctx->pd = ibv_alloc_pd(ctx->context);
    if (!ctx->pd) {
        fprintf(stderr, "Failed to allocate PD\n");
        goto clean_device;
    }

    ctx->mr = calloc(num_qps, sizeof(struct ibv_mr *));
    for (int i = 0; i < num_qps; ++i) {
        ctx->mr[i] = ibv_reg_mr(ctx->pd, ctx->buf[i], size, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);
        printf("Mem register success!\n");
        if (!ctx->mr[i]) {
            fprintf(stderr, "Failed to register MR\n");
            goto clean_pd;
        }
    }
    ctx->cq = calloc(num_qps, sizeof(struct ibv_cq *));
    ctx->qp = calloc(num_qps, sizeof(struct ibv_qp *));
    if (!ctx->cq || !ctx->qp) {
        fprintf(stderr, "Failed to allocate CQs or QPs array\n");
        goto clean_mr;
    }
    printf("Start initializing cq\n");
    for (int i = 0; i < num_qps; i++) {
        ctx->cq[i] = ibv_create_cq(ctx->context, rx_depth, NULL, ctx->channel, 0);
        if (!ctx->cq[i]) {
            fprintf(stderr, "Failed to create CQ\n");
            goto clean_cq_qp;
        }
        struct ibv_qp_attr attr;
        struct ibv_qp_init_attr init_attr = {
            .send_cq = ctx->cq[i],
            .recv_cq = ctx->cq[i],
            .cap = {
                .max_send_wr = rx_depth,
                .max_recv_wr = rx_depth,
                .max_send_sge = 1,
                .max_recv_sge = 1
            },
            .qp_type = IBV_QPT_RC
        };

        ctx->qp[i] = ibv_create_qp(ctx->pd, &init_attr);
        if (!ctx->qp[i]) {
            fprintf(stderr, "Failed to create QP\n");
            goto clean_cq_qp;
        }
        ibv_query_qp(ctx->qp[i], &attr, IBV_QP_CAP, &init_attr);
		if (init_attr.cap.max_inline_data >= size * num_qps) {
			ctx->send_flags |= IBV_SEND_INLINE;
		}
        {
            struct ibv_qp_attr attr = {
                .qp_state        = IBV_QPS_INIT,
                .pkey_index      = 0,
                .port_num        = port,
                .qp_access_flags = 0
            };

            if (ibv_modify_qp(ctx->qp[i], &attr,
                    IBV_QP_STATE              |
                    IBV_QP_PKEY_INDEX         |
                    IBV_QP_PORT               |
                    IBV_QP_ACCESS_FLAGS)) {
                fprintf(stderr, "Failed to modify QP to INIT\n");
                goto clean_cq_qp;
            }
        }
    }
    printf("Initializing cq end\n");

    return ctx;

clean_cq_qp:
    // Add code to clean up CQs and QPs if needed
    for (int i = 0; i < num_qps; i++) {
        if (ctx->qp[i]) ibv_destroy_qp(ctx->qp[i]);
        if (ctx->cq[i]) ibv_destroy_cq(ctx->cq[i]);
    }
clean_mr:
    for (int i = 0; i < num_qps; ++i)
    ibv_dereg_mr(ctx->mr[i]);
clean_pd:
    ibv_dealloc_pd(ctx->pd);
clean_device:
    ibv_close_device(ctx->context);
clean_buffer:
    for (int i = 0; i < num_qps; ++i) free(ctx->buf[i]);
clean_ctx:
    free(ctx);
    return NULL;
}


void pp_close_ctx(struct pingpong_context *ctx) {
    for (int i = 0; i < ctx->qp_num; i++) {
        if (ctx->qp[i]) {
            ibv_destroy_qp(ctx->qp[i]);
        }
        if (ctx->cq[i]) {
            ibv_destroy_cq(ctx->cq[i]);
        }
    }
    for (int i = 0; i < ctx->qp_num; i ++) {
        if (ctx->mr[i] && ibv_dereg_mr(ctx->mr[i])) {
            fprintf(stderr, "Failed to deregister MR\n");
            exit(1);
        }
    }
    for (int i = 0; i < ctx->qp_num; ++i) free(ctx->buf[i]);
    ibv_dealloc_pd(ctx->pd);
    ibv_close_device(ctx->context);
    free(ctx->cq);
    free(ctx->qp);
    free(ctx);
}


int setup_server_socket(unsigned int port) {
    struct addrinfo hints, *res = NULL, *t;
    int sockfd, rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    char *str_port;
    if (asprintf(&str_port, "%d", port) < 0)
		return NULL;
    rc = getaddrinfo(NULL, str_port, &hints, &res);

    if (rc != 0) {
        fprintf(stderr, "getaddrinfo failed (%s)\n", gai_strerror(rc));
        exit(EXIT_FAILURE);
    }

    for (t = res; t; t = t->ai_next) {
        sockfd = socket(t->ai_family, t->ai_socktype, t->ai_protocol);
        if (sockfd >= 0) {
            int optval = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
            if (!bind(sockfd, t->ai_addr, t->ai_addrlen) && !listen(sockfd, 10)) {
                break;  // Success
            }
            close(sockfd);
            sockfd = -1;
        }
    }


    freeaddrinfo(res);

    if (sockfd < 0) {
        fprintf(stderr, "Could not setup server socket\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}


int accept_client_connection(int server_sock) {
    struct sockaddr_in6 client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

    if (client_sock < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // 可选：打印客户端地址
    char client_ip[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip, sizeof(client_ip))) {
        printf("Client connected: %s\n", client_ip);
    }

    return client_sock;
}


static int pp_connect_ctx(struct pingpong_context *ctx, int port, int my_psn,
			  enum ibv_mtu mtu, int sl,
			  struct pingpong_dest *dest, int sgid_idx, int qp_index)
{
	struct ibv_qp_attr attr = {
		.qp_state		= IBV_QPS_RTR,
		.path_mtu		= mtu,
		.dest_qp_num	= dest[qp_index].qpn,
		.rq_psn			= dest[qp_index].psn,
		.max_dest_rd_atomic	= 1,
		.min_rnr_timer		= 12,
		.ah_attr		= {
			.is_global	= 0,
			.dlid		= dest[qp_index].lid,
			.sl		    = sl,
			.src_path_bits	= 0,
			.port_num	= port
		}
	};

	if (dest[qp_index].gid.global.interface_id) {
		attr.ah_attr.is_global = 1;
		attr.ah_attr.grh.hop_limit = 1;
		attr.ah_attr.grh.dgid = dest[qp_index].gid;
		attr.ah_attr.grh.sgid_index = sgid_idx;
	}
	if (ibv_modify_qp(ctx->qp[qp_index], &attr,
			  IBV_QP_STATE              |
			  IBV_QP_AV                 |
			  IBV_QP_PATH_MTU           |
			  IBV_QP_DEST_QPN           |
			  IBV_QP_RQ_PSN             |
			  IBV_QP_MAX_DEST_RD_ATOMIC |
			  IBV_QP_MIN_RNR_TIMER)) {
		fprintf(stderr, "Failed to modify QP to RTR\n");
		return 1;
	}

	attr.qp_state	    = IBV_QPS_RTS;
	attr.timeout	    = 14;
	attr.retry_cnt	    = 7;
	attr.rnr_retry	    = 7;
	attr.sq_psn	    = my_psn;
	attr.max_rd_atomic  = 1;
	if (ibv_modify_qp(ctx->qp[qp_index], &attr,
			  IBV_QP_STATE              |
			  IBV_QP_TIMEOUT            |
			  IBV_QP_RETRY_CNT          |
			  IBV_QP_RNR_RETRY          |
			  IBV_QP_SQ_PSN             |
			  IBV_QP_MAX_QP_RD_ATOMIC)) {
		fprintf(stderr, "Failed to modify QP to RTS\n");
		return 1;
	}
    printf("Modify qp to RTS success\n");
	return 0;
}

void* push_thread_function(void* arg) {
    thread_data* dd = (thread_data*)arg;
    // pid_t tid = gettid();
    // 绑定线程到特定的 CPU
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    // CPU_SET(dd->thread_id % CPU_SETSIZE, &cpuset);  // 假设你的 thread_data 结构中有 thread_id 字段
    CPU_SET((dd->thread_id + global_process_id)% 8, &cpuset);
    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        return (void*)-1;
    }
    // printf("Thread %d is set to run on CPU %d.\n", dd->thread_id, dd->thread_id);

    struct ibv_wc wc;
    int num_wc = 0;


//接受信息

    while (1) {
        if (ibv_poll_cq(dd->cq, 1, &wc) == 0) continue;
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
            ibv_wc_status_str(wc.status), wc.status, (int)wc.wr_id);
            return (void*)-1;
        }
        if ((int)wc.wr_id & 1) {
            dd->recv_num += 2;
            struct ibv_sge list = {
                .addr = (uintptr_t)(dd->buf),
                .length = dd->recv_size,
                .lkey = dd->mr->lkey
            };
            struct ibv_recv_wr recv_wr = {
                .wr_id = dd->recv_num,
                .sg_list = &list,
                .num_sge = 1,
            };
            struct ibv_recv_wr *recv_bad_wr;
            if(ibv_post_recv(dd->qp, &recv_wr, &recv_bad_wr)) {
                fprintf(stderr, "Failed to repost recv WR\n");
                return (void*)-1;
            }
            break;
        }
    }


    return NULL;
}


void* thread_function(void* arg) {
    thread_data* dd = (thread_data*)arg;
    // pid_t tid = gettid();
    // 绑定线程到特定的 CPU
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    // CPU_SET(dd->thread_id % CPU_SETSIZE, &cpuset);  // 假设你的 thread_data 结构中有 thread_id 字段
    CPU_SET((dd->thread_id + global_process_id) % 8, &cpuset);
    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        return (void*)-1;
    }
    // printf("Thread %d is set to run on CPU %d.\n", dd->thread_id, dd->thread_id);

    struct ibv_send_wr *send_bad_wr;
    struct ibv_wc wc;
    int num_wc = 0;
    struct ibv_sge list = {
        .addr = (uintptr_t)(dd->buf),
        .length = dd->msg_size,
        .lkey = dd->mr->lkey
    };
    struct ibv_send_wr send_wr = {
        .wr_id = dd->send_num,
        .sg_list = &list,
        .num_sge = 1,
        .opcode = IBV_WR_SEND,
        .send_flags = dd->send_flags,
    };

//接受信息

    while (1) {
        if (ibv_poll_cq(dd->cq, 1, &wc) == 0) continue;
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
            ibv_wc_status_str(wc.status), wc.status, (int)wc.wr_id);
            return (void*)-1;
        }
        if ((int)wc.wr_id & 1) {
            dd->recv_num += 2;
            struct ibv_sge list = {
                .addr = (uintptr_t)(dd->buf),
                .length = dd->recv_size,
                .lkey = dd->mr->lkey
            };
            struct ibv_recv_wr recv_wr = {
                .wr_id = dd->recv_num,
                .sg_list = &list,
                .num_sge = 1,
            };
            struct ibv_recv_wr *recv_bad_wr;
            if(ibv_post_recv(dd->qp, &recv_wr, &recv_bad_wr)) {
                fprintf(stderr, "Failed to repost recv WR\n");
                return (void*)-1;
            }
            break;
        }
    }
    // printf('send\n');
    dd->send_num += 2;
    if (ibv_post_send(dd->qp, &send_wr, &send_bad_wr)) {
        fprintf(stderr, "Couldn't post send\n");
        return (void*)-1;
    } // 假设消息大小为32字节

    return NULL;
}

int main(int argc, char *argv[]) {
    global_process_id = getpid();
    struct ibv_device *ib_dev = NULL;
    struct pingpong_context *ctx;
    int ib_port = 1;
    int num_clients = 1;
    char *dev_name = NULL;
	struct ibv_device **dev_list;
    char *ib_devname = NULL;
    unsigned int port = 18515;
    unsigned int size = 4096;
    enum ibv_mtu mtu = IBV_MTU_1024;
    unsigned int rx_depth = 500;
    unsigned int iters = 1000;
    int num_cq_events = 0;
    int sl = 0;
    int gidx = -1;
    int qps_per_client = 1;
    int start_size = 512;
    int end_size = 128 * 1024 * 1024;
    // Parse command line arguments
    int opt;
    char *mode = "pull"; // 默认为pull

    srand48(getpid() * time(NULL));
    while (1) {
        int c;
        static struct option long_options[] = {
            { .name = "port",     .has_arg = 1, .val = 'p' },
            { .name = "ib-dev",   .has_arg = 1, .val = 'd' },
            { .name = "ib-port",  .has_arg = 1, .val = 'i' },
            { .name = "size",     .has_arg = 1, .val = 's' },
            { .name = "mtu",      .has_arg = 1, .val = 'm' },
            { .name = "rx-depth", .has_arg = 1, .val = 'r' },
            { .name = "iters",    .has_arg = 1, .val = 'n' },
            { .name = "sl",       .has_arg = 1, .val = 'l' },
            { .name = "gid-idx",  .has_arg = 1, .val = 'g' },
            { .name = "qps-per-client",  .has_arg = 1, .val = 'q' },
            { .name = "num-client",  .has_arg = 1, .val = 'z' },
            { .name = "start-size",  .has_arg = 1, .val = 'a' },
            { .name = "end-size",  .has_arg = 1, .val = 'b' },
            { .name = "mode", .has_arg = 1, .val = 'M' },
            { 0 }
        };
        c = getopt_long(argc, argv, "p:d:i:s:m:r:n:l:g:q:z:M:b:", long_options, NULL);
        if (c == -1)
            break;

        switch (c) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 'd':
            ib_devname = strdup(optarg);
            break;
        case 'i':
            ib_port = strtol(optarg, NULL, 0);
            break;
        case 's':
            size = strtoul(optarg, NULL, 0);
            break;
        case 'm':
            mtu = strtol(optarg, NULL, 0);
            break;
        case 'r':
            rx_depth = strtoul(optarg, NULL, 0);
            break;
        case 'n':
            iters = strtoul(optarg, NULL, 0);
            break;
        case 'l':
            sl = strtol(optarg, NULL, 0);
            break;
        case 'g':
            gidx = strtol(optarg, NULL, 0);
            break;
        case 'q':
            qps_per_client = strtol(optarg, NULL, 0);
            break;
        case 'z':
            num_clients = strtol(optarg, NULL, 0);
            printf("Num of clients:%d\n", num_clients);
            break;
        case 'a':
            start_size = strtol(optarg, NULL, 0);
            break;
        case 'b':
            end_size = strtol(optarg, NULL, 0);
            break;
        case 'M':
            mode = strdup(optarg);
            if (strcmp(mode, "push") != 0 && strcmp(mode, "pull") != 0 && strcmp(mode, "all") != 0
            && strcmp(mode, "multipush") != 0 && strcmp(mode, "multipull") != 0 && strcmp(mode, "multi") != 0
            && strcmp(mode, "allmulti") != 0) {
            fprintf(stderr, "Invalid mode. Use 'push', 'pull', 'all', 'multipush', "\
                    "'multipull', 'multi', and 'allmulti'.\n");
                return 1;
            }
            break;
        default:
            fprintf(stderr, "Usage: %s\n", argv[0]);
            return 1;
        }
    }

    // Initialization: Device, context, PD, MR, CQ, QP
    // Exchange connection data with clients
    // Main loop: Post receive, poll for completion, send response
    // Cleanup

    page_size = sysconf(_SC_PAGESIZE);
	dev_list = ibv_get_device_list(NULL);
	if (!dev_list) {
		perror("Failed to get IB devices list");
		exit(EXIT_FAILURE);
	}

	if (dev_name) {
		for (int i = 0; dev_list[i]; ++i)
			if (!strcmp(ibv_get_device_name(dev_list[i]), dev_name)) {
				ib_dev = dev_list[i];
				break;
			}
	} else {
		ib_dev = dev_list[0];
	}
    if ((strcmp(mode, "multipush") == 0) ||(strcmp(mode, "multipull") == 0) || (strcmp(mode, "multi") == 0) || (strcmp(mode, "allmulti") == 0)){
        const char* cgroup_path = "/sys/fs/cgroup/cpuset/myapp/tasks";
        FILE* fp = fopen(cgroup_path, "w");

        if (fp == NULL) {
            perror("Failed to open cgroup tasks file");
            return EXIT_FAILURE;
        }

        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }
	if (!ib_dev) {
		fprintf(stderr, "IB device not found\n");
		exit(EXIT_FAILURE);
	}
    int num_qps = num_clients * qps_per_client;

	ctx = pp_init_ctx(ib_dev, end_size, rx_depth, ib_port, num_clients * qps_per_client);
	if (!ctx) {
		fprintf(stderr, "Couldn't create context\n");
		exit(EXIT_FAILURE);
	}
    printf("Post recv start\n");
    for (int j = 0; j < ctx->qp_num; ++j) {
		 if (pp_post_recv(ctx, 1, j) != 1) {
            fprintf(stderr, "Failed to post recv\n");
            exit(1);
        } // 为每个QP投递rx_depth个接收请求
	}

    if (pp_get_port_info(ctx->context, ib_port, &ctx->portinfo)) {
		fprintf(stderr, "Couldn't get port info\n");
		return 1;
	}

    struct pingpong_dest *my_dests = calloc(num_qps, sizeof(struct pingpong_dest));
    struct pingpong_dest *rem_dests = calloc(num_qps, sizeof(struct pingpong_dest));
    if (!my_dests || !rem_dests) {
        fprintf(stderr, "Failed to allocate memory for destination information\n");
        exit(EXIT_FAILURE);
    }

    char gid[INET6_ADDRSTRLEN];
    for (int i = 0; i < num_qps; ++i) {
        my_dests[i].lid = ctx->portinfo.lid;
        if (gidx >= 0) {
            if (ibv_query_gid(ctx->context, ib_port, gidx, &my_dests[i].gid)) {
                fprintf(stderr, "Can't read GID of index %d for server %d\n", gidx, i);
                // 错误处理
            }
        } else {
            memset(&my_dests[i].gid, 0, sizeof(my_dests[i].gid));
        }

		my_dests[i].qpn = ctx->qp[i]->qp_num; // 注意：每个qp[i]对应不同的服务器
		my_dests[i].psn = lrand48() & 0xffffff;

        inet_ntop(AF_INET6, &my_dests[i].gid, gid, sizeof gid);
	}
    printf("GID:%s\n", gid);

    char ip_str[INET_ADDRSTRLEN];  // 用于 IPv4 地址
    for (int i = 0; i < num_qps; ++i) {
        my_dests[i].lid = ctx->portinfo.lid;
        if (gidx >= 0) {
            if (ibv_query_gid(ctx->context, ib_port, gidx, &my_dests[i].gid)) {
                fprintf(stderr, "Can't read GID of index %d for server %d\n", gidx, i);
                // 错误处理
            }
        } else {
            memset(&my_dests[i].gid, 0, sizeof(my_dests[i].gid));
        }

        my_dests[i].qpn = ctx->qp[i]->qp_num;  // 注意：每个qp[i]对应不同的服务器
        my_dests[i].psn = lrand48() & 0xffffff;

        // 假设我们有一个有效的 IPv4 地址存储在 my_dests[i].gid 中
        // 需要确保这里的地址转换逻辑与你的具体用例相符
        // IPv4 地址通常不直接存储在 gid 结构中，这里仅为示例
        inet_ntop(AF_INET, &my_dests[i].gid, ip_str, sizeof ip_str);
    }
    printf("IP:%s\n", ip_str);


	// Assume the existence of a setup function that prepares
	// server socket to accept connections from clients
	int server_sock = setup_server_socket(port);

	for (int i = 0; i < num_clients; ++i) {
		int client_sockfd = accept_client_connection(server_sock);
		// Exchange connection data for each QP
		for (int j = 0; j < qps_per_client; ++j) {
            struct pingpong_dest *local_dest = &my_dests[j + i * qps_per_client];
            struct pingpong_dest *remote_dest = &rem_dests[j + i * qps_per_client];
            char msg[sizeof "0000:000000:000000:00000000000000000000000000000000"];
            char gid[33];


            if (read(client_sockfd, msg, sizeof(msg)) != sizeof(msg)) {
                fprintf(stderr, "Failed to receive remote connection data\n");
                exit(EXIT_FAILURE);
            }

            sscanf(msg, "%x:%x:%x:%s", &remote_dest->lid, &remote_dest->qpn, &remote_dest->psn, gid);
            wire_gid_to_gid(gid, &remote_dest->gid);
            printf("Received Remote Destination Info:\n");
            printf(" LID: 0x%x\n", remote_dest->lid);
            printf(" QPN: 0x%x\n", remote_dest->qpn);
            printf(" PSN: 0x%x\n", remote_dest->psn);
            printf(" GID: %s\n", gid);

            if (pp_connect_ctx(ctx, ib_port, my_dests[j + i * qps_per_client].psn, mtu, sl, rem_dests, gidx, j + i * qps_per_client)) {
                fprintf(stderr, "Failed to connect QPs\n");
                return 1;
            }
            gid_to_wire_gid(&local_dest->gid, gid);
            sprintf(msg, "%04x:%06x:%06x:%s", local_dest->lid, local_dest->qpn,
                                    local_dest->psn, gid);
            if (write(client_sockfd, msg, sizeof msg) != sizeof msg ||
                read(client_sockfd, msg, sizeof msg) != sizeof "done") {
                fprintf(stderr, "Couldn't send/recv local address\n");
                free(remote_dest);
                remote_dest = NULL;
                close(client_sockfd);
                exit(EXIT_FAILURE);
            }
        }
        close(client_sockfd);
	}
    printf("Start Iterations\n");
    int ne = 0;
    struct ibv_wc wc[2];
	if ((strcmp(mode, "pull") == 0) || (strcmp(mode, "all") == 0) || (strcmp(mode, "allmulti") == 0) ) {
    //pull
    for (int ss = start_size; ss <= end_size / 2; ss = ss << 1) {
        printf("Current Size:%d\n", ss);

        for (int i = 0; i < iters; ++i) {
            int num_wc = 0;
            while (num_wc < ctx->qp_num) {
                for (int j = 0; j < ctx->qp_num; ++j) {
                    struct ibv_wc wc[1001];
                    do {
                        ne = ibv_poll_cq(pp_cq(ctx, j), 1, wc);
                        if (ne < 0) {
                            fprintf(stderr, "Poll CQ failed\n");
                            exit(1);
                        }
                    } while (ne <= 0);
                    for (int ii = 0; ii < ne; ++ii) {
                        if (wc[ii].status != IBV_WC_SUCCESS) {
                            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
                            ibv_wc_status_str(wc[ii].status), wc[ii].status, (int)wc[ii].wr_id);
                            exit(1);
                        }
                        if ((int)wc[ii].wr_id & 1) {
                            pp_post_send(ctx, j, ss);
                            ++num_wc;
                            if (pp_post_recv(ctx, 1, j) != 1) {
                                fprintf(stderr, "Failed to repost recv WR\n");
                                exit(1);
                            }
                        }
                    }
                }
            }
        }



    }

    }
    if (strcmp(mode, "push") == 0 || (strcmp(mode, "all") == 0) || (strcmp(mode, "allmulti") == 0) ){
    //push
    for (int ss = start_size; ss <= end_size / 2; ss = ss << 1) {
        printf("Current Size:%d\n", ss);
        for (int i = 0; i < iters; ++i) {
            int num_wc = 0;
            while (num_wc < ctx->qp_num) {
                for (int j = 0; j < ctx->qp_num; ++j) {

                    do {
                        ne = ibv_poll_cq(pp_cq(ctx, j), 1, wc);
                        if (ne < 0) {
                            fprintf(stderr, "Poll CQ failed\n");
                            exit(1);
                        }
                    } while (ne <= 0);
                    for (int ii = 0; ii < ne; ++ii) {
                        if (wc[ii].status != IBV_WC_SUCCESS) {
                            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
                            ibv_wc_status_str(wc[ii].status), wc[ii].status, (int)wc[ii].wr_id);
                            exit(1);
                        }
                        if ((int)wc[ii].wr_id & 1) {
                            // pp_post_send(ctx, j, size);
                            ++num_wc;
                            if (pp_post_recv(ctx, 1, j) != 1) {
                                fprintf(stderr, "Failed to repost recv WR\n");
                                exit(1);
                            }
                        }
                    }



                }
            }
        }
    }

    }

    if (strcmp(mode, "multipull") == 0 || (strcmp(mode, "multi") == 0) || (strcmp(mode, "allmulti") == 0)){
        //multipull
    for (int ss = start_size; ss <= end_size / 2; ss = ss << 1) {
        printf("Current Size:%d\n", ss);

        int udx = -1;
        pthread_t threads[NUM_THREADS];
        thread_data data[NUM_THREADS];
        void *status;
        for (int round = 0; round < iters; ++round) {
                // printf("Round:%d\n", round);
                // 记录开始时间
                for (int i = 0; i < num_qps; ++i) {
                    data[i].thread_id = i;
                    data[i].qp = ctx->qp[i];
                    data[i].cq = ctx->cq[i];
                    data[i].mr = ctx->mr[i];
                    data[i].buf = ctx->buf[i];
                    data[i].msg_size = ss;
                    data[i].recv_size = end_size;
                    data[i].send_num = 0;
                    data[i].recv_num = 1;
                    data[i].send_flags = ctx->send_flags;
                }
                for (int i = 0; i < num_qps; ++i) {
                    if (pthread_create(&threads[i], NULL, thread_function, (void*)&data[i])) {
                        printf("Error: pthread create failed\n");
                        exit(1);
                    }
                }

                for (int i = 0; i < num_qps; ++i) {
                    pthread_join(threads[i], &status);
                    if ((long)status == -1) {
                        printf("Thread #%d exited with error\n", i);
                        exit(1);
                    }
                }

        }
    }

    }

    if (strcmp(mode, "multipush") == 0 || (strcmp(mode, "multi") == 0) || (strcmp(mode, "allmulti") == 0)){
        //multipull
    for (int ss = start_size; ss <= end_size / 2; ss = ss << 1) {
        printf("Current Size:%d\n", ss);

        int udx = -1;
        pthread_t threads[NUM_THREADS];
        thread_data data[NUM_THREADS];
        void *status;
        for (int round = 0; round < iters; ++round) {
                // printf("Round:%d\n", round);
                // 记录开始时间
                for (int i = 0; i < num_qps; ++i) {
                    data[i].thread_id = i;
                    data[i].qp = ctx->qp[i];
                    data[i].cq = ctx->cq[i];
                    data[i].mr = ctx->mr[i];
                    data[i].buf = ctx->buf[i];
                    data[i].msg_size = ss;
                    data[i].recv_size = end_size;
                    data[i].send_num = 0;
                    data[i].recv_num = 1;
                    data[i].send_flags = ctx->send_flags;
                }
                for (int i = 0; i < num_qps; ++i) {
                    if (pthread_create(&threads[i], NULL, push_thread_function, (void*)&data[i])) {
                        printf("Error: pthread create failed\n");
                        exit(1);
                    }
                }

                for (int i = 0; i < num_qps; ++i) {
                    pthread_join(threads[i], &status);
                    if ((long)status == -1) {
                        printf("Thread #%d exited with error\n", i);
                        exit(1);
                    }
                }

        }
    }

    }

	pp_close_ctx(ctx);
	close(server_sock);
	ibv_free_device_list(dev_list);
}