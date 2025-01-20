/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <chrono>
#include <dlfcn.h>
#include <climits>
#include <sys/wait.h>
#include "hccl_test_common.h"
#include <sys/syscall.h>
#include "hccl_check_common.h"

const double RESULT_PROCESSION = 0.001;
const int MULTIPER_100 = 100;

int CheckBufResultFloat(const void *resultBuf, const void *checkBuf, u64 count)
{
    u64 i = 0;
    int err = 0;
    float *c_buf = (float *)checkBuf;
    float *result = (float *)resultBuf;
    u64 first_err_pos = ULLONG_MAX;
    for (i = 0; i < count; ++i) {
        if (fabsf(c_buf[i] - result[i]) > HCCL_EPSION_FLOAT) {
            if (fabsf(result[i]) > 0) {
                if (fabsf(fabsf(c_buf[i] - result[i]) / result[i]) <= (HCCL_EPSION_FLOAT * MULTIPER_100)) continue;
            } else {
                if (fabsf(c_buf[i] - result[i]) <= RESULT_PROCESSION) continue;
            }
            if (first_err_pos == ULLONG_MAX) {
                first_err_pos = i;
            }
            err++;
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