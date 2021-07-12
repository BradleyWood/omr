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

INSTANTIATE_TEST_CASE_P(Special, XDirectEncodingTest, ::testing::Values(
//      std::make_tuple(TR::InstOpCode::assocreg, TRTest::BinaryInstruction(), false),
      std::make_tuple(TR::InstOpCode::RET, 0x000000c3, false),
      std::make_tuple(TR::InstOpCode::RET, 0x000000c3, false)
));

INSTANTIATE_TEST_CASE_P(Special, XLabelEncodingTest, ::testing::Values(
      std::make_tuple(TR::InstOpCode::JMP4,  0x4, 0x000002eb),
      std::make_tuple(TR::InstOpCode::JMP4, -0x4, 0x0000faeb),
      std::make_tuple(TR::InstOpCode::JGE4,  0x4, 0x0000027d),
      std::make_tuple(TR::InstOpCode::JGE4, -0x4, 0x0000fa7d),
      std::make_tuple(TR::InstOpCode::JGE4,  0x0, 0x0000fe7d)
));
