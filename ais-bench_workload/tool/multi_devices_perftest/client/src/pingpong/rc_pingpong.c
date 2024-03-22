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

#include "pingpong.h"

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (a) < (b) ? (a) : (b)
#define MAX_SERVERS 100000
#define MAX_COMMUNICATION_ITERS 100000
#define NUM_THREADS 128

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;

enum {
    PINGPONG_RECV_WRID = 1,
    PINGPONG_SEND_WRID = 2,
};

static int page_size;
static int use_odp;
static int use_ts;
static int validate_buf;
static int recv_cnt = 1;
static int send_cnt = 0;
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
    struct timespec startimes;
    struct timespec endtimes;
}thread_data;

// pid_t gettid() {
//     return syscall(SYS_gettid);
// }

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


static int pp_connect_ctx(struct pingpong_context *ctx, int port, int my_psn, enum ibv_mtu mtu, int sl, struct pingpong_dest *dest, int sgid_idx, int qp_index) {
    struct ibv_qp_attr attr = {
        .qp_state        = IBV_QPS_RTR,
        .path_mtu        = mtu,
        .dest_qp_num     = dest[qp_index].qpn,
        .rq_psn          = dest[qp_index].psn,
        .max_dest_rd_atomic = 1,
        .min_rnr_timer   = 12,
        .ah_attr = {
            .is_global      = 0,
            .dlid           = dest[qp_index].lid,
            .sl             = sl,
            .src_path_bits  = 0,
            .port_num       = port,
        },
    };

    if (dest[qp_index].gid.global.interface_id) {
        attr.ah_attr.is_global = 1;
        attr.ah_attr.grh.dgid = dest[qp_index].gid;
        attr.ah_attr.grh.sgid_index = sgid_idx;
        attr.ah_attr.grh.hop_limit = 1;
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
    attr.qp_state       = IBV_QPS_RTS;
    attr.timeout        = 14;
    attr.retry_cnt      = 7;
    attr.rnr_retry      = 7; /* Infinite retry */
    attr.sq_psn         = my_psn;
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

    return 0;
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
    printf("Mem alloc success!\n");
    for (int i = 0; i < num_qps; ++i) {memset(ctx->buf[i], 0x7b, alloc_size);printf("Progress %d / %d\n", i, num_qps);}
    printf("Initialize Buffer success\n");

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
    printf("PD access success\n");
    ctx->mr = calloc(num_qps, sizeof(struct ibv_mr *));
    for (int i = 0; i < num_qps; ++i) {
        ctx->mr[i] = ibv_reg_mr(ctx->pd, ctx->buf[i], size, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);
        printf("Mem register success!\n");
        if (!ctx->mr[i]) {
            fprintf(stderr, "Failed to register MR\n");
            goto clean_pd;
        }
    }
    printf("MR register success!\n");
    ctx->cq = calloc(num_qps, sizeof(struct ibv_cq *));
    ctx->qp = calloc(num_qps, sizeof(struct ibv_qp *));
    if (!ctx->cq || !ctx->qp) {
        fprintf(stderr, "Failed to allocate CQs or QPs array\n");
        goto clean_mr;
    }

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


int pp_close_ctx(struct pingpong_context *ctx) {
    int ret = 0;

    for (int i = 0; i < ctx->qp_num; i++) {
        if (ctx->qp[i] && ibv_destroy_qp(ctx->qp[i])) {
            fprintf(stderr, "Failed to destroy QP\n");
            ret = 1;
        }
    }
    for (int i = 0; i < ctx->qp_num; i ++) {
        if (ctx->mr[i] && ibv_dereg_mr(ctx->mr[i])) {
            fprintf(stderr, "Failed to deregister MR\n");
            ret = 1;
        }
    }


    if (ctx->pd && ibv_dealloc_pd(ctx->pd)) {
        fprintf(stderr, "Failed to deallocate PD\n");
        ret = 1;
    }

    if (ctx->context && ibv_close_device(ctx->context)) {
        fprintf(stderr, "Failed to close device context\n");
        ret = 1;
    }

     for (int i = 0; i < ctx->qp_num; ++i) free(ctx->buf[i]);
    free(ctx->cq);
    free(ctx->qp);
    free(ctx);

    return ret;
}

int establish_ipv6_connection(const char *server_ipv6_addr, const int server_port) {
    struct addrinfo *res, *t;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	char *service;
    int sockfd = -1;
    int n;
    if (asprintf(&service, "%d", server_port) < 0)
		return NULL;
    n = getaddrinfo(server_ipv6_addr, service, &hints, &res);

    if (n != 0) {
		fprintf(stderr, "%s for %s:%d\n", gai_strerror(n), server_ipv6_addr, server_port);
		free(service);
		return NULL;
	}
    for (t = res; t; t = t->ai_next) {
		sockfd = socket(t->ai_family, t->ai_socktype, t->ai_protocol);
		if (sockfd >= 0) {
			if (!connect(sockfd, t->ai_addr, t->ai_addrlen))
				break;
			close(sockfd);
			sockfd = -1;
		}
	}
    freeaddrinfo(res);
	free(service);
    if (sockfd < 0) {
		fprintf(stderr, "Couldn't connect to %s:%d\n", server_ipv6_addr, server_port);
		return NULL;
	}
    return sockfd;
}


double tmp[MAX_COMMUNICATION_ITERS] = {0};
void merge_sort(double q[], int l, int r)
{
    if (l >= r) return;

    int mid = l + r >> 1;
    merge_sort(q, l, mid);
    merge_sort(q, mid + 1, r);

    int k = 0, i = l, j = mid + 1;
    while (i <= mid && j <= r)
        if (q[i] <= q[j]) tmp[k ++ ] = q[i ++ ];
        else tmp[k ++ ] = q[j ++ ];

    while (i <= mid) tmp[k ++ ] = q[i ++ ];
    while (j <= r) tmp[k ++ ] = q[j ++ ];

    for (i = l, j = 0; i <= r; i ++, j ++ ) q[i] = tmp[j];
}

void* push_thread_function(void* arg) {
    thread_data* dd = (thread_data*)arg;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(dd->thread_id % 8, &cpuset);
    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        return (void*)-1;
    }


    dd->send_num += 2;
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
    struct ibv_send_wr *send_bad_wr;
    struct ibv_wc wc;
    int num_wc = 0;
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    clock_gettime(CLOCK_MONOTONIC, &dd->startimes);
    if (ibv_post_send(dd->qp, &send_wr, &send_bad_wr)) {
        fprintf(stderr, "Couldn't post send\n");
        return (void*)-1;
    } // 假设消息大小为32字节

    while (1) {
        if (ibv_poll_cq(dd->cq, 1, &wc) == 0) continue;
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
            ibv_wc_status_str(wc.status), wc.status, (int)wc.wr_id);
            return (void*)-1;
        }
        break;
    }

    clock_gettime(CLOCK_MONOTONIC, &dd->endtimes);
    return NULL;
}

