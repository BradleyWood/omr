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

#include <unordered_map>
#include <queue>

#include "X86InstructionScheduler.hpp"

#include "codegen/RealRegister.hpp"
#include "codegen/MemoryReference.hpp"
#include "codegen/RegisterConstants.hpp"
#include "x/codegen/X86Instruction.hpp"
#include "codegen/InstOpCode.hpp"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/processor-flags.h>
#include <asm/msr.h>

#define MAX_SOURCE_REGISTERS 6 /* Worst Case RegMaskRegMem = 1 mask src, 1 src, mr: 1 base, 1 index reg */


namespace OMR
{
namespace X86
{
TR::MemoryReference *
X86InstructionScheduler::getDestMemRef(TR::Instruction *instruction)
   {
   TR::MemoryReference *mr = instruction->getMemoryReference();

   if (!instruction->getOpCode().sourceIsMemRef() && instruction->getOpCode().modifiesTarget())
      return mr;

   return NULL;
   }

TR::MemoryReference *
X86InstructionScheduler::getSourceMemRef(TR::Instruction *instruction)
   {
   TR::MemoryReference *mr = instruction->getMemoryReference();

   if (instruction->getOpCode().sourceIsMemRef())
      return mr;

   return NULL;
   }

TR::RealRegister::RegNum
X86InstructionScheduler::getDestRegister(TR::Instruction *instruction)
   {
   TR::Register *targetReg = instruction->getTargetRegister();

   if (targetReg)
      {
      return targetReg->getRealRegister()->getRegisterNumber();
      }

   return TR::RealRegister::NoReg;
   }

int32_t
X86InstructionScheduler::getSourceRegisters(TR::Instruction *instruction, TR::RealRegister::RegNum *sourceRegs)
   {
   TR::MemoryReference *mr = instruction->getMemoryReference();
   int32_t numRegs = 0;

#define ADD_REG_IF_VALID(reg)                                                                 \
   if (reg && reg->getRealRegister()->getRegisterNumber() != TR::RealRegister::NoReg) \
      {                                                                                       \
      sourceRegs[numRegs++] = reg->getRealRegister()->getRegisterNumber();                    \
      }

   ADD_REG_IF_VALID(instruction->getSourceRegister());
   ADD_REG_IF_VALID(instruction->getSource2ndRegister());
   ADD_REG_IF_VALID(instruction->getMaskRegister());

   if (instruction->getOpCode().usesTarget())
      {
      ADD_REG_IF_VALID(instruction->getTargetRegister());
      }

   if (mr)
      {
      // Ignore address register (mr->getAddressRegister()) as it would be used in another instruction
      ADD_REG_IF_VALID(mr->getBaseRegister());
      ADD_REG_IF_VALID(mr->getIndexRegister());
      ADD_REG_IF_VALID(mr->getAddressRegister());
      }

#undef ADD_REG_IF_VALID

   return numRegs;
   }

X86InstructionLatency
X86InstructionScheduler::getInstructionLatency(TR::Instruction *instruction)
   {
   X86InstructionLatency latency;

   switch (instruction->getOpCodeValue())
      {
      // LOWEST LATENCY
      case TR::InstOpCode::MOV4RegImm4:
      case TR::InstOpCode::MOV8RegImm4:
      case TR::InstOpCode::MOV4RegReg:
      case TR::InstOpCode::MOV8RegReg:

      case TR::InstOpCode::TEST4RegReg:
      case TR::InstOpCode::TEST8RegReg:
      case TR::InstOpCode::TEST4RegImm4:
      case TR::InstOpCode::TEST8RegImm4:

      case TR::InstOpCode::CMP4RegReg:
      case TR::InstOpCode::CMP8RegReg:
      case TR::InstOpCode::CMP4RegImm4:
      case TR::InstOpCode::CMP4RegImms:
      case TR::InstOpCode::CMP8RegImm4:
      case TR::InstOpCode::CMP8RegImms:
         latency = LOW;
         break;

         // MEDIUM LATENCY

      case TR::InstOpCode::XCHG1RegReg:
      case TR::InstOpCode::XCHG2RegReg:
      case TR::InstOpCode::XCHG4RegReg:
      case TR::InstOpCode::XCHG8RegReg:
      case TR::InstOpCode::XCHG2AccReg:
      case TR::InstOpCode::XCHG4AccReg:
      case TR::InstOpCode::XCHG8AccReg:
         latency = MEDIUM;
         break;

         // HIGH LATENCY

      case TR::InstOpCode::MUL1AccReg:
      case TR::InstOpCode::MUL2AccReg:
      case TR::InstOpCode::MUL4AccReg:
      case TR::InstOpCode::MUL8AccReg:

      case TR::InstOpCode::IMUL1AccReg:
      case TR::InstOpCode::IMUL2AccReg:
      case TR::InstOpCode::IMUL4AccReg:
      case TR::InstOpCode::IMUL8AccReg:

      case TR::InstOpCode::IMUL2RegRegImm2:
      case TR::InstOpCode::IMUL2RegRegImms:
      case TR::InstOpCode::IMUL4RegRegImm4:
      case TR::InstOpCode::IMUL4RegRegImms:
      case TR::InstOpCode::IMUL8RegRegImm4:
      case TR::InstOpCode::IMUL8RegRegImms:

      case TR::InstOpCode::CMPPSRegRegImm1:
      case TR::InstOpCode::CMPPDRegRegImm1:
         latency = HIGH;
         break;

         // HIGHEST LATENCY
      case TR::InstOpCode::IDIV1AccReg:
      case TR::InstOpCode::IDIV1AccMem:
      case TR::InstOpCode::IDIV2AccReg:
      case TR::InstOpCode::IDIV2AccMem:
      case TR::InstOpCode::IDIV4AccReg:
      case TR::InstOpCode::IDIV4AccMem:
      case TR::InstOpCode::IDIV8AccReg:
      case TR::InstOpCode::IDIV8AccMem:
      case TR::InstOpCode::DIV4AccReg:
      case TR::InstOpCode::DIV4AccMem:
      case TR::InstOpCode::DIV8AccMem:

      case TR::InstOpCode::POPCNT4RegReg:
      case TR::InstOpCode::POPCNT8RegReg:

      case TR::InstOpCode::DIVSSRegReg:
      case TR::InstOpCode::DIVSSRegMem:
      case TR::InstOpCode::DIVPSRegReg:
      case TR::InstOpCode::DIVPDRegMem:

      case TR::InstOpCode::SQRTSSRegReg:
      case TR::InstOpCode::SQRTSDRegReg:
      case TR::InstOpCode::SQRTPSRegReg:
      case TR::InstOpCode::SQRTPDRegReg:
         latency = HIGHEST;
         break;

      default:
         // Most instructions will be classified standard
         latency = STANDARD;
      }

   if (instruction->getMemoryReference())
      return latency > HIGH ? latency : HIGH;

   return latency;
   }

bool
X86InstructionScheduler::readsFromRegister(TR::Instruction *instruction, TR::RealRegister::RegNum reg)
   {
//   if (instruction->getOpCode().isPopOp() || instruction->getOpCode().isPushOp())
//      {
//      // PUSH/POP instructions modify stack pointer;
//      // Pop instructions modify additional register;
//      TR::RealRegister::RegNum popDest = getDestRegister(instruction);
//      return reg == TR::RealRegister::esp || (popDest != TR::RealRegister::NoReg && reg == popDest);
//      }

   if (instruction->getOpCode().sourceRegIsImplicit())
      {
      // Not too many instructions with implicit target
      // Be conservative;
      // todo: figure out which instructions use which implicit registers
      switch (reg)
         {
         case TR::RealRegister::eax:
         case TR::RealRegister::ebx:
            // Might only happen for cpuid instructions which the compiler does not use;
         case TR::RealRegister::ecx:
            // REP instructions ?
         case TR::RealRegister::edx:
         case TR::RealRegister::esi:
            // String operations?
         case TR::RealRegister::edi:
            // String operations?
         case TR::RealRegister::esp:
            // I think for PUSH, POP, CALL, RET, ENTER, LEAVE
            // We can probably ignore control flow instructions
            // Because we will not schedule instructions across control flow
            return true;
         default:
            break;
         }
      }

   if (reg == TR::RealRegister::NoReg)
      return false;

   TR::RealRegister::RegNum srcRegs[MAX_SOURCE_REGISTERS];
   int32_t numSrcRegs = getSourceRegisters(instruction, srcRegs);

   for (int32_t i = 0; i < numSrcRegs; i++)
      {
      if (reg == TR::RealRegister::RegNum::vfp && TR::RealRegister::RegNum::esp == srcRegs[i])
         return true;
      if (reg == srcRegs[i])
         return true;
      }

   return false;
   }

// todo;
bool
X86InstructionScheduler::writesToRegister(TR::Instruction *instruction, TR::RealRegister::RegNum reg)
   {
   if (instruction->getOpCode().isPopOp() || instruction->getOpCode().isPushOp())
      {
      // PUSH/POP instructions modify stack pointer;
      // Pop instructions modify additional register;
      TR::RealRegister::RegNum popDest = getDestRegister(instruction);
      return reg == TR::RealRegister::esp || (popDest != TR::RealRegister::NoReg && reg == popDest);
      }
   else if (instruction->getOpCode().targetRegIsImplicit())
      {
      // Not too many instructions with implicit target
      // Be conservative;
      // todo: figure out which instructions use which implicit registers
      switch (reg)
         {
         case TR::RealRegister::eax:
         case TR::RealRegister::ebx:
            // Might only happen for cpuid instructions which the compiler does not use;
         case TR::RealRegister::ecx:
            // REP instructions ?
         case TR::RealRegister::edx:
         case TR::RealRegister::esi:
            // String operations?
         case TR::RealRegister::edi:
            // String operations?
         case TR::RealRegister::esp:
            // I think for PUSH, POP, CALL, RET, ENTER, LEAVE
            // We can probably ignore control flow instructions
            // Because we will not schedule instructions across control flow
            return true;
         default:
            break;
         }
      }

   if (reg == TR::RealRegister::NoReg)
      return false;

   TR::RealRegister::RegNum destReg = getDestRegister(instruction);

   if (reg == TR::RealRegister::RegNum::vfp && TR::RealRegister::RegNum::esp == destReg)
      return true;

   if (destReg == TR::RealRegister::NoReg)
      return false;

   return destReg == reg;
   }

bool
X86InstructionScheduler::writesToMem(TR::Instruction *instruction)
   {
   switch (instruction->getKind())
      {
      case OMR::Instruction::IsMemImm:
         // Could possibly be a read like CMP4MemImm4
      case OMR::Instruction::IsMemImmSym:
      case OMR::Instruction::IsMemImmSnippet:
      case OMR::Instruction::IsMemReg:
      case OMR::Instruction::IsMemMaskReg:
      case OMR::Instruction::IsMemRegImm:
      case OMR::Instruction::IsFPMemReg:
         return true;
      default:
         return false;
      }
   }

bool
X86InstructionScheduler::readsFromMem(TR::Instruction *instruction)
   {
   switch (instruction->getKind())
      {
      case OMR::Instruction::IsRegMem:
      case OMR::Instruction::IsRegMemImm:
      case OMR::Instruction::IsRegRegMem:
      case OMR::Instruction::IsRegMaskMem:
      case OMR::Instruction::IsRegMaskRegMem:
      case OMR::Instruction::IsFPRegMem:
      case OMR::Instruction::IsMem:
      case OMR::Instruction::IsMemTable:  // I assume this is a jump table; so probably not
      case OMR::Instruction::IsMemImm:    // Possible?
      case OMR::Instruction::IsMemImmSym: // Possible?
         return true;
      case OMR::Instruction::IsCallMem:
         // Technically possible conflict, but no generated code will write to the address of a call in the same block
         // I will leave  this case here so its not forgotten
      default:
         return false;
      }
   }

bool
X86InstructionScheduler::isControlFlowInstruction(TR::Instruction *instruction)
   {
   // todo; Should we consider call instructions to not be control flow?
   // Flow returns back to the same block after the call;
   // Call instructions may modify registers;

   // todo; Some of these instruction kinds can be used without control flow;

   switch (instruction->getKind())
      {
      case OMR::Instruction::IsLabel:
      case OMR::Instruction::IsVirtualGuardNOP:
      case OMR::Instruction::IsFence:
      case OMR::Instruction::IsPadding:
      case OMR::Instruction::IsAlignment:
      case OMR::Instruction::IsBoundaryAvoidance:
      case OMR::Instruction::IsPatchableCodeAlignment:
      case OMR::Instruction::IsImm:
      case OMR::Instruction::IsImmSnippet:
      case OMR::Instruction::IsImmSym:
      case OMR::Instruction::IsImm64:
      case OMR::Instruction::IsImm64Sym:
      case OMR::Instruction::IsVFPSave:
      case OMR::Instruction::IsVFPRestore:
      case OMR::Instruction::IsVFPDedicate:
      case OMR::Instruction::IsVFPRelease:
      case OMR::Instruction::IsVFPCallCleanup:
      case OMR::Instruction::IsRegImmSym:
      case OMR::Instruction::IsRegImm64:
      case OMR::Instruction::IsRegImm64Sym:
      case OMR::Instruction::IsMemTable:
      case OMR::Instruction::IsCallMem:
      case OMR::Instruction::IsMemImmSym:
      case OMR::Instruction::IsMemImmSnippet:
         return true;
      default:
         break;
      }

   switch (instruction->getOpCodeValue())
      {
      case TR::InstOpCode::retn:
      case TR::InstOpCode::RET:
      case TR::InstOpCode::RETImm2:
         return true;
      default:
         break;
      }

   return instruction->getOpCode().isBranchOp() || instruction->getOpCode().isCallOp() ||
          instruction->getOpCode().isPseudoOp();
   }

bool
X86InstructionScheduler::hasMemoryConflict(TR::Instruction *instructionA, TR::Instruction *instructionB,
                                           TR::CodeGenerator *cg)
   {
   bool aHasMemRef = instructionA->getMemoryReference() != NULL;
   bool bHasMemRef = instructionB->getMemoryReference() != NULL;

   if (aHasMemRef && bHasMemRef)
      {
      if (readsFromMem(instructionA) && readsFromMem(instructionB))
         {
         traceMsg(cg->comp(), "\nNo memory  dependency between instruction A and B because both instructions are READs\n");
         }

      traceMsg(cg->comp(), "\nPossible memory dependency between instruction A and B\n");

      return true;
      }
//   bool aHasMemRef = writesToMem(instructionA) || readsFromMem(instructionB);
//   bool bHasMemRef = writesToMem(instructionB) || readsFromMem(instructionB);

   // We can't determine if the memory regions overlap
   // so conservatively if both instructions touch memory, assume there is a dependency
   traceMsg(cg->comp(), "\nNo memory  dependency between instruction A and B\n");
   return false;
   }

bool
X86InstructionScheduler::hasRegisterDependency(TR::Instruction *instructionA, TR::Instruction *instructionB,
                                               TR::CodeGenerator *cg)
   {
   TR::RealRegister::RegNum sourceRegsInstructionA[MAX_SOURCE_REGISTERS];
   TR::RealRegister::RegNum sourceRegsInstructionB[MAX_SOURCE_REGISTERS];

   TR::RealRegister::RegNum destRegA = getDestRegister(instructionA);
   TR::RealRegister::RegNum destRegB = getDestRegister(instructionB);

   int32_t numSrcRegsA = getSourceRegisters(instructionA, sourceRegsInstructionA);
   int32_t numSrcRegsB = getSourceRegisters(instructionB, sourceRegsInstructionB);

   // Check Read-After-Write (RAW) Dependency (True Dependency)

   traceMsg(cg->comp(), "\nChecking register dependencies between these instructions:\n");
   if (cg->comp()->getOption(TR_TraceCG))
      cg->getDebug()->print(cg->comp()->getOutFile(), instructionA);
   if (cg->comp()->getOption(TR_TraceCG))
      cg->getDebug()->print(cg->comp()->getOutFile(), instructionB);
   traceMsg(cg->comp(), "\n\n");

   if (readsFromRegister(instructionB, destRegA))
      {
      traceMsg(cg->comp(), "Instruction B reads from target register of instruction A\n");
      return true;
      }

//   auto aMR = instructionA->getMemoryReference();
//   auto bMR = instructionB->getMemoryReference();
//
//   if (aMR && aMR->getBaseRegister() &&
//       aMR->getBaseRegister()->getRealRegister()->getRegisterNumber() == OMR::RealRegister::vfp)
//      return true;
//
//   if (bMR && bMR->getBaseRegister() &&
//       bMR->getBaseRegister()->getRealRegister()->getRegisterNumber() == OMR::RealRegister::vfp)
//      return true;

//   if (destRegA == OMR::RealRegister::vfp || destRegA == OMR::RealRegister::esp || destRegA == OMR::RealRegister::ebp)
//      return true;
//
//   if (destRegB == OMR::RealRegister::vfp || destRegB == OMR::RealRegister::esp || destRegB == OMR::RealRegister::ebp)
//      return true;
   //

//   switch (instructionB->getOpCodeValue())
//      {
//      case TR::InstOpCode::CMPXCHG1MemReg:
//      case TR::InstOpCode::CMPXCHG2MemReg:
//      case TR::InstOpCode::CMPXCHG4MemReg:
//      case TR::InstOpCode::CMPXCHG8MemReg:
//      case TR::InstOpCode::CMPXCHG8BMem:
//      case TR::InstOpCode::CMPXCHG16BMem:
//      case TR::InstOpCode::LCMPXCHG1MemReg:
//      case TR::InstOpCode::LCMPXCHG2MemReg:
//      case TR::InstOpCode::LCMPXCHG4MemReg:
//      case TR::InstOpCode::LCMPXCHG8MemReg:
//      case TR::InstOpCode::LCMPXCHG8BMem:
//      case TR::InstOpCode::LCMPXCHG16BMem:
//      case TR::InstOpCode::XALCMPXCHG8MemReg:
//      case TR::InstOpCode::XACMPXCHG8MemReg:
//      case TR::InstOpCode::XALCMPXCHG4MemReg:
//      case TR::InstOpCode::XACMPXCHG4MemReg:
//      case TR::InstOpCode::XCHG2AccReg:
//      case TR::InstOpCode::XCHG4AccReg:
//      case TR::InstOpCode::XCHG8AccReg:
//      case TR::InstOpCode::XCHG1RegReg:
//      case TR::InstOpCode::XCHG2RegReg:
//      case TR::InstOpCode::XCHG4RegReg:
//      case TR::InstOpCode::XCHG8RegReg:
//      case TR::InstOpCode::XCHG1RegMem:
//      case TR::InstOpCode::XCHG2RegMem:
//      case TR::InstOpCode::XCHG4RegMem:
//      case TR::InstOpCode::XCHG8RegMem:
//      case TR::InstOpCode::XCHG1MemReg:
//      case TR::InstOpCode::XCHG2MemReg:
//      case TR::InstOpCode::XCHG4MemReg:
//      case TR::InstOpCode::XCHG8MemReg:
//         return true;
//      default:
//         break;
//      }
//   if (instructionB->getOpCode().modifiesSource() || instructionA->getOpCode().modifiesSource() ||
//       instructionA->getOpCode().usesTarget() || instructionB->getOpCode().usesTarget())
//      {
//      return true;
//      }

   if (instructionB->getOpCode().modifiesSource() || instructionB->getOpCode().usesTarget())
      {
      for (int32_t i = 0; i < numSrcRegsA; i++)
         {
         TR::RealRegister::RegNum srcRegA = sourceRegsInstructionA[i];

         if (writesToRegister(instructionB, srcRegA) || readsFromRegister(instructionB, srcRegA))
            {
            traceMsg(cg->comp(), "Instruction B modifies its src reg and instruction B uses that reg\n");
            return true;
            }
         }
      }

   if (instructionA->getOpCode().isPushOp() || instructionA->getOpCode().isPopOp())
      {
      if (readsFromRegister(instructionB, TR::RealRegister::esp))
         {
         traceMsg(cg->comp(), "Instruction A modifies the stack ptr and instruction B reads the stack pointer\n");
         return true;
         }
      else if (writesToRegister(instructionB, TR::RealRegister::esp))
         {
         traceMsg(cg->comp(), "Instruction A modifies the stack ptr and instruction B writes to stack pointer\n");
         return true;
         }
      }
//   if (writesToRegister(instructionB, TR::RealRegister::esp)
//       && (readsFromRegister(instructionA, TR::RealRegister::esp) ||
//           writesToRegister(instructionA, TR::RealRegister::esp)))
//      {
//      traceMsg(cg->comp(), "Instruction A reads or writes the stack ptr and instruction B writes to stack pointer\n");
//      return true;
//      }

   // Check Write-After-Write (WAW) Dependency (Output Dependency)
   if (writesToRegister(instructionB, destRegA))
      {
      traceMsg(cg->comp(), "Instruction B writes to target register of instruction A\n");
      return true;
      }

   // Write-After-Read (WAR) Dependency (Anti-Dependency)
   for (int32_t i = 0; i < numSrcRegsA; i++)
      {
      TR::RealRegister::RegNum srcRegA = sourceRegsInstructionA[i];

      if (writesToRegister(instructionB, srcRegA))
         {
         traceMsg(cg->comp(), "Instruction B writes to srcReg[%d] of instruction A\n", i);
         return true;
         }
      }

   bool aSetsFlags = instructionA->getOpCode().setsCCForCompare() || instructionA->getOpCode().setsCCForTest() ||
                     instructionA->getOpCode().modifiesSomeArithmeticFlags();
   bool aReadsFlags = instructionA->getOpCode().testsSomeFlag() || instructionA->getOpCode().isBranchOp();

   bool bSetsFlags = instructionB->getOpCode().setsCCForCompare() || instructionB->getOpCode().setsCCForTest() ||
                     instructionB->getOpCode().modifiesSomeArithmeticFlags();
   bool bReadsFlags = instructionB->getOpCode().testsSomeFlag() || instructionB->getOpCode().isBranchOp();


   traceMsg(cg->comp(), "Instruction A sets flags=%s\n", aSetsFlags ? "true" : "false");
   traceMsg(cg->comp(), "Instruction A reads flags=%s\n", aReadsFlags ? "true" : "false");
   traceMsg(cg->comp(), "Instruction B sets flags=%s\n", bSetsFlags ? "true" : "false");
   traceMsg(cg->comp(), "Instruction B reads flags=%s\n", bReadsFlags ? "true" : "false");

   // For example, jump depends on instructions setting flags such as test
   if (aSetsFlags || aReadsFlags || bSetsFlags || bReadsFlags)
//   if ((aSetsFlags && bReadsFlags) || (aSetsFlags && bSetsFlags))
      {
      traceMsg(cg->comp(), "Instruction B reads flags modified by instruction A\n");
      return true;
      }

   traceMsg(cg->comp(), "No dependency between instruction A and instruction B\n");

   return false;
   }

bool
X86InstructionScheduler::hasDependency(TR::Instruction *instructionA, TR::Instruction *instructionB,
                                       TR::CodeGenerator *cg)
   {
   return hasRegisterDependency(instructionA, instructionB, cg) || hasMemoryConflict(instructionA, instructionB, cg);
   }

std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>>
X86InstructionScheduler::buildDependencyGraph(const std::vector<TR::Instruction *> &instrList, TR::CodeGenerator *cg)
   {
   std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>> dependencyGraph;

   for (size_t i = 0; i < instrList.size(); ++i)
      {
      dependencyGraph[instrList[i]] = std::set<TR::Instruction *>();
      }

   for (size_t i = 0; i < instrList.size(); ++i)
      {
      TR::Instruction *instrA = instrList[i];

      for (size_t j = i + 1; j < instrList.size(); ++j)
         {
         TR::Instruction *instrB = instrList[j];

         if (hasDependency(instrB, instrA, cg))
            {
            // A has dependant instruction B
            dependencyGraph[instrA].insert(instrB);
            }
         }
      }

   return dependencyGraph;
   }

std::vector<TR::Instruction *>
X86InstructionScheduler::getNextBlock(TR::Instruction *currentInstruction, TR::CodeGenerator *cg)
   {
   if (isControlFlowInstruction(currentInstruction))
      {
      return {currentInstruction};
      }

   std::vector<TR::Instruction *> block = std::vector<TR::Instruction *>();

   while (currentInstruction)
      {
      block.push_back(currentInstruction);

      currentInstruction = currentInstruction->getNext();
      if (isControlFlowInstruction(currentInstruction))
         {
         break;
         }
      }

   return block;
   }

static const char *opCodeToMnemonicMap[] =
   {
#define INSTRUCTION(name, mnemonic, binary, property0, property1, features) #mnemonic

#include "codegen/X86Ops.ins"

#undef INSTRUCTION
   };

void
X86InstructionScheduler::printBlock(std::vector<TR::Instruction *> block, TR::CodeGenerator *cg)
   {
   traceMsg(cg->comp(), "----------------------------B L O C K----------------------------\n");

   for (const auto &currentInstruction: block)
      {
      TR::RealRegister::RegNum sourceRegsInstruction[MAX_SOURCE_REGISTERS];
      int32_t numSrcRegs = getSourceRegisters(currentInstruction, sourceRegsInstruction);
      bool hasDest = getDestRegister(currentInstruction) != TR::RealRegister::NoReg;
      bool srcMemRef = getSourceMemRef(currentInstruction) != NULL;
      bool destMemRef = getDestMemRef(currentInstruction) != NULL;
      bool controlFlow = isControlFlowInstruction(currentInstruction);

      int32_t nodeIdx = currentInstruction->getNode()->getGlobalIndex();

      if (cg->comp()->getOutFile())
         {
         cg->getDebug()->print(cg->comp()->getOutFile(), currentInstruction);
         }
//      traceMsg(cg->comp(),
//               "\nInstruction Scheduler: ptr=%p, node=n%dn, mnemonic=%s, numSrcRegs=%d, hasTargetReg=%s hasSrcMemRef=%s, hasDestMemRef=%s, isControlFlow=%s\n\n\n",
//               currentInstruction,
//               nodeIdx,
//               opCodeToMnemonicMap[currentInstruction->getOpCodeValue()],
//               numSrcRegs,
//               hasDest ? "true" : "false",
//               srcMemRef ? "true" : "false",
//               destMemRef ? "true" : "false",
//               controlFlow ? "true" : "false"
//      );
      }

   traceMsg(cg->comp(), "\n-----------------------------------------------------------------\n");
   }

void
X86InstructionScheduler::printDependencyGraph(std::vector<TR::Instruction *> block,
                                              std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>> graph,
                                              TR::CodeGenerator *cg)
   {
   traceMsg(cg->comp(), "---------------------------D E P E N D E N C Y - G R A P H---------------------------\n\n");

   for (const auto &instruction: block)
      {
      std::set<TR::Instruction *> dependencies = graph[instruction];

      if (cg->comp()->getOutFile())
         cg->getDebug()->print(cg->comp()->getOutFile(), instruction);
      traceMsg(cg->comp(), "\n\n        {");

      for (const auto &item: dependencies)
         {
         if (cg->comp()->getOutFile())
            cg->getDebug()->print(cg->comp()->getOutFile(), item);
         }

      traceMsg(cg->comp(), "\n        }\n");
      }

   traceMsg(cg->comp(), "-----------------------------------------------------------------------------------\n\n\n\n");
   }

std::vector<TR::Instruction *>
X86InstructionScheduler::topologicalSort(
   const std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>> dependencyGraph,
   const std::vector<TR::Instruction *> instructions)
   {
   // Step 1: Count incoming edges for each instruction
   std::vector<TR::Instruction *> sortedInstructions;
   std::unordered_map<TR::Instruction *, int32_t> inDegree;

   for (const auto &instr: instructions)
      {
      inDegree[instr] = 0; // Initialize in-degree to 0
      }

   for (const auto &item: dependencyGraph)
      {
      TR::Instruction *node = item.first;
      std::set<TR::Instruction *> dependents = item.second;

      for (const auto &dependent: dependents)
         {
         if (inDegree.count(dependent)) // Ensure dependent is in instructions
            {
            inDegree[dependent]++;
            }
         }
      }

   // Step 2: Create a queue for nodes with no incoming edges
   auto compare = [&](TR::Instruction *a, TR::Instruction *b)
      {
      return getInstructionLatency(a) < getInstructionLatency(b);
      };

   std::priority_queue<TR::Instruction *, std::vector<TR::Instruction *>, decltype(compare)> readyQueue(compare);

   for (const auto &pair: inDegree)
      {
      TR::Instruction *instr = pair.first;
      int32_t degree = pair.second;

      if (degree == 0)
         {
         readyQueue.push(instr);
         }
      }

   while (!readyQueue.empty())
      {
      TR::Instruction *current = readyQueue.top();
      readyQueue.pop();
      sortedInstructions.push_back(current);

      // Reduce the in-degree of dependent nodes
      if (dependencyGraph.count(current)) // Handle missing entries
         {
         for (const auto &dependent: dependencyGraph.at(current))
            {
            inDegree[dependent]--;
            if (inDegree[dependent] == 0)
               {
               readyQueue.push(dependent);
               }
            }
         }
      }

   TR_ASSERT_FATAL(sortedInstructions.size() == instructions.size(), "Cycle detected in dependency graph!");

   return sortedInstructions;
   }

void X86InstructionScheduler::scheduleBlock(std::vector<TR::Instruction *> block, TR::CodeGenerator *cg)
   {
   if (block.size() > 2)
      {
      std::unordered_map<TR::Instruction *, std::set<TR::Instruction *>> depGraph = buildDependencyGraph(block, cg);

      printDependencyGraph(block, depGraph, cg);

      traceMsg(cg->comp(), "-------------- BLOCK BEFORE INSTRUCTION SCHEDULING --------------\n");
      printBlock(block, cg);

      std::vector<TR::Instruction *> sortedBlock = topologicalSort(depGraph, block);

      traceMsg(cg->comp(), "--------------- BLOCK AFTER INSTRUCTION SCHEDULING ---------------\n");
      printBlock(sortedBlock, cg);

      traceMsg(cg->comp(), "\n");

      bool transformed = false;

      for (int i = 0; i < block.size(); i++)
         {
         if (block[i] != sortedBlock[i])
            {
            transformed = true;
            break;
            }
         }

      if (!transformed)
         {
         traceMsg(cg->comp(), "No instruction scheduling transformations were made.");
//         if (cg->comp()->getOption(TR_TraceCG))
//         printf("No instruction scheduling transformations were made.\n");
         return;
         }
      else
         {
         static int ensureCapacityInternal = 0;
//         if (strncmp(cg->comp()->getCurrentMethod()->nameChars(), "getPermissionCollection", 7) == 0)
         ensureCapacityInternal++;

//         if (ensureCapacityInternal >= 95 && ensureCapacityInternal <= 107) {
//            traceMsg(cg->comp(), "Scheduling transformations was skipped. %d\n", ensureCapacityInternal);
//            printf("Scheduling transformations was skipped. %d\n", ensureCapacityInternal);
//            return;
//            }

         traceMsg(cg->comp(), "Scheduling transformations has been made. %d\n", ensureCapacityInternal);

         if (block.size() > 10)
            {
            printf("\nlarge change made : %zu\n", block.size());
            }
//         if (cg->comp()->getOption(TR_TraceCG))
//         printf("Scheduling transformations has been made: %d\n", ensureCapacityInternal);
         }

      TR::Instruction *proceedingInstruction = block.front()->getPrev();
      TR::Instruction *nextBlockInstruction = block.back()->getNext();

      try
         {
         for (int i = 0; i < block.size() - 1; i++)
            {
            TR::Instruction *currentInstruction = sortedBlock[i];
            TR::Instruction *nextInstruction = sortedBlock[i + 1];

            currentInstruction->setNext(nextInstruction);
            nextInstruction->setPrev(currentInstruction);
            }

         proceedingInstruction->setNext(sortedBlock[0]);
         sortedBlock[0]->setPrev(proceedingInstruction);

         sortedBlock.back()->setNext(nextBlockInstruction);

         if (nextBlockInstruction)
            nextBlockInstruction->setPrev(sortedBlock.back());
         }
      catch (std::exception &e)
         {
//         printf("\nException caught : %s\n", e.what());
         }


      traceMsg(cg->comp(), "\n\n");
      }
   }

void X86InstructionScheduler::perform(TR::CodeGenerator *cg)
   {
   static bool enable = feGetEnv("TR_EnableInstructionScheduling") != NULL;
   if (!enable)
      {
      return;
      }

//   prctl(PR_SET_)
//   ARCH_SET_CPUID


//   if (strncmp(cg->comp()->getCurrentMethod()->nameChars(), "transfer", 7) == 0)
//      {
//      printf("Skipping: %s\n", cg->comp()->getCurrentMethod()->nameChars());
//      return;
//      }
//   printf("Performing Instruction Scheduling: %s\n", cg->comp()->getCurrentMethod()->nameChars());

   if (cg->comp()->getOption(TR_TraceCG))
      {
      traceMsg(cg->comp(), "Performing Instruction Scheduling\n");
      }

   TR::Instruction *firstInstruction = cg->getFirstInstruction();
   TR::Instruction *currentInstruction = firstInstruction;

   while (currentInstruction != NULL)
      {
      std::vector<TR::Instruction *> block = getNextBlock(currentInstruction, cg);
      scheduleBlock(block, cg);

      currentInstruction = block.back()->getNext();
      }
   }

}
}

