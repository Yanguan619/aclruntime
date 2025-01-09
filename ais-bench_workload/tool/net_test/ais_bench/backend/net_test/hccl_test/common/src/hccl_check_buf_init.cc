#include <iostream>
#include <chrono>
#include <dlfcn.h>
#include <climits>
#include <sys/wait.h>
#include "hccl_test_common.h"
#include <sys/syscall.h>
#include "hccl_check_buf_init.h"
#include <map>

void HostBufInitFp32(void* dstBuf, u64 count, int val)
{
    float* f_tmp = NULL;
    f_tmp = (float*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        f_tmp[j] = val;
    }
    return;
}

void HostBufInitInt8(void* dstBuf, u64 count, int val)
{
    char* tmp = NULL;
    tmp = (char*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        tmp[j] = val % 128;
    }
    return;
}

void HostBufInitInt32(void* dstBuf, u64 count, int val)
{
    int* t_tmp = NULL;
    t_tmp = (int*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        t_tmp[j] = val;
    }
    return;
}

void HostBufInitFp16(void* dstBuf, u64 count, int val)
{
    unsigned short* f16_temp = NULL;
    f16_temp = (u16*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        f16_temp[j] = Fp16IeeeFromFp32Value(val);
    }
    return;
}

void HostBufInitInt16(void* dstBuf, u64 count, int val)
{
    short* s16_tmp = NULL;
    s16_tmp = (s16*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        s16_tmp[j] = val;
    }
    return;
}

void HostBufInitInt64(void* dstBuf, u64 count, int val)
{
    int64_t* tmp_buf = NULL;
    tmp_buf = (int64_t*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        tmp_buf[j] = val;
    }
    return;
}

void HostBufInitBfp16(void* dstBuf, u64 count, int val)
{
    unsigned short *f16_temp = (u16*)dstBuf;
    for (u64 j = 0; j < count; ++j) {
        f16_temp[j] = Fp32ToBf16(val);  // fp32转换bf16
    }
    return;
}

void HcclHostBufInit(void* dstBuf, u64 count, int dtype, int val)
{
    if (functionMap.find(dtype) != functionMap.end()) {
        functionMap[dtype](dstBuf, count, val);
    }
    return;
}

void ReduceCheckBufInitFp32(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    float* f_tmp = NULL;
    f_tmp = (float*)dstBuf;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            f_tmp[j] = val * rank_size;
        }
    } else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            f_tmp[j] = pow(val, rank_size);
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            f_tmp[j] = val;
        }
    }
    return;
}

void ReduceCheckBufInitInt8(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    char* tmp = NULL;
    tmp = (char*)dstBuf;
    int n = 0;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            n = (val % 128) * rank_size;
            if (n > 127) {
                n = 127;
            }
            tmp[j] = n; // 大于128取127
        }
    } else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            n = ((int)pow(val % 128, rank_size)); // 大于128取127
            if (n > 127) {
                n = 127;
            }
            tmp[j] = n; // 大于128取127
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            if (val > 127) {
                val = 127;
            }
            tmp[j] = val % 128;
        }
    }
    return;
}

void ReduceCheckBufInitInt32(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    int*  t_tmp = NULL;
    t_tmp = (int*)dstBuf;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            t_tmp[j] = val * rank_size;
        }
    } else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            t_tmp[j] = pow(val, rank_size);
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            t_tmp[j] = val;
        }
    }
    return;
}

void ReduceCheckBufInitFp16(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    u16* f16_temp = NULL;
    f16_temp = (u16*)dstBuf;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            f16_temp[j] = Fp16IeeeFromFp32Value(val * rank_size);
        }
    } else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            f16_temp[j] = Fp16IeeeFromFp32Value(pow(val, rank_size));
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            f16_temp[j] = Fp16IeeeFromFp32Value(val);
        }
    }
    return;
}

void ReduceCheckBufInitInt16(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    s16* s16_temp = NULL;
    s16_temp = (s16*)dstBuf;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            s16_temp[j] = val * rank_size;
        }
    } else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            s16_temp[j] = pow(val, rank_size);
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            s16_temp[j] = val;
        }
    }
    return;
}

void ReduceCheckBufInitInt64(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    int64_t* temp = nullptr;
    temp = (int64_t*)dstBuf;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            temp[j] = val * rank_size;
        }
    } else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            temp[j] = pow(val, rank_size);
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            temp[j] = val;
        }
    }
    return;
}

void ReduceCheckBufInitBfp16(void* dstBuf, u64 count, int val, int op, int rank_size)
{
    unsigned short *f16_temp = (u16*)dstBuf;
    if (op == HCCL_REDUCE_SUM) {
        for (u64 j = 0; j < count; ++j) {
            f16_temp[j] = Fp32ToBf16(val * rank_size);
        }
    }
    else if (op == HCCL_REDUCE_PROD) {
        for (u64 j = 0; j < count; ++j) {
            f16_temp[j] = Fp32ToBf16(pow(val, rank_size));
        }
    } else if (op == HCCL_REDUCE_MIN || op == HCCL_REDUCE_MAX) {
        for (u64 j = 0; j < count; ++j) {
            f16_temp[j] = Fp32ToBf16(val);
        }
    }
    return;
}

