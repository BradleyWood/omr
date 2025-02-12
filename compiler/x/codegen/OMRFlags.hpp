/*******************************************************************************
 * Copyright IBM Corp. and others 2025
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
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
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#ifndef OMR_OMRFLAGS_HPP
#define OMR_OMRFLAGS_HPP

enum InsnFlags
   {
   NoFlag,
   ModifiesTarget,
   ModifiesSource,
   UsesTarget,
   SingleFP,
   DoubleFP,
   ByteImmediate,
   ShortImmediate,
   IntImmediate,
   SignExtendImmediate,
   TestsZeroFlag,
   ModifiesZeroFlag,
   TestsSignFlag,
   ModifiesSignFlag,
   TestsCarryFlag,
   ModifiesCarryFlag,
   TestsOverflowFlag,
   ModifiesOverflowFlag,
   ByteSource,
   ByteTarget,
   ShortSource,
   ShortTarget,
   IntSource,
   IntTarget,
   TestsParityFlag,
   ModifiesParityFlag,
   TargetRegisterInOpcode,
   TargetRegisterInModRM,
   TargetRegisterIgnored,
   SourceRegisterInModRM,
   SourceRegisterIgnored,
   BranchOp,
   HasPopInstruction,
   IsPopInstruction,
   SourceOpTarget,
   HasDirectionBit,
   PushOp,
   PopOp,
   ShiftOp,
   RotateOp,
   SetsCCForCompare,
   SetsCCForTest,
   SupportsLockPrefix,
   SIMDSingleSource,
   DoubleWordSource,
   DoubleWordTarget,
   XMMSource,
   XMMTarget,
   PseudoOp,
   NeedsRepPrefix,
   NeedsLockPrefix,
   CallOp,
   SourceIsMemRef,
   SourceCanBeMemRef,
   SourceRegIsImplicit,
   TargetRegIsImplicit,
   FusableCompare,
   VectorIntMask,
   ZMMTarget,
   YMMSource,
   YMMTarget,
   ZMMSource,
   VectorLongMask,
   LongSource,
   LongTarget,
   LongImmediate,
   IsIA32Only,
   TestsSomeFlag,
   SetsSomeArithmeticFlag,
   Vector2Vector,
   Scalar2Vector,
   Vector2Scalar,
   SIMDNarrowing,
   SIMDExtension,
   V2V_RATIO_1x,
   V2V_RATIO_2x,
   V2V_RATIO_4x,
   V2V_RATIO_8x,
   Legacy,
   VEX128Supported,
   VEX256Supported,
   VEX_REQ_AVX512,
   EVEX128Supported,
   EVEX256Supported,
   EVEX512Supported,
   MinTargetSupported,
   SSE3Supported,
   SSE4Supported,
   SSE4_1Supported,
   LegacyMask,
   AVX,
   V128_AVX2,
   V256_AVX2,
   FMA,
   AVX512F,
   AVX512VL,
   AVX512BW,
   AVX512DQ,
   AVX512CD,
   AVX512VBMI2,
   AVX512BITALG,
   AVX512VPOPCNTDQ,
   NUM_INSN_FLAGS
   };

#define VEX128RequiresAVX              VEX128Supported , AVX
#define VEX128RequiresAVX2             VEX128Supported , V128_AVX2
#define VEX256RequiresAVX              VEX256Supported , AVX
#define VEX256RequiresAVX2             VEX256Supported , V256_AVX2
#define EVEX128RequiresAVX512F         EVEX128Supported, AVX512F
#define EVEX128RequiresAVX512VL        EVEX128Supported, AVX512VL
#define EVEX128RequiresAVX512BW        EVEX128Supported, AVX512BW
#define EVEX128RequiresAVX512DQ        EVEX128Supported, AVX512DQ
#define EVEX128RequiresAVX512CD        EVEX128Supported, AVX512CD
#define EVEX128RequiresAVX512VBMI2     EVEX128Supported, AVX512VBMI2
#define EVEX128RequiresAVX512BITALG    EVEX128Supported, AVX512BITALG
#define EVEX128RequiresAVX512VPOPCNTDQ EVEX128Supported, AVX512VPOPCNTDQ
#define EVEX256RequiresAVX512F         EVEX256Supported, AVX512F
#define EVEX256RequiresAVX512VL        EVEX256Supported, AVX512VL
#define EVEX256RequiresAVX512BW        EVEX256Supported, AVX512BW
#define EVEX256RequiresAVX512DQ        EVEX256Supported, AVX512DQ
#define EVEX256RequiresAVX512CD        EVEX256Supported, AVX512CD
#define EVEX256RequiresAVX512VBMI2     EVEX256Supported, AVX512VBMI2
#define EVEX256RequiresAVX512BITALG    EVEX256Supported, AVX512BITALG
#define EVEX256RequiresAVX512VPOPCNTDQ EVEX256Supported, AVX512VPOPCNTDQ
#define EVEX512RequiresAVX512F         EVEX512Supported, AVX512F
#define EVEX512RequiresAVX512BW        EVEX512Supported, AVX512BW
#define EVEX512RequiresAVX512DQ        EVEX512Supported, AVX512DQ
#define EVEX512RequiresAVX512CD        EVEX512Supported, AVX512CD
#define EVEX512RequiresAVX512VBMI2     EVEX512Supported, AVX512VBMI2
#define EVEX512RequiresAVX512BITALG    EVEX512Supported, AVX512BITALG
#define EVEX512RequiresAVX512VPOPCNTDQ EVEX512Supported, AVX512VPOPCNTDQ
#define VEX128RequiresFMA              VEX128Supported,  FMA
#define VEX256RequiresFMA              VEX256Supported,  FMA

template <typename T, uint32_t numFlags>
struct Flags
   {
   static constexpr std::size_t NUM_FLAGS = static_cast<std::size_t>(numFlags);
   static constexpr std::size_t NUM_UINT32S = (NUM_FLAGS + 31) / 32;
   uint32_t flags[NUM_UINT32S]{};

   constexpr Flags(const uint32_t a) : flags{a} {}
   constexpr Flags(const uint32_t a, const uint32_t b) : flags{a, b} {}
   constexpr Flags(const uint32_t a, const uint32_t b, const uint32_t c) : flags{a, b, c} {}
   constexpr Flags(const uint32_t a, const uint32_t b, const uint32_t c, const uint32_t d) : flags{a, b, c, d} {}

   bool test(T flag)
      {
      return flags[static_cast<uint32_t>(flag >> 32)] & (flag & 0xffffffff);
      }
   };

#define BINARY(value)
#define GET_MACRO(_1,_2,_3,_4,NAME,...) NAME

#define PROPERTY(word, value) (value / 32 == word ? value % 32 : 0)
#define PROPERTIES_4(word, a, b, c, d) PROPERTY(word, a) + PROPERTY(word, b) + PROPERTY(word, c) + PROPERTY(word, d)
#define PROPERTIES_3(word, a, b, c) PROPERTY(word, a) + PROPERTY(word, b) + PROPERTY(word, c)
#define PROPERTIES_2(word, a, b) PROPERTY(word, a) + PROPERTY(word, b)
#define PROPERTIES_1(word, a) PROPERTY(word, a)
#define PROPERTIES_0(word) 0


#define EXPAND(...) __VA_ARGS__
#define COUNT_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, N, ...) N
#define COUNT_ARGS(...) EXPAND(COUNT_ARGS_(__VA_ARGS__, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define CALL_MACRO_ON_EACH(a, ...) EXPAND(CALL_MACRO_ON_EACH_(COUNT_ARGS(__VA_ARGS__), a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_(N, a, ...) EXPAND(CALL_MACRO_ON_EACH__(N, a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH__(N, a, ...) EXPAND(CALL_MACRO_ON_EACH_##N(a, __VA_ARGS__))

#define CALL_MACRO_ON_EACH_1(a, X) PROPERTIES_IMPL(a, X)
#define CALL_MACRO_ON_EACH_2(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_1(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_3(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_2(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_4(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_3(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_5(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_4(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_6(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_5(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_7(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_6(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_8(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_7(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_9(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_8(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_10(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_9(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_11(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_10(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_12(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_11(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_13(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_12(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_14(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_13(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_15(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_14(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_16(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_15(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_17(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_16(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_18(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_17(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_19(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_18(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_20(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_19(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_21(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_20(a, __VA_ARGS__))
#define CALL_MACRO_ON_EACH_22(a, X, ...) PROPERTIES_IMPL(a, X) + EXPAND(CALL_MACRO_ON_EACH_21(a, __VA_ARGS__))


#define PROPERTIES_IMPL(word, ...) GET_MACRO(__VA_ARGS__, PROPERTIES_4, PROPERTIES_3, PROPERTIES_2, PROPERTIES_1, PROPERTIES_0)(word, __VA_ARGS__)
#define PROPERTIES(...) { CALL_MACRO_ON_EACH(0, __VA_ARGS__), CALL_MACRO_ON_EACH(1, __VA_ARGS__), CALL_MACRO_ON_EACH(2, __VA_ARGS__), CALL_MACRO_ON_EACH(3, __VA_ARGS__) }

#define INSTRUCTION(name, mnemonic, binary, props) props



const Flags<InsnFlags, NUM_INSN_FLAGS> props[] =
   {

         PROPERTIES(ModifiesTarget , SourceRegisterInModRM,
                    XMMSource , XMMTarget , SIMDSingleSource , SourceCanBeMemRef,
                    MinTargetSupported,
                    VEX128Supported, VEX256Supported ,
                    EVEX128Supported,
                    EVEX256Supported ,
                    EVEX512Supported )

//   PROPERTIES(ModifiesTarget , SourceRegisterInModRM,
//              XMMSource , XMMTarget , SIMDSingleSource , SourceCanBeMemRef,
//              MinTargetSupported ,
//              VEX128Supported , VEX128RequiresAVX , VEX256Supported , VEX256RequiresAVX ,
//              EVEX128Supported , EVEX128RequiresAVX512F , EVEX128RequiresAVX512VL ,
//              EVEX256Supported , EVEX256RequiresAVX512F , EVEX256RequiresAVX512VL ,
//              EVEX512Supported , EVEX512RequiresAVX512F)
   };


void test() {
//   uint32_t gg = PROPERTIES(AVX,AVX2);
//   Flags<InsnFlags, NUM_INSN_FLAGS> flags(PROPERTIES(AVX2, AVX512DQ), PROPERTIES(AVX2, AVX512DQ));

}

#endif //OMR_OMRFLAGS_HPP
