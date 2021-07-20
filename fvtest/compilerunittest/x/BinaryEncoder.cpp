/*******************************************************************************
 * Copyright (c) 2020, 2021 IBM Corp. and others
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

uint32_t* TRTest::BinaryEncoderTest::getAlignedBuf() {
   return reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(buf));
}

TRTest::BinaryInstruction TRTest::BinaryEncoderTest::getEncodedInstruction(size_t size, uint32_t off) {
   int end = (int) ceil((off + size) / 4.0);
   return std::vector<uint32_t>(&getAlignedBuf()[off / 4], &getAlignedBuf()[end]);
}

class XDirectEncodingTest : public TRTest::BinaryEncoderTest, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TRTest::BinaryInstruction, bool>> {};

TEST_P(XDirectEncodingTest, encode) {
   auto instr = generateInstruction(std::get<0>(GetParam()), fakeNode, cg());

   ASSERT_EQ(std::get<1>(GetParam()), encodeInstruction(instr));
}

class XLabelEncodingTest : public TRTest::BinaryEncoderTest, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, ssize_t, TRTest::BinaryInstruction>> {};

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

class XRegRegEncodingTest : public TRTest::BinaryEncoderTest, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TRTest::BinaryInstruction>> {};

TEST_P(XRegRegEncodingTest, encode) {
   auto regA = cg()->machine()->getRealRegister(std::get<1>(GetParam()));
   auto regB = cg()->machine()->getRealRegister(std::get<2>(GetParam()));

   auto instr = generateRegRegInstruction(std::get<0>(GetParam()), fakeNode, regA, regB, cg());

   ASSERT_EQ(std::get<3>(GetParam()), encodeInstruction(instr));
}

INSTANTIATE_TEST_CASE_P(Special, XDirectEncodingTest, ::testing::Values(
      std::make_tuple(TR::InstOpCode::RET,  0x000000c3, false),
      std::make_tuple(TR::InstOpCode::UD2,  0x00000b0f, false),
      std::make_tuple(TR::InstOpCode::INT3, 0x000000cc, false),
      std::make_tuple(TR::InstOpCode::RET,  0x000000c3, false)
));


INSTANTIATE_TEST_CASE_P(Special, XLabelEncodingTest, ::testing::Values(
      std::make_tuple(TR::InstOpCode::label,      -1, TRTest::BinaryInstruction()),
      std::make_tuple(TR::InstOpCode::JE4,         0, 0x0000fe74),
      std::make_tuple(TR::InstOpCode::JE4,        -4, 0x0000fa74),
      std::make_tuple(TR::InstOpCode::JE4,    0x7ffc, TRTest::BinaryInstruction(0x7ff6840f, 0x00000000)),
      std::make_tuple(TR::InstOpCode::JE4,   -0x8000, TRTest::BinaryInstruction(0x7ffa840f, 0x0000ffff)),
      std::make_tuple(TR::InstOpCode::JLE4,        0, 0x0000fe7e),
      std::make_tuple(TR::InstOpCode::JLE4,       -4, 0x0000fa7e),
      std::make_tuple(TR::InstOpCode::JLE4,   0x7ffc, TRTest::BinaryInstruction(0x7ff68e0f, 0x00000000)),
      std::make_tuple(TR::InstOpCode::JLE4,  -0x8000, TRTest::BinaryInstruction(0x7ffa8e0f, 0x0000ffff)),
      std::make_tuple(TR::InstOpCode::JGE4,        0, 0x0000fe7d),
      std::make_tuple(TR::InstOpCode::JGE4,       -4, 0x0000fa7d),
      std::make_tuple(TR::InstOpCode::JGE4,   0x7ffc, TRTest::BinaryInstruction(0x7ff68d0f, 0x00000000)),
      std::make_tuple(TR::InstOpCode::JGE4,  -0x8000, TRTest::BinaryInstruction(0x7ffa8d0f, 0x0000ffff)),
      std::make_tuple(TR::InstOpCode::JMP4,        0, 0x0000feeb),
      std::make_tuple(TR::InstOpCode::JMP4,       -4, 0x0000faeb),
      std::make_tuple(TR::InstOpCode::JMP4,   0x7ffc, TRTest::BinaryInstruction(0x007ff7e9, 0x00000000)),
      std::make_tuple(TR::InstOpCode::JMP4,  -0x8000, TRTest::BinaryInstruction(0xff7ffbe9, 0x000000ff))
));

INSTANTIATE_TEST_CASE_P(Branch, XRegRegEncodingTest, ::testing::ValuesIn(*TRTest::MakeVector<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TRTest::BinaryInstruction>>(
      std::make_tuple(TR::InstOpCode::XOR4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,  0x0000c033),
      std::make_tuple(TR::InstOpCode::XOR4RegReg,    TR::RealRegister::ecx, TR::RealRegister::ebp,  0x0000cd33),
      std::make_tuple(TR::InstOpCode::ADD4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,  0x0000c003),
      std::make_tuple(TR::InstOpCode::SUB4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,  0x0000c02b),
      std::make_tuple(TR::InstOpCode::IMUL4RegReg,   TR::RealRegister::eax, TR::RealRegister::eax,  0x00c0af0f),

      std::make_tuple(TR::InstOpCode::DIVSSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75e0ff3),
      std::make_tuple(TR::InstOpCode::DIVSSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85e0ff3),
      std::make_tuple(TR::InstOpCode::PMOVZXWDRegReg,  TR::RealRegister::xmm0, TR::RealRegister::xmm7, TRTest::BinaryInstruction(0x33380f66, 0x000000c7)),
      std::make_tuple(TR::InstOpCode::PMOVZXWDRegReg,  TR::RealRegister::xmm7, TR::RealRegister::xmm0, TRTest::BinaryInstruction(0x33380f66, 0x000000f8)),
      std::make_tuple(TR::InstOpCode::PADDDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7fe0f66),
      std::make_tuple(TR::InstOpCode::PADDDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8fe0f66),
      std::make_tuple(TR::InstOpCode::PADDBRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7fc0f66),
      std::make_tuple(TR::InstOpCode::PADDBRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8fc0f66),
      std::make_tuple(TR::InstOpCode::PMOVZXBDRegReg,  TR::RealRegister::xmm0, TR::RealRegister::xmm7, TRTest::BinaryInstruction(0x31380f66, 0x000000c7)),
      std::make_tuple(TR::InstOpCode::PMOVZXBDRegReg,  TR::RealRegister::xmm7, TR::RealRegister::xmm0, TRTest::BinaryInstruction(0x31380f66, 0x000000f8)),
      std::make_tuple(TR::InstOpCode::IMUL8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1af0f48),
      std::make_tuple(TR::InstOpCode::IMUL8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8af0f48),
      std::make_tuple(TR::InstOpCode::PMULLWRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7d50f66),
      std::make_tuple(TR::InstOpCode::PMULLWRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8d50f66),
      std::make_tuple(TR::InstOpCode::MULPDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7590f66),
      std::make_tuple(TR::InstOpCode::MULPDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8590f66),
      std::make_tuple(TR::InstOpCode::DIVSDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75e0ff2),
      std::make_tuple(TR::InstOpCode::DIVSDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85e0ff2),
      std::make_tuple(TR::InstOpCode::CMOVG8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14f0f48),
      std::make_tuple(TR::InstOpCode::CMOVG8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84f0f48),
      std::make_tuple(TR::InstOpCode::MULSSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7590ff3),
      std::make_tuple(TR::InstOpCode::MULSSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8590ff3),
      std::make_tuple(TR::InstOpCode::CMOVE8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1440f48),
      std::make_tuple(TR::InstOpCode::CMOVE8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8440f48),
      std::make_tuple(TR::InstOpCode::XCHG8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c88748),
      std::make_tuple(TR::InstOpCode::XCHG8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c18748),
      std::make_tuple(TR::InstOpCode::MOV8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c18b48),
      std::make_tuple(TR::InstOpCode::MOV8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c88b48),
      std::make_tuple(TR::InstOpCode::CMOVB8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1420f48),
      std::make_tuple(TR::InstOpCode::CMOVB8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8420f48),
      std::make_tuple(TR::InstOpCode::POPCNT8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx, TRTest::BinaryInstruction(0xb80f48f3, 0x000000c1)),
      std::make_tuple(TR::InstOpCode::POPCNT8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax, TRTest::BinaryInstruction(0xb80f48f3, 0x000000c8)),
      std::make_tuple(TR::InstOpCode::VMOVDQURegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc76ffac5),
      std::make_tuple(TR::InstOpCode::VMOVDQURegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf86ffac5),
      std::make_tuple(TR::InstOpCode::PMULLDRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, TRTest::BinaryInstruction(0x40380f66, 0x000000c7)),
      std::make_tuple(TR::InstOpCode::PMULLDRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, TRTest::BinaryInstruction(0x40380f66, 0x000000f8)),
      std::make_tuple(TR::InstOpCode::SUBSDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75c0ff2),
      std::make_tuple(TR::InstOpCode::SUBSDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85c0ff2),
      std::make_tuple(TR::InstOpCode::CMOVL8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14c0f48),
      std::make_tuple(TR::InstOpCode::CMOVL8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84c0f48),
      std::make_tuple(TR::InstOpCode::ADD8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c10348),
      std::make_tuple(TR::InstOpCode::ADD8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c80348),
      std::make_tuple(TR::InstOpCode::ADC8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c11348),
      std::make_tuple(TR::InstOpCode::ADC8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c81348),
      std::make_tuple(TR::InstOpCode::PCMPGTBRegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7640f66),
      std::make_tuple(TR::InstOpCode::PCMPGTBRegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8640f66),
      std::make_tuple(TR::InstOpCode::MULSDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7590ff2),
      std::make_tuple(TR::InstOpCode::MULSDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8590ff2),

      std::make_tuple(TR::InstOpCode::TEST8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c88548),
      std::make_tuple(TR::InstOpCode::TEST8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c18548),

      std::make_tuple(TR::InstOpCode::PUNPCKHBWRegReg, TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7680f66),
      std::make_tuple(TR::InstOpCode::PUNPCKHBWRegReg, TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8680f66),
      std::make_tuple(TR::InstOpCode::MOVDQURegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc76f0ff3),
      std::make_tuple(TR::InstOpCode::MOVDQURegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf86f0ff3),
      std::make_tuple(TR::InstOpCode::PSHUFBRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, TRTest::BinaryInstruction(0x00380f66, 0x000000c7)),
      std::make_tuple(TR::InstOpCode::PSHUFBRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, TRTest::BinaryInstruction(0x00380f66, 0x000000f8)),
      std::make_tuple(TR::InstOpCode::SQRTSSRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7510ff3),
      std::make_tuple(TR::InstOpCode::SQRTSSRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8510ff3),
      std::make_tuple(TR::InstOpCode::SUBSSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75c0ff3),
      std::make_tuple(TR::InstOpCode::SUBSSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85c0ff3),
      std::make_tuple(TR::InstOpCode::CMOVLE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14e0f48),
      std::make_tuple(TR::InstOpCode::CMOVLE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84e0f48),
      std::make_tuple(TR::InstOpCode::PADDWRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7fd0f66),
      std::make_tuple(TR::InstOpCode::PADDWRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8fd0f66),
      std::make_tuple(TR::InstOpCode::BSF8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1bc0f48),
      std::make_tuple(TR::InstOpCode::BSF8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8bc0f48),
      std::make_tuple(TR::InstOpCode::ADDSDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7580ff2),
      std::make_tuple(TR::InstOpCode::ADDSDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8580ff2),
      std::make_tuple(TR::InstOpCode::PADDQRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7d40f66),
      std::make_tuple(TR::InstOpCode::PADDQRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8d40f66),
      std::make_tuple(TR::InstOpCode::SHLD4RegRegCL,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c8a50f),
      std::make_tuple(TR::InstOpCode::SHLD4RegRegCL,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c1a50f),
      std::make_tuple(TR::InstOpCode::PUNPCKLBWRegReg, TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7600f66),
      std::make_tuple(TR::InstOpCode::PUNPCKLBWRegReg, TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8600f66),
      std::make_tuple(TR::InstOpCode::PANDNRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7df0f66),
      std::make_tuple(TR::InstOpCode::PANDNRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8df0f66),
      std::make_tuple(TR::InstOpCode::SUBPDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75c0f66),
      std::make_tuple(TR::InstOpCode::SUBPDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85c0f66),
      std::make_tuple(TR::InstOpCode::BSR8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1bd0f48),
      std::make_tuple(TR::InstOpCode::BSR8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8bd0f48),
      std::make_tuple(TR::InstOpCode::MOVSSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7100ff3),
      std::make_tuple(TR::InstOpCode::MOVSSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8100ff3),
      std::make_tuple(TR::InstOpCode::XORPDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7570f66),
      std::make_tuple(TR::InstOpCode::XORPDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8570f66),
      std::make_tuple(TR::InstOpCode::PSUBQRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7fb0f66),
      std::make_tuple(TR::InstOpCode::PSUBQRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8fb0f66),
      std::make_tuple(TR::InstOpCode::SUB8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c12b48),
      std::make_tuple(TR::InstOpCode::SUB8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c82b48),
      std::make_tuple(TR::InstOpCode::MOVAPSRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c7280f),
      std::make_tuple(TR::InstOpCode::MOVAPSRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f8280f),
      std::make_tuple(TR::InstOpCode::ADDSSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7580ff3),
      std::make_tuple(TR::InstOpCode::ADDSSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8580ff3),
      std::make_tuple(TR::InstOpCode::MOVUPDRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7100f66),
      std::make_tuple(TR::InstOpCode::MOVUPDRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8100f66),
      std::make_tuple(TR::InstOpCode::CMP8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c13b48),
      std::make_tuple(TR::InstOpCode::CMP8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c83b48),
      std::make_tuple(TR::InstOpCode::PSUBWRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7f90f66),
      std::make_tuple(TR::InstOpCode::PSUBWRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8f90f66),
      std::make_tuple(TR::InstOpCode::CVTSD2SSRegReg,  TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75a0ff2),
      std::make_tuple(TR::InstOpCode::CVTSD2SSRegReg,  TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85a0ff2),
      std::make_tuple(TR::InstOpCode::PACKUSWBRegReg,  TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7670f66),
      std::make_tuple(TR::InstOpCode::PACKUSWBRegReg,  TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8670f66),
      std::make_tuple(TR::InstOpCode::SUBPSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c75c0f),
      std::make_tuple(TR::InstOpCode::SUBPSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f85c0f),
      std::make_tuple(TR::InstOpCode::PCMPGTWRegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7650f66),
      std::make_tuple(TR::InstOpCode::PCMPGTWRegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8650f66),
      std::make_tuple(TR::InstOpCode::CMOVGE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14d0f48),
      std::make_tuple(TR::InstOpCode::CMOVGE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84d0f48),
      std::make_tuple(TR::InstOpCode::CVTSS2SDRegReg,  TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75a0ff3),
      std::make_tuple(TR::InstOpCode::CVTSS2SDRegReg,  TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85a0ff3),
      std::make_tuple(TR::InstOpCode::ADDPDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7580f66),
      std::make_tuple(TR::InstOpCode::ADDPDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8580f66),
      std::make_tuple(TR::InstOpCode::MOVSDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7100ff2),
      std::make_tuple(TR::InstOpCode::MOVSDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8100ff2),
      std::make_tuple(TR::InstOpCode::XORPSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c7570f),
      std::make_tuple(TR::InstOpCode::XORPSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f8570f),
      std::make_tuple(TR::InstOpCode::BT8RegReg,       TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc8a30f48),
      std::make_tuple(TR::InstOpCode::BT8RegReg,       TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc1a30f48),
      std::make_tuple(TR::InstOpCode::MOVAPDRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7280f66),
      std::make_tuple(TR::InstOpCode::MOVAPDRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8280f66),
      std::make_tuple(TR::InstOpCode::PORRegReg,       TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7eb0f66),
      std::make_tuple(TR::InstOpCode::PORRegReg,       TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8eb0f66),
      std::make_tuple(TR::InstOpCode::PANDRegReg,      TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7db0f66),
      std::make_tuple(TR::InstOpCode::PANDRegReg,      TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8db0f66),
      std::make_tuple(TR::InstOpCode::AND8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c12348),
      std::make_tuple(TR::InstOpCode::AND8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c82348),
      std::make_tuple(TR::InstOpCode::SQRTSDRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7510ff2),
      std::make_tuple(TR::InstOpCode::SQRTSDRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8510ff2),
      std::make_tuple(TR::InstOpCode::XOR8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c13348),
      std::make_tuple(TR::InstOpCode::XOR8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c83348),
      std::make_tuple(TR::InstOpCode::PXORRegReg,      TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7ef0f66),
      std::make_tuple(TR::InstOpCode::PXORRegReg,      TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8ef0f66),
      std::make_tuple(TR::InstOpCode::BTS4RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c8ab0f),
      std::make_tuple(TR::InstOpCode::BTS4RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c1ab0f),
      std::make_tuple(TR::InstOpCode::ADDPSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c7580f),
      std::make_tuple(TR::InstOpCode::ADDPSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f8580f),
      std::make_tuple(TR::InstOpCode::PTESTRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, TRTest::BinaryInstruction(0x17380f66, 0x000000c7)),
      std::make_tuple(TR::InstOpCode::PTESTRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, TRTest::BinaryInstruction(0x17380f66, 0x000000f8)),
      std::make_tuple(TR::InstOpCode::OR8RegReg,       TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c10b48),
      std::make_tuple(TR::InstOpCode::OR8RegReg,       TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c80b48),
      std::make_tuple(TR::InstOpCode::CMOVS8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1480f48),
      std::make_tuple(TR::InstOpCode::CMOVS8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8480f48),
      std::make_tuple(TR::InstOpCode::UCOMISSRegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c72e0f),
      std::make_tuple(TR::InstOpCode::UCOMISSRegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f82e0f),
      std::make_tuple(TR::InstOpCode::DIVPSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c75e0f),
      std::make_tuple(TR::InstOpCode::DIVPSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f85e0f),
      std::make_tuple(TR::InstOpCode::ANDNPDRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7550f66),
      std::make_tuple(TR::InstOpCode::ANDNPDRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8550f66),
      std::make_tuple(TR::InstOpCode::PCMPEQWRegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7750f66),
      std::make_tuple(TR::InstOpCode::PCMPEQWRegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8750f66),
      std::make_tuple(TR::InstOpCode::CMOVNE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1450f48),
      std::make_tuple(TR::InstOpCode::CMOVNE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8450f48),
      std::make_tuple(TR::InstOpCode::MOVQRegReg8,     TR::RealRegister::xmm0, TR::RealRegister::r10,  TRTest::BinaryInstruction(0x6e0f4966, 0x000000c2)),
      std::make_tuple(TR::InstOpCode::MOVQRegReg8,     TR::RealRegister::xmm7, TR::RealRegister::r10,  TRTest::BinaryInstruction(0x6e0f4966, 0x000000fa)),
      std::make_tuple(TR::InstOpCode::MULPSRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c7590f),
      std::make_tuple(TR::InstOpCode::MULPSRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f8590f),
      std::make_tuple(TR::InstOpCode::PSUBBRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7f80f66),
      std::make_tuple(TR::InstOpCode::PSUBBRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8f80f66),
      std::make_tuple(TR::InstOpCode::SBB8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c11b48),
      std::make_tuple(TR::InstOpCode::SBB8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c81b48),
      std::make_tuple(TR::InstOpCode::SHRD4RegRegCL,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c8ad0f),
      std::make_tuple(TR::InstOpCode::SHRD4RegRegCL,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c1ad0f),
      std::make_tuple(TR::InstOpCode::PSUBDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7fa0f66),
      std::make_tuple(TR::InstOpCode::PSUBDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8fa0f66),
      std::make_tuple(TR::InstOpCode::UCOMISDRegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc72e0f66),
      std::make_tuple(TR::InstOpCode::UCOMISDRegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf82e0f66),
      std::make_tuple(TR::InstOpCode::DIVPDRegReg,     TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc75e0f66),
      std::make_tuple(TR::InstOpCode::DIVPDRegReg,     TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf85e0f66),
      std::make_tuple(TR::InstOpCode::MOVUPSRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c7100f),
      std::make_tuple(TR::InstOpCode::MOVUPSRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f8100f),
      std::make_tuple(TR::InstOpCode::ANDNPSRegReg,    TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0x00c7550f),
      std::make_tuple(TR::InstOpCode::ANDNPSRegReg,    TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0x00f8550f),
      std::make_tuple(TR::InstOpCode::PCMPEQBRegReg,   TR::RealRegister::xmm0, TR::RealRegister::xmm7, 0xc7740f66),
      std::make_tuple(TR::InstOpCode::PCMPEQBRegReg,   TR::RealRegister::xmm7, TR::RealRegister::xmm0, 0xf8740f66)
)));
