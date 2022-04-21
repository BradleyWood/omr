/*******************************************************************************
 * Copyright (c) 2021, 2022 IBM Corp. and others
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

#include <gtest/gtest.h>
#include "../CodeGenTest.hpp"
#include "codegen/OMRX86Instruction.hpp"


class XDirectEncodingTest : public TRTest::BinaryEncoderTest<>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TRTest::BinaryInstruction>> {};

TEST_P(XDirectEncodingTest, encode) {
    auto instr = generateInstruction(std::get<0>(GetParam()), fakeNode, cg());

    ASSERT_EQ(std::get<1>(GetParam()), encodeInstruction(instr));
}

class XLabelEncodingTest : public TRTest::BinaryEncoderTest<>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, size_t, TRTest::BinaryInstruction>> {};

TEST_P(XLabelEncodingTest, encode) {
    auto label = generateLabelSymbol(cg());
    label->setCodeLocation(reinterpret_cast<uint8_t*>(getAlignedBuf()) + std::get<1>(GetParam()));

    auto instr = generateLabelInstruction(
        std::get<0>(GetParam()),
        fakeNode,
        label,
        cg()
    );

    ASSERT_EQ(
        std::get<2>(GetParam()),
        encodeInstruction(instr)
   );
}

class XRegRegEncodingTest : public TRTest::BinaryEncoderTest<>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TRTest::BinaryInstruction>> {};

TEST_P(XRegRegEncodingTest, encode) {
    auto regA = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto regB = cg()->machine()->getRealRegister(std::get<2>(GetParam()));

    auto instr = generateRegRegInstruction(std::get<0>(GetParam()), fakeNode, regA, regB, cg());

    TR::InstOpCode opcode = std::get<0>(GetParam());
    if (opcode.info().supportsAVX() && !cg()->comp()->target().cpu.supportsAVX()) {
        // For some reason, choosing SSE/AVX instruction variant is not decided until binary encoding
        // temporarily skip this test because if the hardware does support AVX
        return;
    }

    ASSERT_EQ(std::get<3>(GetParam()), encodeInstruction(instr));
}

class XRegMemEncodingTest : public TRTest::BinaryEncoderTest<>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, int32_t, TRTest::BinaryInstruction>> {};

TEST_P(XRegMemEncodingTest, encode) {
    auto regA = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto mrBaseReg = cg()->machine()->getRealRegister(std::get<2>(GetParam()));
    auto mrOffset = std::get<3>(GetParam());

    auto mr = generateX86MemoryReference(mrBaseReg, mrOffset, cg());
    auto instr = generateRegMemInstruction(std::get<0>(GetParam()), fakeNode, regA, mr, cg());

    TR::InstOpCode opcode = std::get<0>(GetParam());

    if (opcode.info().supportsAVX() && !cg()->comp()->target().cpu.supportsAVX()) {
        // For some reason, choosing SSE/AVX instruction variant is not decided until binary encoding
        // temporarily skip this test because if the hardware does support AVX
        return;
    }

    ASSERT_EQ(std::get<4>(GetParam()), encodeInstruction(instr));
}

INSTANTIATE_TEST_CASE_P(Special, XDirectEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::UD2,  "0f0b"),
    std::make_tuple(TR::InstOpCode::INT3, "cc"),
    std::make_tuple(TR::InstOpCode::RET,  "c3")
));

INSTANTIATE_TEST_CASE_P(Special, XLabelEncodingTest, ::testing::Values(
    std::make_tuple(TR::InstOpCode::JMP4,  0x4, "eb02"),
    std::make_tuple(TR::InstOpCode::JMP4, -0x4, "ebfa"),
    std::make_tuple(TR::InstOpCode::JGE4,  0x4, "7d02"),
    std::make_tuple(TR::InstOpCode::JGE4, -0x4, "7dfa"),
    std::make_tuple(TR::InstOpCode::JGE4,  0x0, "7dfe")
));

class XRegRegEncEncodingTest : public TRTest::BinaryEncoderTest<>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, OMR::X86::Encoding, TRTest::BinaryInstruction>> {};

TEST_P(XRegRegEncEncodingTest, encode) {
    auto regA = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto regB = cg()->machine()->getRealRegister(std::get<2>(GetParam()));
    auto enc = std::get<3>(GetParam());

    auto instr = generateRegRegInstruction(std::get<0>(GetParam()), fakeNode, regA, regB, cg(), enc);

    ASSERT_EQ(std::get<4>(GetParam()), encodeInstruction(instr));
}

INSTANTIATE_TEST_CASE_P(AVXSimdTest, XRegRegEncEncodingTest, ::testing::ValuesIn(*TRTest::MakeVector<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, OMR::X86::Encoding, TRTest::BinaryInstruction>>(
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::Legacy,    "660ffcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm2, TR::RealRegister::xmm1, OMR::X86::Legacy,    "660ffcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, OMR::X86::Legacy,    "660ffcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::VEX_L128,  "c5f1fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm2, TR::RealRegister::xmm1, OMR::X86::VEX_L128,  "c5e9fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, OMR::X86::VEX_L128,  "c5f1fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::VEX_L256,  "c5f5fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm2, TR::RealRegister::ymm1, OMR::X86::VEX_L256,  "c5edfcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm1, TR::RealRegister::ymm1, OMR::X86::VEX_L256,  "c5f5fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::EVEX_L128, "62f17508fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm2, TR::RealRegister::xmm1, OMR::X86::EVEX_L128, "62f16d08fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, OMR::X86::EVEX_L128, "62f17508fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm1, TR::RealRegister::ymm2, OMR::X86::EVEX_L256, "62f17528fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm2, TR::RealRegister::ymm1, OMR::X86::EVEX_L256, "62f16d28fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm1, TR::RealRegister::ymm1, OMR::X86::EVEX_L256, "62f17528fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::zmm1, TR::RealRegister::zmm2, OMR::X86::EVEX_L512, "62f17548fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::zmm2, TR::RealRegister::zmm1, OMR::X86::EVEX_L512, "62f16d48fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::zmm1, TR::RealRegister::zmm1, OMR::X86::EVEX_L512, "62f17548fcc9")
)));

class XRegRegRegEncEncodingTest : public TRTest::BinaryEncoderTest<>, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TR::RealRegister::RegNum, OMR::X86::Encoding, TRTest::BinaryInstruction>> {};

TEST_P(XRegRegRegEncEncodingTest, encode) {
    auto regA = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
    auto regB = cg()->machine()->getRealRegister(std::get<2>(GetParam()));
    auto regC = cg()->machine()->getRealRegister(std::get<3>(GetParam()));
    auto enc = std::get<4>(GetParam());

    auto instr = generateRegRegRegInstruction(std::get<0>(GetParam()), fakeNode, regA, regB, regC, cg(), enc);

    ASSERT_EQ(std::get<5>(GetParam()), encodeInstruction(instr));
}

INSTANTIATE_TEST_CASE_P(AVXSimdTest, XRegRegRegEncEncodingTest, ::testing::ValuesIn(*TRTest::MakeVector<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TR::RealRegister::RegNum, OMR::X86::Encoding, TRTest::BinaryInstruction>>(
    /* Cannot encode 3-operand forms as legacy SSE instruction */
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::VEX_L128,  "c5f1fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm2, TR::RealRegister::xmm2, TR::RealRegister::xmm1, OMR::X86::VEX_L128,  "c5e9fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, TR::RealRegister::xmm1, OMR::X86::VEX_L128,  "c5f1fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::VEX_L256,  "c5f5fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm2, TR::RealRegister::ymm2, TR::RealRegister::ymm1, OMR::X86::VEX_L256,  "c5edfcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm1, TR::RealRegister::ymm1, TR::RealRegister::ymm1, OMR::X86::VEX_L256,  "c5f5fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, TR::RealRegister::xmm2, OMR::X86::EVEX_L128, "62f17508fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm2, TR::RealRegister::xmm2, TR::RealRegister::xmm1, OMR::X86::EVEX_L128, "62f16d08fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::xmm1, TR::RealRegister::xmm1, TR::RealRegister::xmm1, OMR::X86::EVEX_L128, "62f17508fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm1, TR::RealRegister::ymm1, TR::RealRegister::ymm2, OMR::X86::EVEX_L256, "62f17528fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm2, TR::RealRegister::ymm2, TR::RealRegister::ymm1, OMR::X86::EVEX_L256, "62f16d28fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::ymm1, TR::RealRegister::ymm1, TR::RealRegister::ymm1, OMR::X86::EVEX_L256, "62f17528fcc9"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::zmm1, TR::RealRegister::zmm1, TR::RealRegister::zmm2, OMR::X86::EVEX_L512, "62f17548fcca"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::zmm2, TR::RealRegister::zmm2, TR::RealRegister::zmm1, OMR::X86::EVEX_L512, "62f16d48fcd1"),
    std::make_tuple(TR::InstOpCode::PADDBRegReg,    TR::RealRegister::zmm1, TR::RealRegister::zmm1, TR::RealRegister::zmm1, OMR::X86::EVEX_L512, "62f17548fcc9")
)));

