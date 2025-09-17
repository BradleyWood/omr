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

#include "APXTest.h"

namespace OMR { namespace X86 {

#define ENCODINGS(...) __VA_ARGS__

#define ENCODING_6(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV)                              \
    {                                                                                                \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV), \
        .opcode = (opcodeV), .rm = (rmV)                                                             \
    }

#define ENCODING_7(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V)                   \
    {                                                                                                \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV), \
        .opcode = (opcodeV), .rm = (rmV), .operand1 = (operand1V)                                    \
    }

#define ENCODING_8(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V, operand2V)        \
    {                                                                                                \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV), \
        .opcode = (opcodeV), .rm = (rmV), .operand1 = (operand1V), .operand2 = (operand2V)           \
    }

#define ENCODING_9(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V, operand2V, operand3V)            \
    {                                                                                                               \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV),                \
        .opcode = (opcodeV), .rm = (rmV), .operand1 = (operand1V), .operand2 = (operand2V), .operand3 = (operand3V) \
    }

#define ENCODING_10(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V, operand2V, operand3V, operand4V) \
    {                                                                                                                \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV),                 \
        .opcode = (opcodeV), .rm = (rmV), .operand1 = (operand1V), .operand2 = (operand2V), .operand3 = (operand3V), \
        .operand4 = (operand4V)                                                                                      \
    }

#define ENCODING_11(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V, operand2V, operand3V, operand4V, \
    flagsV)                                                                                                          \
    {                                                                                                                \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV),                 \
        .opcode = (opcodeV), .rm = (rmV), .operand1 = (operand1V), .operand2 = (operand2V), .operand3 = (operand3V), \
        .operand4 = (operand4V), .flagsAffected = static_cast<FlagsAffected>((flagsV))                               \
    }

#define ENCODING_12(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V, operand2V, operand3V, operand4V, \
    flagsV, propertiesV)                                                                                             \
    {                                                                                                                \
        .prefixType = (prefixTypeV), .prefix = (prefixV), .map = (mapV), .prefixFlags = (pfxFlagsV),                 \
        .opcode = (opcodeV), .rm = (rmV), .operand1 = (operand1V), .operand2 = (operand2V), .operand3 = (operand3V), \
        .operand4 = (operand4V), .flagsAffected = static_cast<FlagsAffected>((flagsV)), .properties = (propertiesV)  \
    }

// Helper macro to count arguments (up to 12 in this example)
#define _ARG_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, COUNT, ...) COUNT
#define _COUNT_ARGS(...) _ARG_COUNT(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _ENCODING_SELECT2(count) ENCODING_##count
#define _ENCODING_SELECT1(count) _ENCODING_SELECT2(count)
#define ENCODING(...) _ENCODING_SELECT1(_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

const OMR::X86::NewInstOpCode *OMR::X86::NewInstOpCode::_metadata[] = {
#define INST(id, str, features, encs) \
    new NewInstOpCode[] { encs, NewInstOpCode() }
#include "APXInsn.ins"
#undef INST
};

template<typename TBuffer>
typename TBuffer::cursor_t OMR::X86::NewInstOpCode::encode(typename TBuffer::cursor_t cursor, uint8_t rexbits) const
{
    TBuffer buffer(cursor);

    if (prefix & _F0) {
        buffer.append(0xF0);
    }

    if (prefix & _F2) {
        buffer.append(0xF2);
    }

    if (prefix & _F3) {
        buffer.append(0xF3);
    }

    switch (prefixType) {
        case PREFIX_LEGACY:
        case PREFIX_REX:
        case PREFIX_REX_APX: {

            if (prefix & _66) {
                buffer.append(0x66);
            }

            if (prefix & _67) {
                buffer.append(0x67);
            }
        } break;
        default:
            break;
    }

#if defined(TR_TARGET_64BIT)
    if (prefixType == PREFIX_REX_APX) {
        buffer.append(0xD5);
    }

    if (rexbits) {
        buffer.append((map == ESC_0F && prefixType == PREFIX_REX_APX ? 0x80 : 0x00) | rexbits);
    }
#endif

    if (prefixType > PREFIX_BAD && prefixType < PREFIX_REX_APX) {
        switch (map) {
            case ESC_0F:
                buffer.append(0x0F);
                break;
            case ESC_OF_38:
                buffer.append(0x0F);
                buffer.append(0x38);
            case ESC_OF_3A:
                buffer.append(0x0F);
                buffer.append(0x3A);
                break;
            default:
                break;
        }
    }

    buffer.append(opcode);

    if (operand1 != NA && operand1 < IMM8) {
        OMR::X86::NewInstOpCode::ModRM modrm = OMR::X86::NewInstOpCode::ModRM(rm);
        buffer.append(modrm);
    }

    return buffer;
}

void OMR::X86::NewInstOpCode::encode(uint8_t *ptr, uint8_t rexbits) const
{
    encode<OMR::X86::NewInstOpCode::Writer>(ptr, rexbits);
}

}} // namespace OMR::X86

