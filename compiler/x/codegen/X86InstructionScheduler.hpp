/*******************************************************************************
 * Copyright IBM Corp. and others 2000
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

#ifndef OMR_X86_INSTRUCTION_SCHEDULER_INCL
#define OMR_X86_INSTRUCTION_SCHEDULER_INCL

#include <unordered_map>
#include "codegen/CodeGenerator.hpp"
#include "codegen/RealRegister.hpp"
#include "codegen/RegisterConstants.hpp"
#include "codegen/MemoryReference.hpp"
#include "x/codegen/X86Instruction.hpp"
#include "codegen/InstOpCode.hpp"

namespace TR
{
class X86MemInstruction;
}
namespace TR
{
class X86RegImmInstruction;
}
namespace TR
{
class X86RegInstruction;
}
namespace TR
{
class X86RegRegInstruction;
}
namespace TR
{
class X86RegMemInstruction;
}
namespace TR
{
class X86MemRegInstruction;
}
namespace TR
{
class X86RegRegMemInstruction;
}
namespace TR
{
class CodeGenerator;
}
namespace TR
{
class Instruction;
}
namespace TR
{
class LabelSymbol;
}
namespace TR
{
class MemoryReference;
}
namespace TR
{
class Register;
}

namespace OMR
{
namespace X86
{

enum X86InstructionLatency
   {
   LOW = 0,
   STANDARD = 1,
   MEDIUM = 2,
   HIGH = 3,
   HIGHEST = 4,
   };

class X86InstructionScheduler
   {

   TR::MemoryReference *getDestMemRef(TR::Instruction *instruction);

   TR::MemoryReference *getSourceMemRef(TR::Instruction *instruction);

   TR::RealRegister::RegNum getDestRegister(TR::Instruction *instruction);

   int32_t getSourceRegisters(TR::Instruction *instruction, TR::RealRegister::RegNum *regs);

   X86InstructionLatency getInstructionLatency(TR::Instruction *instruction);

   bool readsFromRegister(TR::Instruction *instruction, TR::RealRegister::RegNum reg);

   bool writesToRegister(TR::Instruction *instruction, TR::RealRegister::RegNum reg);

   bool writesToMem(TR::Instruction *instruction);

   bool readsFromMem(TR::Instruction *instruction);

   bool isControlFlowInstruction(TR::Instruction *instruction);

   bool hasMemoryConflict(TR::Instruction *instructionA, TR::Instruction *instructionB, TR::CodeGenerator *cg);

   bool hasRegisterDependency(TR::Instruction *instructionA, TR::Instruction *instructionB, TR::CodeGenerator *cg);

   bool hasDependency(TR::Instruction *instructionA, TR::Instruction *instructionB, TR::CodeGenerator *cg);

   std::vector<TR::Instruction *> getNextBlock(TR::Instruction *currentInstruction, TR::CodeGenerator *cg);

   void printBlock(std::vector<TR::Instruction *> block, TR::CodeGenerator *cg);

   void printDependencyGraph(std::vector<TR::Instruction *> block,
                             std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>> graph,
                             TR::CodeGenerator *cg);

   std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>>
   buildDependencyGraph(const std::vector<TR::Instruction *> &instrList, TR::CodeGenerator *cg);

   std::vector<TR::Instruction *>
   topologicalSort(const std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>> dependencyGraph,
                   const std::vector<TR::Instruction *> instructions);

   void scheduleBlock(std::vector<TR::Instruction *> block, TR::CodeGenerator *cg);

public:
   void perform(TR::CodeGenerator *cg);

   };
}
}

#endif
