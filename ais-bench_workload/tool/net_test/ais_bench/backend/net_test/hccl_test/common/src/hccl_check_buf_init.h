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

#ifndef HCCL_CHECK_BUF_INIT_H_
#define HCCL_CHECK_BUF_INIT_H_
#include <map>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <hccl/hccl_types.h>
#include "hccl_test_common.h"
#include "hccl_test_logger.h"

const int OFFSET_16 = 16;
const int OFFSET_13 = 13;
const int EIGHT_BIT = 256;


static inline float Fp32FromBits (uint32_t w)
{
#if defined(__OPENCL_VERSION__)
    return as_float(w);
#elif defined(__CUDA_ARCH__)
    return __uint_as_float((unsigned int)w);
#elif defined(__INTEL_COMPILER)
    return _castu32_f32(w);
#else
    union {
        uint32_t as_bits;
        float as_value;
    } fp32 = { w };
    return fp32.as_value;
#endif
}

static inline uint32_t Fp32ToBits(float f)
{
#if defined(__OPENCL_VERSION__)
    return as_uint(f);
#elif defined(__CUDA_ARCH__)
    return (uint32_t)__float_as_uint(f);
#elif defined(__INTEL_COMPILER)
    return _castf32_u32(f);
#else
    union {
        float as_value;
        uint32_t as_bits;
    } fp32 = { f };
    return fp32.as_bits;
#endif
}

static inline uint16_t Fp16IeeeFromFp32Value(float f)
{
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || defined(__GNUC__) && !defined(__STRICT_ANSI__)
    const float scale_to_inf = 0x1.0p+112f;
    const float scale_to_zero = 0x1.0p-110f;
#else
    const float scale_to_inf = Fp32FromBits(UINT32_C(0x77800000));
    const float scale_to_zero = Fp32FromBits(UINT32_C(0x08800000));
#endif
    float base = (fabsf(f) * scale_to_inf) * scale_to_zero;

    const uint32_t w = Fp32ToBits(f);
    const uint32_t shl1_w = w + w;
    const uint32_t sign = w & UINT32_C(0x80000000);
    uint32_t bias = shl1_w & UINT32_C(0xFF000000);
    if (bias < UINT32_C(0x71000000)) {
        bias = UINT32_C(0x71000000);
    }

    base = Fp32FromBits((bias >> 1) + UINT32_C(0x07800000)) + base;
    const uint32_t bits = Fp32ToBits(base);
    const uint32_t exp_bits = (bits >> OFFSET_13) & UINT32_C(0x00007C00);
    const uint32_t mantissa_bits = bits & UINT32_C(0x00000FFF);
    const uint32_t nonsign = exp_bits + mantissa_bits;
    return (sign >> OFFSET_16) | (shl1_w > UINT32_C(0xFF000000) ? UINT16_C(0x7E00) : nonsign);
}

static inline uint16_t Fp32ToBf16(float x)
{
    float y = x;
    int *p = (int *) &y;
    unsigned int exp;
    unsigned int man;
    exp = *p & 0x7F800000u;
    man = *p & 0x007FFFFFu;
    if (exp == 0 && man == 0) {
        // zero
        return x;
    }
    if (exp == 0x7F800000u) {
        // infinity or Nans
        return x;
    }
    // Normalized number
    // round to nearest
    float r = x;
    int *pr = (int *) &r;
    *pr &= 0xff800000; // r has the same exp as x
    r = r / EIGHT_BIT;
    y = x + r;

    *p &= 0xffff0000;

    return y;
}

typedef void(*HostBufInitFunc)(void *, u64, int);
extern std::map<int, HostBufInitFunc> functionMap;

typedef void(*ReduceCheckBufInitFunc)(void *, u64, int, int, int);
extern std::map<int, ReduceCheckBufInitFunc> functionReduceMap;

typedef int(*AllToAllCheckResult)(const void*, u64*, u64*, int, int);
extern std::map<int, AllToAllCheckResult> functionAllToAllMap;

extern void HcclHostBufInit(void *dstBuf, unsigned long long count, int dtype, int val);
extern void HcclReduceCheckBufInit(void *dstBuf, unsigned long long count, int dtype, int op, int val,
    int rankSize);
extern int HcclAlltoallvCheckResult(void *checkBuf, unsigned long long *recvCounts, unsigned long long *recvDisp,
    int rankId, int rankSize, int dtype);

#endif