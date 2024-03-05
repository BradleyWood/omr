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

#include <stdlib.h>
#include <string.h>
#include "env/CPU.hpp"
#include "env/CompilerEnv.hpp"
#include "env/JitConfig.hpp"
#include "env/ProcessorInfo.hpp"
#include "infra/Flags.hpp"
#include "x/runtime/X86Runtime.hpp"
#include "codegen/CodeGenerator.hpp"

TR::CPU
OMR::X86::CPU::detect(OMRPortLibrary * const omrPortLib)
   {
   if (omrPortLib == NULL)
      return TR::CPU();

   // Only enable the features that compiler currently uses
   uint32_t const enabledFeatures [] = {OMR_FEATURE_X86_FPU, OMR_FEATURE_X86_CX8, OMR_FEATURE_X86_CMOV,
                                        OMR_FEATURE_X86_MMX, OMR_FEATURE_X86_SSE, OMR_FEATURE_X86_SSE2,
                                        OMR_FEATURE_X86_SSSE3, OMR_FEATURE_X86_SSE4_1, OMR_FEATURE_X86_SSE4_2,
                                        OMR_FEATURE_X86_POPCNT, OMR_FEATURE_X86_AESNI, OMR_FEATURE_X86_OSXSAVE,
                                        OMR_FEATURE_X86_AVX, OMR_FEATURE_X86_AVX2, OMR_FEATURE_X86_FMA, OMR_FEATURE_X86_HLE,
                                        OMR_FEATURE_X86_RTM, OMR_FEATURE_X86_AVX512F, OMR_FEATURE_X86_AVX512VL,
                                        OMR_FEATURE_X86_AVX512BW, OMR_FEATURE_X86_AVX512DQ, OMR_FEATURE_X86_AVX512CD,
                                        OMR_FEATURE_X86_AVX512_VBMI2, OMR_FEATURE_X86_AVX512_VPOPCNTDQ,
                                        OMR_FEATURE_X86_AVX512_BITALG
                                        };

   OMRPORT_ACCESS_FROM_OMRPORT(omrPortLib);
   OMRProcessorDesc featureMasks;
   memset(featureMasks.features, 0, OMRPORT_SYSINFO_FEATURES_SIZE*sizeof(uint32_t));
   for (size_t i = 0; i < sizeof(enabledFeatures)/sizeof(uint32_t); i++)
      {
      omrsysinfo_processor_set_feature(&featureMasks, enabledFeatures[i], TRUE);
      }

   OMRProcessorDesc processorDescription;
   omrsysinfo_get_processor_description(&processorDescription);
   for (size_t i = 0; i < OMRPORT_SYSINFO_FEATURES_SIZE; i++)
      {
      processorDescription.features[i] &= featureMasks.features[i];
      }

   if (TRUE == omrsysinfo_processor_has_feature(&processorDescription, OMR_FEATURE_X86_OSXSAVE))
      {
      static const bool disableAVX = feGetEnv("TR_DisableAVX") != NULL;
      if (((6 & _xgetbv(0)) != 6) || disableAVX) // '6' = mask for XCR0[2:1]='11b' (XMM state and YMM state are enabled)
         {
         // Unset OSXSAVE if not enabled via CR0
         omrsysinfo_processor_set_feature(&processorDescription, OMR_FEATURE_X86_OSXSAVE, FALSE);
         }
      }

   return TR::CPU(processorDescription);
   }

TR_X86CPUIDBuffer *
OMR::X86::CPU::queryX86TargetCPUID()
   {
   static TR_X86CPUIDBuffer *buf = NULL;

   if (!buf)
      {
      buf = reinterpret_cast<TR_X86CPUIDBuffer *>(malloc(sizeof(TR_X86CPUIDBuffer)));
      if (!buf)
         return NULL;
      jitGetCPUID(buf);
      }

   return buf;
   }

const char *
OMR::X86::CPU::getX86ProcessorVendorId()
   {
   static char buf[13];

   // Terminate the vendor ID with NULL before returning.
   //
   strncpy(buf, self()->queryX86TargetCPUID()->_vendorId, 12);
   buf[12] = '\0';

   return buf;
   }

uint32_t
OMR::X86::CPU::getX86ProcessorSignature()
   {
   return self()->queryX86TargetCPUID()->_processorSignature;
   }

uint32_t
OMR::X86::CPU::getX86ProcessorFeatureFlags()
   {
   if (TR::Compiler->omrPortLib == NULL)
      return self()->queryX86TargetCPUID()->_featureFlags;

   return self()->_processorDescription.features[0];
   }

uint32_t
OMR::X86::CPU::getX86ProcessorFeatureFlags2()
   {
   if (TR::Compiler->omrPortLib == NULL)
      return self()->queryX86TargetCPUID()->_featureFlags2;

   return self()->_processorDescription.features[1];
   }

uint32_t
OMR::X86::CPU::getX86ProcessorFeatureFlags8()
   {
   if (TR::Compiler->omrPortLib == NULL)
      return self()->queryX86TargetCPUID()->_featureFlags8;

   return self()->_processorDescription.features[3];
   }

