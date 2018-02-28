// Copyright (c) 2016-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

/**
 * Return number of arguments
 */
#define __VA_NARG__(...) \
    _NARG_(__VA_ARGS__,_RSEQ_N())

#define _NARG_(...) \
    _ARG_N(__VA_ARGS__)

#define _ARG_N( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
    _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
    _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,N,...) N

#define _RSEQ_N() \
    63,62,61,60,59,58,57,56,55,54, \
    53,52,51,50,49,48,47,46,45,44, \
    43,42,41,40,39,38,37,36,35,34, \
    33,32,31,30,29,28,27,26,25,24, \
    23,22,21,20,19,18,17,16,15,14, \
    13,12,11,10, 9, 8, 7, 6, 5, 4, \
    3, 2, 1, 0

#define __MCRCAT(a, b)  a ## b
#define _MCRCAT(a, b)   __MCRCAT(a, b)

/**
 * @brief Create a string for format and arguments
 *
 * Note that argument macros are supported up to 5.
 */
#define _KVS1() \
    ""
#define _KVS3(k1, f1, v1) \
    "{" #k1 ":" #f1 "}", \
    v1
#define _KVS6(k1, f1, v1, k2, f2, v2) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 "}", \
    v1, v2
#define _KVS7(k1, f1, v1, k2, f2, v2, ...) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 "}", \
    v1, v2
#define _KVS9(k1, f1, v1, k2, f2, v2, k3, f3, v3) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 "}", \
    v1, v2, v3
#define _KVS10(k1, f1, v1, k2, f2, v2, k3, f3, v3, ...) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 "}", \
    v1, v2, v3
#define _KVS12(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 "}", \
    v1, v2, v3, v4
#define _KVS13(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, ...) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 "}", \
    v1, v2, v3, v4
#define _KVS15(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 "}", \
    v1, v2, v3, v4, v5
#define _KVS16(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5, ...) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 "}", \
    v1, v2, v3, v4, v5
#define _KVS18(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5, \
    k6, f6, v6) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 ", " \
    #k6 ":" #f6 "}", \
    v1, v2, v3, v4, v5, v6
#define _KVS21(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5, \
    k6, f6, v6, k7, f7, v7) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 ", " \
    #k6 ":" #f6 ", " #k7 ":" #f7 "}", \
    v1, v2, v3, v4, v5, v6, v7
#define _KVS24(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5, \
    k6, f6, v6, k7, f7, v7, k8, f8, v8) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 ", " \
    #k6 ":" #f6 ", " #k7 ":" #f7 ", " #k8 ":" #f8 "}", \
    v1, v2, v3, v4, v5, v6, v7, v8
#define _KVS27(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5, \
    k6, f6, v6, k7, f7, v7, k8, f8, v8, k9, f9, v9) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 ", " \
    #k6 ":" #f6 ", " #k7 ":" #f7 ", " #k8 ":" #f8 ", " #k9 ":" #f9 "}", \
    v1, v2, v3, v4, v5, v6, v7, v8, v9
#define _KVS30(k1, f1, v1, k2, f2, v2, k3, f3, v3, k4, f4, v4, k5, f5, v5, \
    k6, f6, v6, k7, f7, v7, k8, f8, v8, k9, f9, v9, k10, f10, v10) \
    "{" #k1 ":" #f1 ", " #k2 ":" #f2 ", " #k3 ":" #f3 ", " #k4 ":" #f4 ", " #k5 ":" #f5 ", " \
    #k6 ":" #f6 ", " #k7 ":" #f7 ", " #k8 ":" #f8 ", " #k9 ":" #f9 ", " #k10 ":" #f10 "}", \
    v1, v2, v3, v4, v5, v6, v7, v8, v9, v10

/**
 * @brief Handle variadic variable.
 *
 * Takes argument macros defined on "PmTrace.h".
 * Substitute to appropriate "_KVS##" macro depends on a number or arguments.
 *
 * Eventually, it will be arguments for "const char* fmt, ..." of _Pmt[...] functions.
 */
#define FORMATTED_VA(...) \
    _MCRCAT(_KVS, __VA_NARG__(__VA_ARGS__))(__VA_ARGS__)
