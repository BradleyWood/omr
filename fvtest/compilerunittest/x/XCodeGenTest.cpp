/*******************************************************************************
 * Copyright IBM Corp. and others 2024
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

#include <gtest/gtest.h>
#include "../CodeGenTest.hpp"
#include "codegen/OMRX86Instruction.hpp"
#include <chrono>

#define SETUP_CUSTOM_REGISTER(type, cg, deps, varName)                  \
        varName = cg->allocateRegister(type);                           \
        deps->addPostCondition(varName, TR::RealRegister::NoReg, cg);

#define SETUP_CUSTOM_REGISTERS_4(type, cg, deps, varA) SETUP_CUSTOM_REGISTER(type, cg, deps, varA)
#define SETUP_CUSTOM_REGISTERS_5(type, cg, deps, varA, varB) SETUP_CUSTOM_REGISTER(type, cg, deps, varA)       \
                                                             SETUP_CUSTOM_REGISTER(type, cg, deps, varB)
#define SETUP_CUSTOM_REGISTERS_6(type, cg, deps, varA, varB, varC) SETUP_CUSTOM_REGISTERS_5(type, cg, deps, varA, varB) \
                                                             SETUP_CUSTOM_REGISTER(type, cg, deps, varC)
#define SETUP_CUSTOM_REGISTERS_7(type, cg, deps, varA, varB, varC, varD) SETUP_CUSTOM_REGISTERS_6(type, cg, deps, varA, varB, varC) \
                                                             SETUP_CUSTOM_REGISTER(type, cg, deps, varD)
#define SETUP_CUSTOM_REGISTERS_8(type, cg, deps, varA, varB, varC, varD, varE) SETUP_CUSTOM_REGISTERS_7(type, cg, deps, varA, varB, varC, varD) \
                                                             SETUP_CUSTOM_REGISTER(type, cg, deps, varE)

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME,...) NAME
#define SETUP_CUSTOM_REGISTERS(...) GET_MACRO(__VA_ARGS__, SETUP_CUSTOM_REGISTERS_8, SETUP_CUSTOM_REGISTERS_7, SETUP_CUSTOM_REGISTERS_6, SETUP_CUSTOM_REGISTERS_5, SETUP_CUSTOM_REGISTERS_4)(__VA_ARGS__)


#define STOP_USING_REGISTERS(cg, ...) \
    {                                       \
    TR::Register* registers[] = { __VA_ARGS__ };                             \
    for (int i = 0; i < sizeof(registers) / sizeof(TR::Register *); ++i) {   \
        cg->stopUsingRegister(registers[i]);                                 \
    }                                       \
    }
#define SETUP_GPR_REGISTERS(cg, deps, ...) SETUP_CUSTOM_REGISTERS(TR_GPR, cg, deps, __VA_ARGS__)
#define SETUP_VRF_REGISTERS(cg, deps, ...) SETUP_CUSTOM_REGISTERS(TR_VRF, cg, deps, __VA_ARGS__)
#define SETUP_FPR_REGISTERS(cg, deps, ...) SETUP_CUSTOM_REGISTERS(TR_FPR, cg, deps, __VA_ARGS__)

class LoopGenTest : public TRTest::IReturnCodeGenTest{};

TEST_F(LoopGenTest, test) {
#define ITERATION_COUNT 105
    auto addr = compile([](TR::Node* node, TR::CodeGenerator* cg) -> TR::Register* {
        TR::Register *resultReg = cg->allocateRegister();

        generateRegRegInstruction(TR::InstOpCode::XOR4RegReg, node, resultReg, resultReg, cg);
        generateLoop(0, ITERATION_COUNT, node, cg, [=](int32_t unrollIteration) {
            generateRegInstruction(TR::InstOpCode::INCReg(), node, resultReg, cg);
            generateRegInstruction(TR::InstOpCode::INCReg(), node, resultReg, cg);
        });

        node->setRegister(resultReg);

        return resultReg;
    });

    auto *function = reinterpret_cast<int64_t (*)()>(addr);

    int64_t result = function();
    printf("Result: %ld\n", result);
    ASSERT_EQ(ITERATION_COUNT * 2, result);
}

TEST_F(LoopGenTest, test2) {
#define ITERATION_COUNT 8192
    static int32_t *src = static_cast<int32_t *>(calloc(sizeof(int32_t), ITERATION_COUNT));
    static int32_t *dest = static_cast<int32_t *>(calloc(sizeof(int32_t), ITERATION_COUNT));

    for (int32_t i = 0; i < ITERATION_COUNT; i++) {
        src[i] = i;
    }

    auto start = std::chrono::high_resolution_clock::now();

    auto addr = compile([](TR::Node* node, TR::CodeGenerator* cg) -> TR::Register* {
        TR::RegisterDependencyConditions *deps = generateRegisterDependencyConditions(0, 5, cg);
        TR::LabelSymbol *label = generateLabelSymbol(cg);
        TR::Register *indexReg, *loopBoundReg, *srcBufReg, *destBufReg, *tmpReg;
        SETUP_GPR_REGISTERS(cg, deps, indexReg, loopBoundReg, srcBufReg, destBufReg);
        SETUP_VRF_REGISTERS(cg, deps, tmpReg);

        intptr_t srcAddr = (int64_t )src;
        intptr_t destAddr = (int64_t )dest;
        TR::TreeEvaluator::loadConstant(node, 0, TR_RematerializableInt, cg, indexReg);
        TR::TreeEvaluator::loadConstant(node, ITERATION_COUNT, TR_RematerializableInt, cg, loopBoundReg);
        TR::TreeEvaluator::loadConstant(node, srcAddr, TR_RematerializableAddress, cg, srcBufReg);
        TR::TreeEvaluator::loadConstant(node, destAddr, TR_RematerializableAddress, cg, destBufReg);

        generateLoop(4, 4, indexReg, loopBoundReg, node, cg, NULL, [=](int32_t unrollIteration) {
            TR::MemoryReference *srcMemRef = generateX86MemoryReference(srcBufReg, indexReg, 2, unrollIteration * sizeof(int32_t) * 4, cg);
            TR::MemoryReference *destMemRef = generateX86MemoryReference(destBufReg, indexReg, 2, unrollIteration * sizeof(int32_t) * 4, cg);

            generateRegMemInstruction(TR::InstOpCode::MOVDQURegMem, node, tmpReg, srcMemRef, cg);
            generateMemRegInstruction(TR::InstOpCode::MOVDQUMemReg, node, destMemRef, tmpReg, cg);
        }, [=](int32_t unrollIteration) {
            TR::MemoryReference *srcMemRef = generateX86MemoryReference(srcBufReg, indexReg, 2, unrollIteration * sizeof(int32_t), cg);
            TR::MemoryReference *destMemRef = generateX86MemoryReference(destBufReg, indexReg, 2, unrollIteration * sizeof(int32_t), cg);

            generateRegMemInstruction(TR::InstOpCode::L4RegMem, node, tmpReg, srcMemRef, cg);
            generateMemRegInstruction(TR::InstOpCode::S4MemReg, node, destMemRef, tmpReg, cg);
        });

        generateLabelInstruction(TR::InstOpCode::label, node, label, deps, cg);
        STOP_USING_REGISTERS(cg, loopBoundReg, srcBufReg, destBufReg, tmpReg);
        node->setRegister(indexReg);

        return indexReg;
    });

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    printf("Compile time: %ld\n", duration.count());

    auto *function = reinterpret_cast<int64_t (*)()>(addr);

    int64_t result = function();
    printf("Result: %ld\n", result);

    for (int32_t i = 0; i < ITERATION_COUNT; i++) {
        printf("Result: %d\n", dest[i]);
        ASSERT_EQ(src[i], dest[i]);
    }
}
