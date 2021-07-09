/*******************************************************************************
 * Copyright (c) 2021, 2021 IBM Corp. and others
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

class XDirectEncodingTest : public TRTest::BinaryEncoderTest, public ::testing::WithParamInterface<std::tuple<TR::InstOpCode::Mnemonic, TRTest::BinaryInstruction>> {};

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
      std::make_tuple(TR::InstOpCode::UD2,  0x00000b0f),
      std::make_tuple(TR::InstOpCode::INT3, 0x000000cc),
      std::make_tuple(TR::InstOpCode::RET,  0x000000c3)
));

INSTANTIATE_TEST_CASE_P(Special, XLabelEncodingTest, ::testing::Values(
      std::make_tuple(TR::InstOpCode::JMP4,  0x4, 0x000002eb),
      std::make_tuple(TR::InstOpCode::JMP4, -0x4, 0x0000faeb),
      std::make_tuple(TR::InstOpCode::JGE4,  0x4, 0x0000027d),
      std::make_tuple(TR::InstOpCode::JGE4, -0x4, 0x0000fa7d),
      std::make_tuple(TR::InstOpCode::JGE4,  0x0, 0x0000fe7d)
));

INSTANTIATE_TEST_CASE_P(Branch, XRegRegEncodingTest, ::testing::ValuesIn(*TRTest::MakeVector<std::tuple<TR::InstOpCode::Mnemonic, TR::RealRegister::RegNum, TR::RealRegister::RegNum, TRTest::BinaryInstruction>>(
      std::make_tuple(TR::InstOpCode::XOR4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,  0x0000c033),
      std::make_tuple(TR::InstOpCode::XOR4RegReg,    TR::RealRegister::ecx, TR::RealRegister::ebp,  0x0000cd33),
      std::make_tuple(TR::InstOpCode::ADD4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,  0x0000c003),
      std::make_tuple(TR::InstOpCode::SUB4RegReg,    TR::RealRegister::eax, TR::RealRegister::eax,  0x0000c02b),
      std::make_tuple(TR::InstOpCode::IMUL4RegReg,   TR::RealRegister::eax, TR::RealRegister::eax,  0x00c0af0f),

      std::make_tuple(TR::InstOpCode::IMUL8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1af0f48),
      std::make_tuple(TR::InstOpCode::IMUL8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8af0f48),

      std::make_tuple(TR::InstOpCode::CMOVG8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14f0f48),
      std::make_tuple(TR::InstOpCode::CMOVG8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84f0f48),
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

      std::make_tuple(TR::InstOpCode::CMOVL8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14c0f48),
      std::make_tuple(TR::InstOpCode::CMOVL8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84c0f48),

      std::make_tuple(TR::InstOpCode::ADD8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c10348),
      std::make_tuple(TR::InstOpCode::ADD8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c80348),
      std::make_tuple(TR::InstOpCode::ADC8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c11348),
      std::make_tuple(TR::InstOpCode::ADC8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c81348),

      std::make_tuple(TR::InstOpCode::TEST8RegReg,     TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c18548),
      std::make_tuple(TR::InstOpCode::TEST8RegReg,     TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c88548),

      std::make_tuple(TR::InstOpCode::CMOVLE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14e0f48),
      std::make_tuple(TR::InstOpCode::CMOVLE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84e0f48),

      std::make_tuple(TR::InstOpCode::BSF8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1bc0f48),
      std::make_tuple(TR::InstOpCode::BSF8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8bc0f48),

      std::make_tuple(TR::InstOpCode::SHLD4RegRegCL,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c8a50f),
      std::make_tuple(TR::InstOpCode::SHLD4RegRegCL,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c1a50f),

      std::make_tuple(TR::InstOpCode::BSR8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1bd0f48),
      std::make_tuple(TR::InstOpCode::BSR8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8bd0f48),

      std::make_tuple(TR::InstOpCode::SUB8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c12b48),
      std::make_tuple(TR::InstOpCode::SUB8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c82b48),

      std::make_tuple(TR::InstOpCode::CMP8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c13b48),
      std::make_tuple(TR::InstOpCode::CMP8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c83b48),

      std::make_tuple(TR::InstOpCode::CMOVGE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc14d0f48),
      std::make_tuple(TR::InstOpCode::CMOVGE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc84d0f48),

      std::make_tuple(TR::InstOpCode::BT8RegReg,       TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc8a30f48),
      std::make_tuple(TR::InstOpCode::BT8RegReg,       TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc1a30f48),

      std::make_tuple(TR::InstOpCode::AND8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c12348),
      std::make_tuple(TR::InstOpCode::AND8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c82348),

      std::make_tuple(TR::InstOpCode::XOR8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c13348),
      std::make_tuple(TR::InstOpCode::XOR8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c83348),

      std::make_tuple(TR::InstOpCode::BTS4RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c8ab0f),
      std::make_tuple(TR::InstOpCode::BTS4RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c1ab0f),

      std::make_tuple(TR::InstOpCode::OR8RegReg,       TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c10b48),
      std::make_tuple(TR::InstOpCode::OR8RegReg,       TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c80b48),

      std::make_tuple(TR::InstOpCode::CMOVS8RegReg,    TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1480f48),
      std::make_tuple(TR::InstOpCode::CMOVS8RegReg,    TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8480f48),

      std::make_tuple(TR::InstOpCode::CMOVNE8RegReg,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0xc1450f48),
      std::make_tuple(TR::InstOpCode::CMOVNE8RegReg,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0xc8450f48),

      std::make_tuple(TR::InstOpCode::SBB8RegReg,      TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c11b48),
      std::make_tuple(TR::InstOpCode::SBB8RegReg,      TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c81b48),

      std::make_tuple(TR::InstOpCode::SHRD4RegRegCL,   TR::RealRegister::eax,  TR::RealRegister::ecx,  0x00c8ad0f),
      std::make_tuple(TR::InstOpCode::SHRD4RegRegCL,   TR::RealRegister::ecx,  TR::RealRegister::eax,  0x00c1ad0f)
)));