INSTANTIATE_TEST_CASE_P(Branch, XRegRegEncodingTest, ::testing::ValuesIn(*TRTest::MakeVector<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TRTest::BinaryInstruction>>(
    std::make_tuple(TR::InstOpCode::XOR4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,    "33c0"),
    std::make_tuple(TR::InstOpCode::XOR4RegReg,    TR::RealRegister::ecx, TR::RealRegister::ebp,    "33cd"),
    std::make_tuple(TR::InstOpCode::ADD4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,    "03c0"),
    std::make_tuple(TR::InstOpCode::SUB4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,    "2bc0"),
    std::make_tuple(TR::InstOpCode::IMUL4RegReg,   TR::RealRegister::eax, TR::RealRegister::eax,    "0fafc0"),

    std::make_tuple(TR::InstOpCode::IMUL8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx, "480fafc1"),
    std::make_tuple(TR::InstOpCode::IMUL8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax, "480fafc8"),

    std::make_tuple(TR::InstOpCode::CMOVG8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx, "480f4fc1"),
    std::make_tuple(TR::InstOpCode::CMOVG8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax, "480f4fc8"),
    std::make_tuple(TR::InstOpCode::CMOVE8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx, "480f44c1"),
    std::make_tuple(TR::InstOpCode::CMOVE8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax, "480f44c8"),

    std::make_tuple(TR::InstOpCode::XCHG8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx, "4887c8"),
    std::make_tuple(TR::InstOpCode::XCHG8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax, "4887c1"),

    std::make_tuple(TR::InstOpCode::MOV8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "488bc1"),
    std::make_tuple(TR::InstOpCode::MOV8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "488bc8"),

    std::make_tuple(TR::InstOpCode::CMOVB8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx, "480f42c1"),
    std::make_tuple(TR::InstOpCode::CMOVB8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax, "480f42c8"),

    std::make_tuple(TR::InstOpCode::POPCNT8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx, "f3480fb8c1"),
    std::make_tuple(TR::InstOpCode::POPCNT8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax, "f3480fb8c8"),

    std::make_tuple(TR::InstOpCode::CMOVL8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx, "480f4cc1"),
    std::make_tuple(TR::InstOpCode::CMOVL8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax, "480f4cc8"),

    std::make_tuple(TR::InstOpCode::ADD8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "4803c1"),
    std::make_tuple(TR::InstOpCode::ADD8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "4803c8"),
    std::make_tuple(TR::InstOpCode::ADC8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "4813c1"),
    std::make_tuple(TR::InstOpCode::ADC8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "4813c8"),

    std::make_tuple(TR::InstOpCode::TEST8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx, "4885c1"),
    std::make_tuple(TR::InstOpCode::TEST8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax, "4885c8"),

    std::make_tuple(TR::InstOpCode::CMOVLE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx, "480f4ec1"),
    std::make_tuple(TR::InstOpCode::CMOVLE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax, "480f4ec8"),

    std::make_tuple(TR::InstOpCode::BSF8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "480fbcc1"),
    std::make_tuple(TR::InstOpCode::BSF8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "480fbcc8"),

    std::make_tuple(TR::InstOpCode::SHLD4RegRegCL,   TR::RealRegister::eax,  TR::RealRegister::ecx, "0fa5c8"),
    std::make_tuple(TR::InstOpCode::SHLD4RegRegCL,   TR::RealRegister::ecx,  TR::RealRegister::eax, "0fa5c1"),

    std::make_tuple(TR::InstOpCode::BSR8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "480fbdc1"),
    std::make_tuple(TR::InstOpCode::BSR8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "480fbdc8"),

    std::make_tuple(TR::InstOpCode::SUB8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "482bc1"),
    std::make_tuple(TR::InstOpCode::SUB8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "482bc8"),

    std::make_tuple(TR::InstOpCode::CMP8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "483bc1"),
    std::make_tuple(TR::InstOpCode::CMP8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "483bc8"),

    std::make_tuple(TR::InstOpCode::CMOVGE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx, "480f4dc1"),
    std::make_tuple(TR::InstOpCode::CMOVGE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax, "480f4dc8"),

    std::make_tuple(TR::InstOpCode::BT8RegReg,       TR::RealRegister::eax,  TR::RealRegister::ecx, "480fa3c8"),
    std::make_tuple(TR::InstOpCode::BT8RegReg,       TR::RealRegister::ecx,  TR::RealRegister::eax, "480fa3c1"),

    std::make_tuple(TR::InstOpCode::AND8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "4823c1"),
    std::make_tuple(TR::InstOpCode::AND8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "4823c8"),

    std::make_tuple(TR::InstOpCode::XOR8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "4833c1"),
    std::make_tuple(TR::InstOpCode::XOR8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "4833c8"),

    std::make_tuple(TR::InstOpCode::BTS4RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "0fabc8"),
    std::make_tuple(TR::InstOpCode::BTS4RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "0fabc1"),

    std::make_tuple(TR::InstOpCode::OR8RegReg,       TR::RealRegister::eax,  TR::RealRegister::ecx, "480bc1"),
    std::make_tuple(TR::InstOpCode::OR8RegReg,       TR::RealRegister::ecx,  TR::RealRegister::eax, "480bc8"),

    std::make_tuple(TR::InstOpCode::CMOVS8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx, "480f48c1"),
    std::make_tuple(TR::InstOpCode::CMOVS8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax, "480f48c8"),

    std::make_tuple(TR::InstOpCode::CMOVNE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx, "480f45c1"),
    std::make_tuple(TR::InstOpCode::CMOVNE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax, "480f45c8"),

    std::make_tuple(TR::InstOpCode::SBB8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx, "481bc1"),
    std::make_tuple(TR::InstOpCode::SBB8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax, "481bc8"),

    std::make_tuple(TR::InstOpCode::SHRD4RegRegCL,   TR::RealRegister::eax,  TR::RealRegister::ecx, "0fadc8"),
    std::make_tuple(TR::InstOpCode::SHRD4RegRegCL,   TR::RealRegister::ecx,  TR::RealRegister::eax, "0fadc1"),

    std::make_tuple(TR::InstOpCode::SQRTPDRegReg,   TR::RealRegister::xmm0,  TR::RealRegister::xmm1, "660f51c1"),
    std::make_tuple(TR::InstOpCode::SQRTPDRegReg,   TR::RealRegister::xmm9,  TR::RealRegister::xmm4, "66440f51cc"),

    std::make_tuple(TR::InstOpCode::VSQRTPDRegReg,   TR::RealRegister::xmm0,  TR::RealRegister::xmm1, "c5f951c1"),
    std::make_tuple(TR::InstOpCode::VSQRTPDRegReg,   TR::RealRegister::xmm9,  TR::RealRegister::xmm4, "c57951cc")
)));

INSTANTIATE_TEST_CASE_P(X86RegMem, XRegMemEncodingTest, ::testing::ValuesIn(*TRTest::MakeVector<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, int32_t, TRTest::BinaryInstruction>>(
     std::make_tuple(TR::InstOpCode::VSQRTPDRegMem, TR::RealRegister::xmm1,  TR::RealRegister::ecx, 0x8, "c5f9514908"),
     std::make_tuple(TR::InstOpCode::VSQRTPDRegMem, TR::RealRegister::xmm10, TR::RealRegister::eax, 0x0, "c5795110")
)));