void* thread_function(void* arg) {
    thread_data* dd = (thread_data*)arg;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(dd->thread_id % 8, &cpuset);
    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        return (void*)-1;
    }

    dd->send_num += 2;
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
    struct ibv_send_wr *send_bad_wr;
    struct ibv_wc wc;
    int num_wc = 0;
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    clock_gettime(CLOCK_MONOTONIC, &dd->startimes);
    if (ibv_post_send(dd->qp, &send_wr, &send_bad_wr)) {
        fprintf(stderr, "Couldn't post send\n");
        return (void*)-1;
    } // 假设消息大小为32字节

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

    clock_gettime(CLOCK_MONOTONIC, &dd->endtimes);
    return NULL;
}
int main(int argc, char *argv[]) {
    // 你的程序逻辑

    // sleep(14);
    struct ibv_device **dev_list;
    struct ibv_device *ib_dev;
    struct pingpong_context *ctx;
    char                    *servername = NULL;
    char                *ib_devname = NULL;
    char 					*server_addresses[MAX_SERVERS];
    struct timespec start, end;
    unsigned int port = 18515;
    int ib_port = 1;
    int num_servers = 0;
    unsigned int size = 4096;
    enum ibv_mtu mtu = IBV_MTU_1024;
    unsigned int rx_depth = 500;
    unsigned int iters = 1000;
    int num_cq_events = 0;
    int sl = 0;
    int gidx = -1;
    int qps_per_server = 1; // Assume this is set somewhere or passed as a command line argument
    int start_size = 512;
    int end_size =  128 * 1024 * 1024;
    char *mode = "pull"; // 默认为pull

    double result[MAX_COMMUNICATION_ITERS];

    srand48(getpid() * time(NULL));
    while (1) {
        int c;
        static struct option long_options[] = {
            { .name = "port",     .has_arg = 1, .val = 'p' },
            { .name = "ib-dev",   .has_arg = 1, .val = 'd' },
            { .name = "ib-port",  .has_arg = 1, .val = 'i' },
            { .name = "size",     .has_arg = 1, .val = 's' },
            { .name = "mtu",      .has_arg = 1, .val = 'm' }, //暂不支持该参数
            { .name = "rx-depth", .has_arg = 1, .val = 'r' },
            { .name = "iters",    .has_arg = 1, .val = 'n' },
            { .name = "sl",       .has_arg = 1, .val = 'l' },
            { .name = "gid-idx",  .has_arg = 1, .val = 'g' },
            { .name = "qps_per_server",  .has_arg = 1, .val = 'q' },
            { .name = "start-size",  .has_arg = 1, .val = 'a' },
            { .name = "end-size",  .has_arg = 1, .val = 'b' },
            { .name = "mode", .has_arg = 1, .val = 'M' },
            { 0 }
        };
        c = getopt_long(argc, argv, "p:d:i:s:m:r:n:l:g:q:M:a:b:", long_options, NULL);
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
            qps_per_server = strtol(optarg, NULL, 0);
            break;
        case 'a':
            start_size = strtol(optarg, NULL, 0);
            break;
        case 'b':
            end_size = strtol(optarg, NULL, 0);
            break;
        case 'M':
            mode = strdup(optarg);//目前多线程功能还不稳定，不建议使用
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

    if (optind == argc - 1){
		servername = strdupa(argv[optind]);
		char *token = strtok(servername, ",");
		while (token != NULL) {
        	server_addresses[num_servers++] = strdupa(token);
        	token = strtok(NULL, ",");
    	}
        for (int i = 0; i < num_servers; ++i) {
			printf("Server %d: %s\n", i, server_addresses[i]);
			// 在这里为每个服务器地址建立连接...
		}
	}
	else if (optind < argc) {
        printf("Error Params\n");
		return 1;
	}
    // 初始化上下文，创建QP等
    // ...
    // 接收逻辑，基于num_qps和服务器数量处理接收到的消息
    // ...
    page_size = sysconf(_SC_PAGESIZE);
    dev_list = ibv_get_device_list(NULL);
    if (!dev_list) {
        perror("Failed to get IB devices list");
        exit(1);
    }
    printf("ibv dev get success!\n");
    // 选择要使用的设备
    if (ib_devname) {
        for (int i = 0; dev_list[i]; ++i) {
            if (!strcmp(ibv_get_device_name(dev_list[i]), ib_devname)) {
                ib_dev = dev_list[i];
                break;
            }
        }
    } else {
        ib_dev = dev_list[0]; // 默认选择第一个设备
    }

    if (!ib_dev) {
        fprintf(stderr, "IB device %s not found\n", ib_devname ? ib_devname : "not specified");
        exit(1);
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
    printf("Start initial ctx\n");
    int num_qps = qps_per_server * num_servers;
    ctx = pp_init_ctx(ib_dev, end_size, rx_depth, ib_port, num_qps);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize context\n");
        return 1;
    }
    printf("Init ctx success!\n");
    for (int i = 0; i < ctx->qp_num; ++i) {
        if (pp_post_recv(ctx, ctx->rx_depth, i) != ctx->rx_depth) {
            fprintf(stderr, "Failed to post recv\n");
            exit(1);
        } // 为每个QP投递rx_depth个接收请求
    }
    printf("Post recv success!\n");
    // 假定结构和之前定义的函数等已经准备好
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
    printf("Init My dests success!\n");
    // 建立到每个服务器的TCP连接并交换RDMA连接信息
    for (int server_idx = 0; server_idx < num_servers; ++server_idx) {
        int sockfd = establish_ipv6_connection(server_addresses[server_idx], port); // 使用之前提供的函数建立连接

        // 为每个QP生成本地目的地信息并发送给服务器
        for (int qp_idx = 0; qp_idx < qps_per_server; ++qp_idx) {
            int idx = server_idx * qps_per_server + qp_idx;
            gid_to_wire_gid(&my_dests[idx].gid, gid);
            char msg[sizeof "0000:000000:000000:00000000000000000000000000000000"];
            sprintf(msg, "%04x:%06x:%06x:%s", my_dests[idx].lid, my_dests[idx].qpn,
                                    my_dests[idx].psn, gid);
            if (write(sockfd, msg, sizeof msg) != sizeof msg) {
                fprintf(stderr, "Couldn't send local address\n");
            }
            if (read(sockfd, msg, sizeof msg) != sizeof msg ||
                write(sockfd, "done", sizeof "done") != sizeof "done") {
                perror("client read/write");
                fprintf(stderr, "Couldn't read/write remote address\n");
            }
            sscanf(msg, "%x:%x:%x:%s", &rem_dests[idx].lid, &rem_dests[idx].qpn,
                              &rem_dests[idx].psn, gid);
            wire_gid_to_gid(gid, &rem_dests[idx].gid);
        }

        close(sockfd); // 关闭TCP连接
    }
    printf("IPv6 connect success!\n");
    // 此时my_dests和rem_dests都已填充完毕，可以继续进行RDMA连接的建立等后续步骤...
    // 配置每个QP
    for (int i = 0; i < num_qps; ++i) {
        // 假设pp_connect_ctx已经被正确定义和实现
        if (pp_connect_ctx(ctx, ib_port, my_dests[i].psn, mtu, sl, rem_dests, gidx, i)) {
            fprintf(stderr, "Failed to connect QPs\n");
            return 1;
        }
    }
    if ((strcmp(mode, "pull") == 0) || (strcmp(mode, "all") == 0) || (strcmp(mode, "allmulti") == 0) ) {
    printf(" pull:%d\n", num_servers);
    for (int ss = start_size; ss <= end_size / 2; ss *= 2) {
        int udx = -1;
        for (int round = 0; round < iters; ++round) {

                // 记录开始时间
                clock_gettime(CLOCK_MONOTONIC, &start);
                // 发送请求
                for (int i = 0; i < ctx->qp_num; ++i) {
                    if (pp_post_send(ctx, i, 32)) {  // write
                        fprintf(stderr, "Couldn't post send\n");
                        exit(1);
                    } // 假设消息大小为32字节
                }

                // 等待所有回复
                int num_wc = 0;
                int ne = 0;
                while (num_wc < 2 * num_qps) {
                    for (int cq_index = 0; cq_index < ctx->qp_num; ++cq_index) {
                        struct ibv_wc wc;
                        if (ibv_poll_cq(pp_cq(ctx, cq_index), 1, &wc) <= 0) continue;
                        if (wc.status != IBV_WC_SUCCESS) {
                            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
                                ibv_wc_status_str(wc.status), wc.status, (int)wc.wr_id);
                            exit(1);
                        }
                        ++num_wc;
                        if ((int)wc.wr_id & 1) {
                            if (pp_post_recv(ctx, 1, cq_index) != 1) {
                                fprintf(stderr, "Failed to repost recv WR\n");
                                exit(1);
                            }
                        }
                    }
                }
                //记录结束时间
                clock_gettime(CLOCK_MONOTONIC, &end);
                long seconds, nanoseconds;
                double microseconds;
                seconds = end.tv_sec - start.tv_sec;
                nanoseconds = end.tv_nsec - start.tv_nsec;
                if (nanoseconds < 0) {
                    --seconds;
                    nanoseconds += 1000000000;
                }
                microseconds = seconds * 1000000 + nanoseconds / 1000.0;
                result[++udx] = microseconds;

                // 记录并处理完成时间
        }
        merge_sort(result, 0, udx);
        double sum = 0;
        for (int i = 0; i <= udx; i ++) sum += result[i];
        printf("Valid Count: %d\n", udx);
        double avg = sum / (udx + 1);
        printf("%d bytes  %d iters min:%.3fus\t  50\%:%.3fus\t 99\%:%.3fus\t 999\%:%.3fus\t Max:%.3fus\tAvg:%.3fus\n",
        ss,iters,result[0],result[(udx + 1) / 2], result[(int)((udx + 1) * 0.99)],
        result[(int)((udx + 1) * 0.999)],result[udx], avg);
    }

    }

    if ((strcmp(mode, "push") == 0) || (strcmp(mode, "all") == 0) || (strcmp(mode, "allmulti") == 0) ){
    printf(" push:%d\n", num_servers);
    for (int ss = start_size; ss <= end_size / 2; ss *= 2) {
        int udx = -1;
        for (int round = 0; round < iters; ++round) {
                // printf("Round:%d\n", round);
                // 记录开始时间
                clock_gettime(CLOCK_MONOTONIC, &start);
                // 发送请求
                for (int i = 0; i < ctx->qp_num; ++i) {
                    if (pp_post_send(ctx, i, ss)) {
                        fprintf(stderr, "Couldn't post send\n");
                        exit(1);
                    } // 假设消息大小为32字节
                }
                // 等待所有回复
                int num_wc = 0;
                int ne = 0;
                while (num_wc < num_qps) {
                    for (int cq_index = 0; cq_index < ctx->qp_num; ++cq_index) {
                        struct ibv_wc wc;
                        if (ibv_poll_cq(pp_cq(ctx, cq_index), 1, &wc) <= 0) continue;
                        if (wc.status != IBV_WC_SUCCESS) {
                            fprintf(stderr, "Completion with status %s (%d) for wr_id %d\n",
                                ibv_wc_status_str(wc.status), wc.status, (int)wc.wr_id);
                            exit(1);
                        }
                        ++num_wc;
                    }
                }
                //记录结束时间
                clock_gettime(CLOCK_MONOTONIC, &end);
                long seconds, nanoseconds;
                double microseconds;
                seconds = end.tv_sec - start.tv_sec;
                nanoseconds = end.tv_nsec - start.tv_nsec;
                if (nanoseconds < 0) {
                    --seconds;
                    nanoseconds += 1000000000;
                }
                microseconds = seconds * 1000000 + nanoseconds / 1000.0;
                result[++udx] = microseconds;

        }
        merge_sort(result, 0, udx);
        double sum = 0;
        for (int i = 0; i <= udx; i ++) sum += result[i];
        printf("Valid Count: %d\n", udx);
        double avg = sum / (udx + 1);
        printf("%d bytes  %d iters min:%.3fus\t  50\%:%.3fus\t 99\%:%.3fus\t 999\%:%.3fus\t Max:%.3fus\tAvg:%.3fus\n",
        ss,iters,result[0],result[(udx + 1) / 2], result[(int)((udx + 1) * 0.99)],
        result[(int)((udx + 1) * 0.999)],result[udx], avg);
    }


    }
    if ((strcmp(mode, "multipull") == 0) || (strcmp(mode, "multi") == 0) || (strcmp(mode, "allmulti") == 0)){
    printf("multithreading pull:%d\n", num_servers);
    for (int size = start_size; size <= end_size / 2; size *= 2) {

        int udx = -1;
        pthread_t threads[NUM_THREADS];
        thread_data data[NUM_THREADS];
        void *status;
        for (int round = 0; round < iters; ++round) {
                for (int i = 0; i < num_qps; ++i) {
                    data[i].thread_id = i;
                    data[i].qp = ctx->qp[i];
                    data[i].cq = ctx->cq[i];
                    data[i].mr = ctx->mr[i];
                    data[i].buf = ctx->buf[i];
                    data[i].msg_size = 32;
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
                pthread_mutex_lock(&mutex);
                ready = 1;
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&mutex);
                for (int i = 0; i < num_qps; ++i) {
                    pthread_join(threads[i], &status);
                    if ((long)status == -1) {
                        printf("Thread #%d exited with error\n", i);
                        exit(1);
                    }
                }
                //记录结束时间
                // clock_gettime(CLOCK_MONOTONIC, &end);
                struct timespec start = data[0].startimes;
                for (int i = 1; i < num_qps; ++i) {
                    if ((data[i].startimes.tv_sec < start.tv_sec) ||
                        (data[i].startimes.tv_sec == start.tv_sec && data[i].startimes.tv_nsec < start.tv_nsec)) {
                        start = data[i].startimes;
                    }
                }

                struct timespec end = data[0].endtimes;
                for (int i = 1; i < num_qps; ++i) {
                    if ((data[i].endtimes.tv_sec > end.tv_sec) ||
                        (data[i].endtimes.tv_sec == end.tv_sec && data[i].endtimes.tv_nsec > end.tv_nsec)) {
                        end = data[i].endtimes;
                    }
                }

                long seconds, nanoseconds;
                double microseconds;
                seconds = end.tv_sec - start.tv_sec;
                nanoseconds = end.tv_nsec - start.tv_nsec;
                if (nanoseconds < 0) {
                    --seconds;
                    nanoseconds += 1000000000;
                }
                microseconds = seconds * 1000000 + nanoseconds / 1000.0;
                result[++udx] = microseconds;

                // 记录并处理完成时间
        }

        merge_sort(result, 0, udx);
        double sum = 0;
        for (int i = 0; i <= udx; i ++) sum += result[i];
        printf("Valid Count: %d\n", udx);
        double avg = sum / (udx + 1);
        printf("%d bytes  %d iters min:%.3fus\t  50\%:%.3fus\t 99\%:%.3fus\t 999\%:%.3fus\t Max:%.3fus\tAvg:%.3fus\n",
        size,iters,result[0],result[(udx + 1) / 2], result[(int)((udx + 1) * 0.99)],
        result[(int)((udx + 1) * 0.999)],result[udx], avg);

    }


    }
    if ((strcmp(mode, "multipush") == 0) || (strcmp(mode, "multi") == 0) || (strcmp(mode, "allmulti") == 0)){
    printf("multithreading push:%d\n", num_servers);
    for (int size = start_size; size <= end_size / 2; size *= 2) {

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
                    data[i].msg_size = size;
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

                pthread_mutex_lock(&mutex);
                ready = 1;
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&mutex);
                for (int i = 0; i < num_qps; ++i) {
                    pthread_join(threads[i], &status);
                    if ((long)status == -1) {
                        printf("Thread #%d exited with error\n", i);
                        exit(1);
                    }
                }

                struct timespec start = data[0].startimes;
                for (int i = 1; i < num_qps; ++i) {
                    if ((data[i].startimes.tv_sec < start.tv_sec) ||
                        (data[i].startimes.tv_sec == start.tv_sec && data[i].startimes.tv_nsec < start.tv_nsec)) {
                        start = data[i].startimes;
                    }
                }
                struct timespec end = data[0].endtimes;
                for (int i = 1; i < num_qps; ++i) {
                    if ((data[i].endtimes.tv_sec > end.tv_sec) ||
                        (data[i].endtimes.tv_sec == end.tv_sec && data[i].endtimes.tv_nsec > end.tv_nsec)) {
                        end = data[i].endtimes;
                    }
                }
                long seconds, nanoseconds;
                double microseconds;
                seconds = end.tv_sec - start.tv_sec;
                nanoseconds = end.tv_nsec - start.tv_nsec;
                if (nanoseconds < 0) {
                    --seconds;
                    nanoseconds += 1000000000;
                }
                microseconds = seconds * 1000000 + nanoseconds / 1000.0;
                result[++udx] = microseconds;

        }
        merge_sort(result, 0, udx);
        double sum = 0;
        for (int i = 0; i <= udx; i ++) sum += result[i];
        printf("Valid Count: %d\n", udx);
        double avg = sum / (udx + 1);
        printf("%d bytes  %d iters min:%.3fus\t  50\%:%.3fus\t 99\%:%.3fus\t 999\%:%.3fus\t Max:%.3fus\tAvg:%.3fus\n",
        size,iters,result[0],result[(udx + 1) / 2], result[(int)((udx + 1) * 0.99)],
        result[(int)((udx + 1) * 0.999)],result[udx], avg);
    }
    }

    pp_close_ctx(ctx);
    if (ib_devname) {
        free(ib_devname); // 释放之前通过strdup分配的内存
    }
    ibv_free_device_list(dev_list);
    return 0;
}