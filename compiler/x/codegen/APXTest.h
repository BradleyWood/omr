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

#ifndef OMR_APXTEST_INCL
#define OMR_APXTEST_INCL

#include <stdint.h>

namespace OMR { namespace X86 {

enum OperandInfo {
    NA,
    MODRM_REG_R,
    MODRM_REG_W,
    MODRM_REG_RW,
    MODRM_RM_R,
    MODRM_RM_W,
    MODRM_RM_RW,
    VEX_VVVV,
    EVEX_VVVV,
    VVVVV_W,
    IMM8,
    IMM16,
    IMM32,
    IMM64,
    IMMS,
};

enum PrefixType {
    PREFIX_BAD,
    PREFIX_LEGACY,
    PREFIX_REX,
    PREFIX_REX_APX,
    PREFIX_VEX,
    PREFIX_EVEX,
    PREFIX_EVEX_APX
};

enum FlagsAffected {
    OF,
    SF,
    ZF,
    AF,
    PF,
    CF
};

enum Map {
    Primary,
    MAP0,
    MAP1,
    MAP2,
    MAP3,
    MAP4,
    MAP5,
    MAP6,
    MAP7,
    ESC_0F = MAP1,
    ESC_OF_38 = MAP2,
    ESC_OF_3A = MAP3,
};

enum Prefix {
    _NP = 0x00000000,
    _66 = 0x00000001,
    _67 = 0x00000002,
    _F0 = 0x00000004,
    _F2 = 0x00000008,
    _F3 = 0x00000010,
};

enum PrefixFlags {
    None = 0x00000000,
    LZ = 0x00000000,
    LLZ = LZ,
    L1 = 0x00000001,
    L2 = 0x00000002,

    W0 = 0x00000000,
    W1 = 0x00000004,
    WIG = W0,

    SCALABLE = 0x00000008,
};

enum RM {
    _0 = 0,
    _1 = 1,
    _2 = 2,
    _3 = 3,
    _4 = 4,
    _5 = 5,
    _6 = 6,
    _7 = 7,
    _R = 8,
};


enum NewMnemonic {
#define INST(id, str, features, encs) id
#include "APXInsn.ins"
    ,NumOpCodes
#undef INST
};

struct NewInstOpCode {
    PrefixType prefixType  : 3;
    uint8_t prefix  : 3;
    uint8_t map : 3;
    uint8_t prefixFlags : 6;
    uint8_t opcode;
    RM rm : 4;

    OperandInfo operand1 : 4;
    OperandInfo operand2 : 4;
    OperandInfo operand3 : 4;
    OperandInfo operand4 : 4;

    FlagsAffected flagsAffected : 8;
    uint32_t properties;


    static const NewInstOpCode* _metadata[];

    template<class TBuffer>
    typename TBuffer::cursor_t encode(typename TBuffer::cursor_t cursor, uint8_t rexbits) const;

    void encode(uint8_t *ptr, uint8_t rexbits) const;

    template<typename TCursor> class BufferBase {
    public:
        typedef TCursor cursor_t;

        inline operator cursor_t() const { return cursor; }

    protected:
        inline BufferBase(cursor_t cursor)
            : cursor(cursor)
        {}

        cursor_t cursor;
    };

    // helper class to calculate length
    class Estimator : public BufferBase<uint8_t> {
    public:
        inline Estimator(cursor_t size)
            : BufferBase<cursor_t>(size)
        {}

        template<typename T> void inline append(T binaries) { cursor += sizeof(T); }
    };

    // helper class to write binaries
    class Writer : public BufferBase<uint8_t *> {
    public:
        inline Writer(cursor_t cursor)
            : BufferBase<cursor_t>(cursor)
        {}

        template<typename T> void inline append(T binaries)
        {
            *((T *)cursor) = binaries;
            cursor += sizeof(T);
        }
    };

    struct ModRM {
        uint8_t rm: 3;
        uint8_t reg: 3;
        uint8_t mod: 2;

        inline ModRM() {}

        inline ModRM(uint8_t opcode)
        {
            rm = 0;
            reg = opcode;
            mod = 0x3;
        }

