#include "OpCodeTest.hpp"
#include "default_compiler.hpp"

#include <stdio.h>
#include <inttypes.h>
#include <fstream>

class IntegerDecomposerTest : public TRTest::JitTest
   {
   };

int mca(char *arch)
   {
   // Example input to send to llvm-mca
   char command_buf[512];
   char *command = "/home/brad/PycharmProjects/AsmTools/.venv/bin/python /home/brad/PycharmProjects/AsmTools/main.py --disassemble --mcpu=%s";
   sprintf(command_buf, command, arch);
   // Open the process for reading its output
   FILE *fp = popen(command_buf, "r");

   // Check if the process was opened successfully
   if (fp == nullptr)
      {
      perror("popen failed");
      return -1;
      }

   char buffer[32];
   while (fgets(buffer, sizeof(buffer), fp) != nullptr);

   // Close the process
   fclose(fp);

   int32_t cycles = atoi(buffer);
   return cycles;
   }

int32_t eval(char *arch, int32_t iConst)
   {
   char buffer[512];
   char *inputTrees =
      "(method return=Int32 args=[Int32]"
      "  (block"
      "    (ireturn"
      "      (imul"
      "        (iload parm=\"a\")"
      "        (iconst %d) ) ) ) )";
   sprintf(buffer, inputTrees, iConst);
   auto trees = parseString(buffer);

   Tril::DefaultCompiler compiler(trees);

   compiler.compile();

   auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();

//    printf("EP: %p", entry_point);

   uint8_t *bytes = (uint8_t *) entry_point;
   int32_t stopIdx = 0;

   for (int i = 0; i < 100; i++)
      {
      if (bytes[i] == 0x41 && bytes[i + 1] == 0x5b && bytes[i + 2] == 0xc3)
         {
         break;
         }
      stopIdx++;
      }

   FILE *outfile = fopen("/home/brad/PycharmProjects/AsmTools/disassembly.bin", "w");

   for (int i = 5; i < stopIdx; i++)
      {
//        printf("%02X ", bytes[i]);
      fprintf(outfile, "%02X ", bytes[i]);
      }

   fclose(outfile);

   return mca(arch);
   }

TEST_F(IntegerDecomposerTest, ScheduleTest)
   {
   int32_t iConstA = 1000;
   int32_t iConstB = 2000;
   char buffer[512];
   sprintf(buffer, "(method return=Int32 args=[Int32]"
                   "  (block"
                   "    (istore temp=\"a\"" // First independent computation
                   "      (imul"
                   "        (iload parm=0)"
                   "        (iconst %d)))" // Multiply by constant 1
                   "    (istore temp=\"b\"" // Second independent computation
                   "      (iadd"
                   "        (iconst 100)"
                   "        (iload parm=0)))" // Add constant to input
                   "    (ireturn"
                   "      (iadd"
                   "        (iload temp=\"a\")" // Load first result
                   "        (iload temp=\"b\")))" // Load second result
                   "  ) ) )", iConstA);

   auto trees = parseString(buffer);

   Tril::DefaultCompiler compiler(trees);

   compiler.compile();
   auto entry_point = compiler.getEntryPoint<int32_t (*)(int32_t)>();

   }

TEST_F(IntegerDecomposerTest, DecompTestBasic)
   {
   int32_t iConst = 6;
   char *cpu = "alderlake";

   char *constOverride = feGetEnv("TR_DecompositionConst");
   char *cpuOverride = feGetEnv("TR_MCPU");

   if (constOverride != NULL)
      {
      iConst = atoi(constOverride);
      }

   if (cpuOverride != NULL)
      {
      cpu = cpuOverride;
      }

   int32_t cycles = eval("alderlake", iConst);
   printf("Evaluating imul by constant %d, numCycles=%d, cpu=%s\n", iConst, cycles, cpu);
   printf("Evaluating imul by constant %d, numCycles=%d, cpu=%s\n", iConst, cycles, cpu);
   }

TEST_F(IntegerDecomposerTest, DecompTest)
   {
   char *microArchitectures[] = {
      "znver2",
      "znver3",
      "core2",
      "sandybridge",
      "haswell",
      "broadwell",
      "skylake",
      "icelake-server",
      "tigerlake",
      "rocketlake",
      "alderlake",
      "sapphirerapids",
   };

//    for (const auto &cpu: microArchitectures) {
//        printf("--------------------------------------------\n");
//        char fileName[128];
//        sprintf(fileName, "IntegerDecomposition-%s.csv", cpu);
//        FILE *outfile = fopen(fileName, "w");
//
//        fprintf(outfile, "Integer Const,Expected,Actual\n");
//
//        for (int i = 0; i < 3000; i++) {
//            int32_t iConst = i;
//            int32_t cycles = eval(cpu, iConst);
//            printf("Evaluating imul by constant %d, numCycles=%d, cpu=%s\n", iConst, cycles, cpu);
//
//            if (cycles <= 110) {
////                printf("Performance is acceptable.\n");
//                fprintf(outfile, "%d,%d,%d\n", iConst, 110, cycles);
//            } else if (cycles >= 0) {
//                fprintf(outfile, "%d,%d,%d\n", iConst, 110, cycles);
////                printf("Performance is worse than using imul instruction.\n");
//            } else {
//                printf("Performance is unknown.\n");
//            }
//        }
//
//        printf("--------------------------------------------\n");
//        fclose(outfile);
//    }
   }