uint32_t
OMR::X86::CPU::getX86ProcessorFeatureFlags10()
   {
   if (TR::Compiler->omrPortLib == NULL)
      return self()->queryX86TargetCPUID()->_featureFlags10;

   return self()->_processorDescription.features[4];
   }

bool
OMR::X86::CPU::getSupportsHardwareSQRT()
   {
   return true;
   }

bool
OMR::X86::CPU::supportsTransactionalMemoryInstructions()
   {
   return self()->supportsFeature(OMR_FEATURE_X86_RTM);
   }

bool
OMR::X86::CPU::isGenuineIntel()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU model tests");
   return self()->isAtLeast(OMR_PROCESSOR_X86_INTEL_FIRST) && self()->isAtMost(OMR_PROCESSOR_X86_INTEL_LAST);
   }

bool
OMR::X86::CPU::isAuthenticAMD()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU model tests");
   return self()->isAtLeast(OMR_PROCESSOR_X86_AMD_FIRST) && self()->isAtMost(OMR_PROCESSOR_X86_AMD_LAST);
   }

bool
OMR::X86::CPU::requiresLFence()
   {
   return false;  /* Dummy for now, we may need TR::InstOpCode::LFENCE in future processors*/
   }

bool
OMR::X86::CPU::supportsMFence()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU feature tests");
   return self()->supportsFeature(OMR_FEATURE_X86_SSE2);
   }

bool
OMR::X86::CPU::supportsLFence()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU feature tests");
   return self()->supportsFeature(OMR_FEATURE_X86_SSE2);
   }

bool
OMR::X86::CPU::supportsSFence()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU model tests");
   return self()->supportsFeature(OMR_FEATURE_X86_SSE2) || self()->supportsFeature(OMR_FEATURE_X86_MMX);
   }

bool
OMR::X86::CPU::prefersMultiByteNOP()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU model tests");
   return self()->isGenuineIntel() && !self()->is(OMR_PROCESSOR_X86_INTELPENTIUM);
   }

bool
OMR::X86::CPU::supportsAVX()
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU feature tests");
   return self()->supportsFeature(OMR_FEATURE_X86_AVX) && self()->supportsFeature(OMR_FEATURE_X86_OSXSAVE);
   }

bool
OMR::X86::CPU::is(OMRProcessorArchitecture p)
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU model tests");
   return _processorDescription.processor == p;
   }

bool
OMR::X86::CPU::supportsFeature(uint32_t feature)
   {
   TR_ASSERT_FATAL(TR::Compiler->omrPortLib != NULL, "OMR Compiler requires the port library to perform CPU feature tests");

   OMRPORT_ACCESS_FROM_OMRPORT(TR::Compiler->omrPortLib);
   return TRUE == omrsysinfo_processor_has_feature(&_processorDescription, feature);
   }

const char*
OMR::X86::CPU::getProcessorName()
   {
   const char* returnString = "";
   switch(_processorDescription.processor)
      {
      case OMR_PROCESSOR_X86_INTELPENTIUM:
         returnString = "X86 Intel Pentium";
         break;

      case OMR_PROCESSOR_X86_INTELP6:
         returnString = "X86 Intel P6";
         break;

      case OMR_PROCESSOR_X86_INTELPENTIUM4:
         returnString = "X86 Intel Netburst Microarchitecture";
         break;

      case OMR_PROCESSOR_X86_INTELCORE2:
         returnString = "X86 Intel Core2 Microarchitecture";
         break;

      case OMR_PROCESSOR_X86_INTELTULSA:
         returnString = "X86 Intel Tulsa";
         break;

      case OMR_PROCESSOR_X86_INTELNEHALEM:
         returnString = "X86 Intel Nehalem";
         break;

      case OMR_PROCESSOR_X86_INTELWESTMERE:
         returnString = "X86 Intel Westmere";
         break;

      case OMR_PROCESSOR_X86_INTELSANDYBRIDGE:
         returnString = "X86 Intel Sandy Bridge";
         break;

      case OMR_PROCESSOR_X86_INTELIVYBRIDGE:
         returnString = "X86 Intel Ivy Bridge";
         break;

      case OMR_PROCESSOR_X86_INTELHASWELL:
         returnString = "X86 Intel Haswell";
         break;

      case OMR_PROCESSOR_X86_INTELBROADWELL:
         returnString = "X86 Intel Broadwell";
         break;

      case OMR_PROCESSOR_X86_INTELSKYLAKE:
         returnString = "X86 Intel Skylake";
         break;

      case OMR_PROCESSOR_X86_AMDK5:
         returnString = "X86 AMDK5";
         break;

      case OMR_PROCESSOR_X86_AMDK6:
         returnString = "X86 AMDK6";
         break;

      case OMR_PROCESSOR_X86_AMDATHLONDURON:
         returnString = "X86 AMD Athlon-Duron";
         break;

      case OMR_PROCESSOR_X86_AMDOPTERON:
         returnString = "X86 AMD Opteron";
         break;

      case OMR_PROCESSOR_X86_AMDFAMILY15H:
         returnString = "X86 AMD Family 15h";
         break;

      default:
         returnString = "Unknown X86 Processor";
         break;
      }
   return returnString;
   }
