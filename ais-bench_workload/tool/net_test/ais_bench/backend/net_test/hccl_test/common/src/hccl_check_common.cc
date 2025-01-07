#include <iostream>
#include <chrono>
#include <dlfcn.h>
#include <climits>
#include <sys/wait.h>
#include "hccl_test_common.h"
#include <sys/syscall.h>
#include "hccl_check_common.h"

int CheckBufResultFloat(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0; // j = 0;
    // int n = 0;
    int err = 0;
    float *c_buf = (float *)checkBuf;
    float *result = (float *)resultBuf;
    u64 first_err_pos = ULLONG_MAX;
    for (i = 0; i < count; ++i) {
        if (fabsf(c_buf[i] - result[i]) > HCCL_EPSION_FLOAT) {
            if (fabsf(result[i]) > 0) {
                if (fabsf(fabsf(c_buf[i] - result[i]) / result[i]) > (HCCL_EPSION_FLOAT * 100)) {
                    if (first_err_pos == ULLONG_MAX) {
                        first_err_pos = i;
                    }
                    err++;
                }
            } else {
                if (fabsf(c_buf[i] - result[i]) > 0.001) {
                    if (first_err_pos == ULLONG_MAX) {
                        first_err_pos = i;
                    }
                    err++;
                }
            }
        }
    }

    if (err > 0) {
        ERROR("Check buf[%llu] error, exp:%f, act:%f ", first_err_pos, c_buf[first_err_pos], result[first_err_pos]);
    }
    if (err > 0) {
        ERROR("Total err is %d", err);
    }
    return err;
}

int CheckBufResultInt8(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0;
    s8 *c_buf = (s8 *)checkBuf;
    s8 *result = (s8 *)resultBuf;
    int err = 0;
    u64 first_err_pos = ULLONG_MAX;
    for (i = 0; i < count; ++i) {
        if (c_buf[i] != result[i]) {
            if (first_err_pos == ULLONG_MAX) {
                first_err_pos = i;
            }
            err++;
        }
    }

    if (err > 0) {
        ERROR("Result buf[%llu] is not right,exp: %d, act:%d ", first_err_pos, c_buf[first_err_pos],
            result[first_err_pos]);
    }
    if (err > 0) {
        ERROR("Total err is %d", err);
    }
    return err;
}

int CheckBufResultHalf(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0;
    u16 *result = (u16 *)resultBuf;
    u16 *s = (u16 *)checkBuf;
    int err = 0;
    u64 first_err_pos = ULLONG_MAX;

    for (i = 0; i < count; ++i) {
        if (s[i] != result[i]) {
            if (first_err_pos == ULLONG_MAX) {
                first_err_pos = i;
            }
            err++;
        }
    }

    if (err > 0) {
        ERROR("Result buf[%llu] is not right,exp: %u, act:%u ", first_err_pos, s[first_err_pos],
            result[first_err_pos]);
    }
    if (err > 0) {
        ERROR("Total err is %d", err);
    }
    return err;
}

int CheckBufResultInt32(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0;
    int *c_buf = (int *)checkBuf;
    int *result = (int *)resultBuf;
    int err = 0;
    u64 first_err_pos = ULLONG_MAX;
    for (i = 0; i < count; ++i) {
        if (c_buf[i] != result[i]) {
            if (first_err_pos == ULLONG_MAX) {
                first_err_pos = i;
            }
            err++;
        }
    }

    if (err > 0) {
        ERROR("Result buf[%llu] is not right,exp: %d, act:%d ", first_err_pos, c_buf[first_err_pos],
            result[first_err_pos]);
    }
    if (err > 0) {
        ERROR("Total err is %d", err);
    }
    return err;
}

int CheckBufResultInt64(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0;
    s64 *c_buf = (s64 *)checkBuf;
    s64 *result = (s64 *)resultBuf;
    int err = 0;
    u64 first_err_pos = ULLONG_MAX;
    for (i = 0; i < count; ++i) {
        if (c_buf[i] != result[i]) {
            if (first_err_pos == ULLONG_MAX) {
                first_err_pos = i;
            }
            err++;
        }
    }

    if (err > 0) {
        ERROR("Result buf[%llu] is not right,exp: %lld, act:%lld ", first_err_pos, c_buf[first_err_pos],
            result[first_err_pos]);
    }
    if (err > 0) {
        ERROR("Total err is %d", err);
    }

    return err;
}

int CheckBufResultU64(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0;
    u64 *c_buf = (u64 *)checkBuf;
    u64 *result = (u64 *)resultBuf;
    int err = 0;
    u64 first_err_pos = ULLONG_MAX;
    for (i = 0; i < count; ++i) {
        if (c_buf[i] != result[i]) {
            if (first_err_pos == ULLONG_MAX) {
                first_err_pos = i;
            }
            err++;
        }
    }

    if (err > 0) {
        ERROR("Result buf[%llu] is not right,exp: %llu, act:%llu ", first_err_pos, c_buf[first_err_pos],
            result[first_err_pos]);
    }

    if (err > 0) {
        ERROR("Total err is %d", err);
    }

    return err;
}