        inline ModRM(const ModRM &other)
        {
            rm = other.rm;
            reg = other.reg;
            mod = other.mod;
        }

        inline operator uint8_t() const { return *((uint8_t *)this); }

        inline uint8_t Reg(uint8_t R = 0) const { return (R << 3) | (0x7 & reg); }

        inline uint8_t RM(uint8_t B = 0) const
        {
            return (B << 3) | (0x7 & rm);
        }

        inline ModRM *setMod(uint8_t mod = 0x03) // 0b11
        {
            this->mod = mod;
            return this;
        }

        inline ModRM *setBase()
        {
            return setMod(0x00); // 0b00
        }

        inline ModRM *setBaseDisp8()
        {
            return setMod(0x01); // 0b01
        }

        inline ModRM *setBaseDisp32()
        {
            return setMod(0x02); // 0b10
        }

        inline ModRM *setIndexOnlyDisp32()
        {
            rm = 0x05; // 0b101
            return setMod(0x00); // 0b00
        }

        inline ModRM *setHasSIB()
        {
            rm = 0x04; // 0b100
            return this;
        }
    };
};



}}

/* Test Macros */

//#define ENCODINGS(...) __VA_ARGS__
//
//#define ENCODING(prefixTypeV, prefixV, mapV, pfxFlagsV, opcodeV, rmV, operand1V, operand2V, ...) \
//    { .prefixType = prefixTypeV, .prefix = prefixV, .map = mapV, .prefixFlags = pfxFlagsV, .opcode = opcodeV, .rm = rmV, .operand1 = operand1V, .operand2 = operand2V, __VA_ARGS__ }
//
//#define ENCODING(...) \
//    { __VA_ARGS__ }
//
//// For encoding table.
//#define INST(id, str, features, encs) \
//    static NewInstOpCode enc_##id[] = { encs };

// For Enum Declaration
//#define INST(id, str, features, encs) id


//void test()
//{
//    INST(ADD4, add, 0,
//        ENCODINGS(
//            ENCODING( /* 03 /r ADD r32, r/m32 */
//                PREFIX_LEGACY, _NP, Primary, None, 0x03, _R,
//                MODRM_REG_RW, MODRM_RM_R
//            ),
//            ENCODING( /* 81 /0 id ADD r/m32, imm32 */
//                PREFIX_LEGACY, _NP, Primary, None, 0x81, _0,
//                MODRM_RM_RW, IMM32
//            ),
//            ENCODING( /* EVEX.LLZ.66.MAP4.SCALABLE 03 /r ADD {NF} {ND=1} rv, rv, rv/mv */
//                PREFIX_EVEX_APX, _66, MAP4, LLZ | SCALABLE, 0x03, _R,
//                MODRM_REG_RW, MODRM_RM_R
//            ),
//            ENCODING( /* EVEX.LLZ.NP.MAP4.SCALABLE 81 /0 id ADD {NF} {ND=0} rv/mv, imm32 */
//                PREFIX_EVEX_APX, _NP, MAP4, LLZ | SCALABLE, 0x81, _0,
//                MODRM_RM_RW, IMM32, NA, NA, ZF
//            ),
//            ENCODING( /* REX-2 Best guess; add reg32, reg32 */
//                PREFIX_REX_APX, _66, MAP4, LLZ | SCALABLE, 0x03, _R,
//                MODRM_REG_RW, MODRM_RM_R
//            ),
//            ENCODING( /* REX-2 Best guess; add reg32, imm32 */
//                PREFIX_REX_APX, _NP, MAP4, LLZ | SCALABLE, 0x81, _0,
//                MODRM_RM_RW, IMM32,
//            )
//        )
//    );
//
//#define VEX_128_0F_W0 .prefixType=PREFIX_VEX,.prefix = _NP,.map=MAP0,.prefixFlags=LZ
//
//    NewInstOpCode testOpcode = ENCODING(
//        VEX_128_0F_W0,
//        .opcode = 0x00, .rm = _R,
//        .operand1 = MODRM_RM_RW, .operand2 = MODRM_REG_R
//    );
//
//}

#endif
