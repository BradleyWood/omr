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

#ifndef CODEGENTEST_HPP
#define CODEGENTEST_HPP

#include "CompilerUnitTest.hpp"

#define INSTRUCTION_MAX_WORDS 4

namespace TRTest {

class CodeGenTest : public TRTest::CompilerUnitTest {
public:
    CodeGenTest() : CompilerUnitTest() {
        fakeNode = TR::Node::create(TR::treetop);
    }

    TR::CodeGenerator* cg() { return _comp.cg(); }
    TR::Node *fakeNode;
};

class BinaryInstruction {
public:
   std::vector<uint32_t> _words;

   BinaryInstruction() {}

   BinaryInstruction(uint32_t instr) {
      _words.push_back(instr);
   }

   BinaryInstruction(uint32_t prefix, uint32_t suffix) {
      _words.push_back(prefix);
      _words.push_back(suffix);
   }

   BinaryInstruction(std::vector<uint32_t> words) : _words(words) {}

   const std::vector<uint32_t>& words() const { return _words; }

   BinaryInstruction prepend(BinaryInstruction other) const {
      std::vector<uint32_t> new_words(other._words);
      for (size_t i = 0; i < _words.size(); i++)
         new_words.push_back(_words[i]);
      return new_words;
   }

   BinaryInstruction operator^(const BinaryInstruction& other) const {
      if (_words.size() != other._words.size())
         throw std::invalid_argument("Attempt to XOR instructions of different sizes");

      BinaryInstruction instr(_words);

      for (size_t i = 0; i < _words.size(); i++)
         instr._words[i] ^= other._words[i];

      return instr;
   }

   bool operator==(const BinaryInstruction& other) const {
      return other._words == _words;
   }
};

std::ostream& operator<<(std::ostream& os, const BinaryInstruction& instr) {
   os << "[";

   for (size_t i = 0; i < instr.words().size(); i++) {
      os << " 0x" << std::hex << std::setw(8) << std::setfill('0') << instr.words()[i];
   }

   os << " ]";

   return os;
}

class BinaryEncoderTest : public TRTest::CodeGenTest {
public:
   uint32_t buf[INSTRUCTION_MAX_WORDS];

   BinaryEncoderTest() {
      cg()->setBinaryBufferStart(reinterpret_cast<uint8_t*>(&getAlignedBuf()[0]));
   }

   uint32_t* getAlignedBuf();

   TRTest::BinaryInstruction getEncodedInstruction(size_t size, uint32_t off = 0) {
      int end = (int) ceil((off + size) / 4.0);
      return std::vector<uint32_t>(&getAlignedBuf()[off / 4], &getAlignedBuf()[end]);
   }

   TRTest::BinaryInstruction encodeInstruction(TR::Instruction *instr, uint32_t off = 0) {
      memset(buf, 0, INSTRUCTION_MAX_WORDS * sizeof(uint32_t));

      instr->estimateBinaryLength(0);
      cg()->setBinaryBufferCursor(reinterpret_cast<uint8_t*>(getAlignedBuf()) + off);

      instr->generateBinaryEncoding();
      return getEncodedInstruction(instr->getBinaryLength(), off);
   }

   TRTest::BinaryInstruction getBlankEncoding(TR::InstOpCode opCode, TR::CodeGenerator *cg);

   TRTest::BinaryInstruction findDifferingBits(TR::InstOpCode op1, TR::InstOpCode op2, TR::CodeGenerator *cg) {
      return getBlankEncoding(op1, cg) ^ getBlankEncoding(op2, cg);
   }

};

}

#endif