void HcclReduceCheckBufInit(void *dstBuf, u64 count, int dtype, int op, int val, int rankSize)
{
    if (functionReduceMap.find(dtype) != functionReduceMap.end()) {
        functionReduceMap[dtype](dstBuf, count, val, op, rankSize);
    }
    return;
}


int AlltoallCheckResultUint64(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    u64 *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        u64 check_val = i + 1;
        result = (u64 *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%llu, act:%llu", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultFp32(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    float *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        float check_val = i + 1;
        result = (float *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%f, act:%f", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultInt8(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    char *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        char check_val = i + 1;
        result = (char *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultInt32(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    int *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        int check_val = i + 1;
        result = (int *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultInt64(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    long long *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        long long check_val = i + 1;
        result = (long long *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%lld, act:%lld", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultInt16(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    short *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        short check_val = i + 1;
        result = (short *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultFp16(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    u16 *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        float val = i + 1;
        u16 check_val = Fp16IeeeFromFp32Value(val);
        result = (u16 *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultUint8(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    uint8_t *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        uint8_t check_val = i + 1;
        result = (uint8_t *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultUint16(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    uint16_t *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        uint16_t check_val = i + 1;
        result = (uint16_t *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultUint32(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    uint32_t *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        uint32_t check_val = i + 1;
        result = (uint32_t *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (result[j] != check_val) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int AlltoallCheckResultBfp32(const void *check_buf, u64 *recv_counts, u64 *recv_disp, int rank_size, int dtype)
{
    int ret = 0;
    u16 *result = NULL;
    for (int i = 0; i < rank_size; ++i) {
        u16 check_val = i + 1;
        result = (u16 *)check_buf + recv_disp[i];
        for (u64 j = 0; j < recv_counts[i]; ++j) {
            if (fabs(result[j] - check_val) / abs(result[j]) > 0.001) {
                ERROR("Check data from rank %d  result[%llu] error, exp:%d, act:%d", i, j, check_val, result[j]);
                ret++;
            }
        }
    }
    return ret;
}

int HcclAlltoallvCheckResult(void *checkBuf, u64 *recvCounts, u64 *recvDisp, int rankId, int rankSize, int dtype)
{
    int ret = 0;
    if (rankSize < 1) { // 接收数据为0则不进行数据校验
        return ret;
    }

    if (functionAllToAllMap.find(dtype) != functionAllToAllMap.end()) {
        ret = functionAllToAllMap[dtype](checkBuf, recvCounts, recvDisp, rankSize, dtype);
    }
    return ret;
}

std::map<int, HostBufInitFunc> functionMap = {
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_FP32, HostBufInitFp32),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_INT8, HostBufInitInt8),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_UINT8, HostBufInitInt8),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_INT32, HostBufInitInt32),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_UINT32, HostBufInitInt32),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_FP16, HostBufInitFp16),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_INT16, HostBufInitInt16),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_UINT16, HostBufInitInt16),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_INT64, HostBufInitInt64),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_FP64, HostBufInitInt64),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_UINT64, HostBufInitInt64),
    std::pair<int, HostBufInitFunc>(HCCL_DATA_TYPE_BFP16, HostBufInitBfp16)
};

std::map<int, ReduceCheckBufInitFunc> functionReduceMap = {
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_FP32, ReduceCheckBufInitFp32),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_INT8, ReduceCheckBufInitInt8),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_INT32, ReduceCheckBufInitInt32),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_FP16, ReduceCheckBufInitFp16),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_INT16, ReduceCheckBufInitInt16),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_INT64, ReduceCheckBufInitInt64),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_UINT64, ReduceCheckBufInitInt64),
    std::pair<int, ReduceCheckBufInitFunc>(HCCL_DATA_TYPE_BFP16, ReduceCheckBufInitBfp16)
};

std::map<int, AllToAllCheckResult> functionAllToAllMap = {
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_UINT64, AlltoallCheckResultUint64),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_FP32, AlltoallCheckResultFp32),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_INT8, AlltoallCheckResultInt8),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_INT32, AlltoallCheckResultInt32),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_INT64, AlltoallCheckResultInt64),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_INT16, AlltoallCheckResultInt16),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_FP16, AlltoallCheckResultFp16),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_UINT8, AlltoallCheckResultUint8),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_UINT16, AlltoallCheckResultUint16),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_UINT32, AlltoallCheckResultUint32),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_FP64, AlltoallCheckResultInt64),
    std::pair<int, AllToAllCheckResult>(HCCL_DATA_TYPE_BFP16, AlltoallCheckResultBfp32)
};
