/*******************************************************************************
 * Copyright IBM Corp. and others 2025
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

#ifndef OMR_CODEGENUTILS_INCL
#define OMR_CODEGENUTILS_INCL

#include "codegen/CodeGenerator.hpp"

namespace OMR
{

namespace X86
{

/**
 * @brief Generates loop control flow with start and end bounds
 *
 * This function generates assembly code for a loop that iterates from a specified
 * starting value to an ending value. The loop body is implemented by a user-provided function.
 *
 * @param begin The starting value of the loop index
 * @param end The ending value of the loop index
 * @param node The node associated with the loop
 * @param cg The code generator used to emit instructions
 * @param genBodyFunction A callback function that generates the body of the loop
 */
void generateLoop(int32_t begin,
                  int32_t end,
                  TR::Node *node,
                  TR::CodeGenerator *cg,
                  std::function<void(int32_t)> genBodyFunction);

/**
 * @brief Generates a loop with parameterized unrolling and optional residue element processing.
 *
 * This function generates an optimized loop with a specified unrolling factor and the ability to process
 * multiple elements per iteration. If the total number of iterations is not divisible by the number of
 * elements processed in each unrolled iteration, a secondary residue loop can be generated to handle
 * the leftover elements.
 *
 * Constraints:
 * - `unrollFactor` must be a power of 2.
 * - `elementsPerIteration` must be a power of 2.
 * - `residueGenBodyFunction` If specified, must only process one iteration of loop index.
 *
 * @param unrollFactor Number of iterations to unroll in the main loop.
 * @param elementsPerIteration Number of elements processed per loop body iteration.
 * @param indexReg The register used to store the loop index.
 * @param maxIndexReg The register containing the loop bound.
 * @param node IL node associated with the loop generation.
 * @param cg Pointer to the code generator responsible for emitting the loop code.
 * @param loopInitializerFunction Function to generate initialization code prior to the first iteration, if applicable.
 * @param genBodyFunction Function to generate the body of the loop for each unrolled iteration.
 * @param residueGenBodyFunction Function to generate the body for the residue loop, if applicable.
 */
void generateLoop(int32_t unrollFactor,
                  int32_t elementsPerIteration,
                  TR::Register *indexReg,
                  TR::Register *maxIndexReg,
                  TR::Node *node,
                  TR::CodeGenerator *cg,
                  std::function<void()> loopInitializerFunction,
                  std::function<void(int32_t)> genBodyFunction,
                  std::function<void(int32_t)> residueGenBodyFunction = NULL);

/**
 * @brief Generates a loop with parameterized unrolling and optional residue element processing.
 *
 * This function generates an optimized loop with a specified unrolling factor and the ability to process
 * multiple elements per iteration. If the total number of iterations is not divisible by the number of
 * elements processed in each unrolled iteration, a secondary residue loop can be generated to handle
 * the leftover elements.
 *
 * Constraints:
 * - `unrollFactor` must be a power of 2.
 * - `elementsPerIteration` must be a power of 2.
 * - `residueGenBodyFunction` If specified, must only process one iteration of loop index.
 *
 * @param unrollFactor Number of iterations to unroll in the main loop.
 * @param elementsPerIteration Number of elements processed per loop body iteration.
 * @param indexReg The register used to store the loop index.
 * @param maxIndexReg The register containing the loop bound.
 * @param node IL node associated with the loop generation.
 * @param cg Pointer to the code generator responsible for emitting the loop code.
 * @param genBodyFunction Function to generate the body of the loop for each unrolled iteration.
 * @param residueGenBodyFunction Function to generate the body for the residue loop, if applicable.
 */
inline void generateLoop(int32_t unrollFactor,
                         int32_t elementsPerIteration,
                         TR::Register *indexReg,
                         TR::Register *maxIndexReg,
                         TR::Node *node,
                         TR::CodeGenerator *cg,
                         std::function<void(int32_t)> genBodyFunction,
                         std::function<void(int32_t)> residueGenBodyFunction = NULL)
   {
   generateLoop(unrollFactor, elementsPerIteration, indexReg, maxIndexReg, node, cg, NULL, genBodyFunction, residueGenBodyFunction);
   }

/**
 * @brief Generates a loop with parameterized unrolling and residue processing.
 *
 * This function generates an unrolled loop where each unrolled iteration
 * processes only one element. A second loop is automatically generated to
 * process leftover (residue) elements, ensuring all elements are handled.
 *
 * Constraints:
 * - `unrollFactor` must be a power of 2.
 * - `genBodyFunction` must only process one iteration of loop index.
 *
 * @param unrollFactor Number of iterations to unroll in the main loop.
 * @param indexReg The register used to store the loop index.
 * @param maxIndexReg The register containing the loop bound.
 * @param node IL node associated with the loop generation.
 * @param cg Pointer to the code generator responsible for emitting the loop code.
 * @param genBodyFunction Function to generate the body of the loop for each unrolled iteration.
 */
inline void generateUnrolledLoopWithResidue(int32_t unrollFactor,
                                            TR::Register *indexReg,
                                            TR::Register *maxIndexReg,
                                            TR::Node *node,
                                            TR::CodeGenerator *cg,
                                            std::function<void(int32_t)> genBodyFunction)
   {
   generateLoop(unrollFactor, 1, indexReg, maxIndexReg, node, cg, NULL, genBodyFunction, genBodyFunction);
   }

/**
 * @brief Generates a simple loop without unrolling or residue processing.
 *
 * This function generates a loop where each iteration processes a single element.
 * It is suitable for scenarios where the loop body function processes exactly one
 * loop index per iteration. No additional residue handling is required.
 *
 * Constraints:
 * - `genBodyFunction` must only process one iteration of loop index.
 *
 * @param indexReg The register used to store the loop index.
 * @param loopBoundReg The register containing the loop bound.
 * @param node IL node associated with the loop generation.
 * @param cg Pointer to the code generator responsible for emitting the loop code.
 * @param genBodyFunction Function to generate the body of the loop for each iteration.
 */
inline void generateLoop(TR::Register *indexReg,
                         TR::Register *maxIndexReg,
                         TR::Node *node,
                         TR::CodeGenerator *cg,
                         std::function<void(int32_t)> genBodyFunction)
   {
   generateLoop(1, 1, indexReg, maxIndexReg, node, cg, NULL, genBodyFunction, NULL);
   }


}

}

#endif
