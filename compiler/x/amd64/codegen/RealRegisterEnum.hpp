/*******************************************************************************
 * Copyright (c) 2000, 2022 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

/*
 * This file will be included within an enum.  Only comments and enumerator
 * definitions are permitted.
 */

   NoReg                   = 0,

   // The order of the GPRS registers is defined by the linkage
   // convention, and the VM may rely on it (eg. for GC).
   eax                     = 1,
   FirstGPR                = eax,
   ebx                     = 2,
   ecx                     = 3,
   edx                     = 4,
   edi                     = 5,
   esi                     = 6,
   ebp                     = 7,
   esp                     = 8,
   r8                      = 9,
   r9                      = 10,
   r10                     = 11,
   r11                     = 12,
   r12                     = 13,
   r13                     = 14,
   r14                     = 15,
   r15                     = 16,
   LastGPR                 = r15,
   Last8BitGPR             = r15,
   LastAssignableGPR       = r15,

   vfp                     = 17,

   FPRMaskOffset           = LastGPR,
   st0                     = 18,
   FirstFPR                = st0,
   st1                     = 19,
   st2                     = 20,
   st3                     = 21,
   st4                     = 22,
   st5                     = 23,
   st6                     = 24,
   st7                     = 25,
   LastFPR                 = st7,
   LastAssignableFPR       = st7,

   XMMRMaskOffset          = LastGPR,
   xmm0                    = 26,
   FirstXMMR               = xmm0,
   xmm1                    = 27,
   xmm2                    = 28,
   xmm3                    = 29,
   xmm4                    = 30,
   xmm5                    = 31,
   xmm6                    = 32,
   xmm7                    = 33,
   xmm8                    = 34,
   xmm9                    = 35,
   xmm10                   = 36,
   FirstSpillReg           = xmm10,
   xmm11                   = 37,
   xmm12                   = 38,
   xmm13                   = 39,
   xmm14                   = 40,
   xmm15                   = 41,
   LastSSE2XMMReg          = xmm15,
   xmm16                   = 42,
   xmm17                   = 43,
   xmm18                   = 44,
   xmm19                   = 45,
   xmm20                   = 46,
   xmm21                   = 47,
   xmm22                   = 48,
   xmm23                   = 49,
   xmm24                   = 50,
   xmm25                   = 51,
   xmm26                   = 52,
   xmm27                   = 53,
   xmm28                   = 54,
   xmm29                   = 55,
   xmm30                   = 56,
   xmm31                   = 57,
   LastSpillReg            = xmm31,
   LastXMMR                = xmm31,

   YMMRMaskOffset          = LastGPR,
   ymm0                    = xmm0,
   FirstYMMR               = ymm0,
   ymm1                    = xmm1,
   ymm2                    = xmm2,
   ymm3                    = xmm3,
   ymm4                    = xmm4,
   ymm5                    = xmm5,
   ymm6                    = xmm6,
   ymm7                    = xmm7,
   ymm8                    = xmm8,
   ymm9                    = xmm9,
   ymm10                   = xmm10,
   ymm11                   = xmm11,
   ymm12                   = xmm12,
   ymm13                   = xmm13,
   ymm14                   = xmm14,
   ymm15                   = xmm15,
   ymm16                   = xmm16,
   ymm17                   = xmm17,
   ymm18                   = xmm18,
   ymm19                   = xmm19,
   ymm20                   = xmm20,
   ymm21                   = xmm21,
   ymm22                   = xmm22,
   ymm23                   = xmm23,
   ymm24                   = xmm24,
   ymm25                   = xmm25,
   ymm26                   = xmm26,
   ymm27                   = xmm27,
   ymm28                   = xmm28,
   ymm29                   = xmm29,
   ymm30                   = xmm30,
   ymm31                   = xmm31,
   LastYMMR                = ymm31,

   ZMMRMaskOffset          = LastGPR,
   zmm0                    = xmm0,
   FirstZMMR               = zmm0,
   zmm1                    = xmm1,
   zmm2                    = xmm2,
   zmm3                    = xmm3,
   zmm4                    = xmm4,
   zmm5                    = xmm5,
   zmm6                    = xmm6,
   zmm7                    = xmm7,
   zmm8                    = xmm8,
   zmm9                    = xmm9,
   zmm10                   = xmm10,
   zmm11                   = xmm11,
   zmm12                   = xmm12,
   zmm13                   = xmm13,
   zmm14                   = xmm14,
   zmm15                   = xmm15,
   zmm16                   = xmm16,
   zmm17                   = xmm17,
   zmm18                   = xmm18,
   zmm19                   = xmm19,
   zmm20                   = xmm20,
   zmm21                   = xmm21,
   zmm22                   = xmm22,
   zmm23                   = xmm23,
   zmm24                   = xmm24,
   zmm25                   = xmm25,
   zmm26                   = xmm26,
   zmm27                   = xmm27,
   zmm28                   = xmm28,
   zmm29                   = xmm29,
   zmm30                   = xmm30,
   zmm31                   = xmm31,
   LastZMMR                = zmm31,

   // avx512 write mask registers
   // k0 -> reserved for no write mask
   k1                      = 58,
   k2                      = 59,
   k3                      = 60,
   k4                      = 61,
   k5                      = 62,
   k6                      = 63,
   k7                      = 64,

   AllFPRegisters          = 65,
   ByteReg                 = 66,
   BestFreeReg             = 67,
   SpilledReg              = 68,
   NumRegisters            = 69,

   NumXMMRegisters         = LastXMMR - FirstXMMR + 1,
   MaxAssignableRegisters  = NumXMMRegisters + (LastAssignableGPR - FirstGPR + 1) - 1 // -1 for stack pointer
