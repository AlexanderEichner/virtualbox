/* $Id$ */
/** @file
 * VBox disassembler - Internal header.
 */

/*
 * Copyright (C) 2026 Alexander Eichner <github@aeichner.de>.
 *
 * This file is part of VirtualBox base platform packages, as
 * available from https://www.virtualbox.org.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, in version 3 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef VBOX_INCLUDED_SRC_DisasmInternal_riscv_h
#define VBOX_INCLUDED_SRC_DisasmInternal_riscv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <VBox/types.h>
#include <VBox/err.h>
#include <VBox/dis.h>
#include <VBox/log.h>

#include <iprt/param.h>
#include "DisasmInternal.h"


/** @addtogroup grp_dis_int Internals.
 * @ingroup grp_dis
 * @{
 */

/** @name Index into g_apfnFullDisasm.
 * @{ */
typedef enum DISPARMPARSEIDX
{
    kDisParmParseNop = 0,
    kDisParmParseReg,
    kDisParmParseImm,
    kDisParmParseStore,
    kDisParmParseLoad,
    kDisParmParseBranch,
    kDisParmParseU,
    kDisParmParseJmp,
    kDisParmParseMax
} DISPARMPARSEIDX;
/** @}  */


/**
 * Opcode structure.
 */
typedef struct DISRISCVOPCODE
{
    /** The value of the fixed bits of the instruction. */
    uint32_t            fValue;
    /** The generic opcode structure. */
    DISOPCODE           Opc;
} DISRISCVOPCODE;
/** Pointer to a const opcode. */
typedef const DISRISCVOPCODE *PCDISRISCVOPCODE;


#define DIS_RISCV_OP(a_fValue, a_szOpcode, a_uOpcode) \
    { a_fValue, OP(a_szOpcode, 0, 0, 0, a_uOpcode, 0, 0, 0, DISOPTYPE_HARMLESS) }
#define DIS_RISCV_OP_F(a_fValue, a_szOpcode, a_uOpcode, a_fOpType) \
    { a_fValue, OP(a_szOpcode, 0, 0, 0, a_uOpcode, 0, 0, 0, a_fOpType) }
#define DIS_RISCV_OP_EX(a_fValue, a_szOpcode, a_uOpcode, a_fOpType) \
    { a_fValue, OP(a_szOpcode, 0, 0, 0, a_uOpcode, 0, 0, 0, a_fOpType) }


/**
 * Instruction class descriptor.
 */
typedef struct DISRISCVINSNCLASS
{
    /** Pointer to the array of opcodes. */
    PCDISRISCVOPCODE        paOpcodes;
    /** Number of opcodes in the opcode table. */
    uint32_t                cOpcodes;
    /** The mask of fixed instruction bits. */
    uint32_t                fFixedInsn;
    /** Opcode decoder function. */
    uint32_t                enmOpcDecode;
    /** The mask of the bits relevant for decoding. */
    uint32_t                fMask;
    /** Number of bits to shift to get an index. */
    uint32_t                cShift;
} DISRISCVINSNCLASS;
/** Pointer to a constant instruction class descriptor. */
typedef const DISRISCVINSNCLASS *PCDISRISCVINSNCLASS;


#define DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(a_Name) \
    static const DISRISCVOPCODE g_aRiscVInsn ## a_Name ## Opcodes[] = {
#define DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(a_Name, a_fFixedInsn, a_enmOpcDecode, a_fMask, a_cShift) \
    }; \
    static const DISRISCVINSNCLASS g_aRiscVInsn ## a_Name = \
    { \
        & g_aRiscVInsn ## a_Name ## Opcodes[0], \
        RT_ELEMENTS(g_aRiscVInsn ## a_Name ## Opcodes), \
        a_fFixedInsn, a_enmOpcDecode, a_fMask, a_cShift \
    }


/**
 * Decoder map when direct indexing is possible.
 */
typedef struct DISRISCVDECODEMAP
{
    /** The bitmask used to decide where to go next. */
    uint32_t                    fMask;
    /** Amount to shift to get at the index. */
    uint32_t                    cShift;
    /** Number of elements in the instruction class array. */
    uint32_t                    cClasses;
    /** Pointer to the array of pointers to the instruction class. */
    const PCDISRISCVINSNCLASS   *papInsnClass;
} DISRISCVDECODEMAP;
/** Pointer to a const decode map. */
typedef const struct DISRISCVDECODEMAP *PCDISRISCVDECODEMAP;

#define DIS_RISCV_DECODE_MAP_DEFINE_BEGIN(a_Name) \
    static const PCDISRISCVINSNCLASS g_aRiscVInsn ## a_Name ## Class[] = {

#define DIS_RISCV_DECODE_MAP_DEFINE_END_NON_STATIC(a_Name, a_fMask, a_cShift) \
    }; \
    DECL_HIDDEN_CONST(DISRISCVDECODEMAP) g_aRiscVInsn ## a_Name = \
    { \
        a_fMask, a_cShift, \
        RT_ELEMENTS(g_aRiscVInsn ## a_Name ## Class), \
        & g_aRiscVInsn ## a_Name ## Class[0] \
    }

#define DIS_RISCV_DECODE_MAP_INVALID_ENTRY NULL
#define DIS_RISCV_DECODE_MAP_ENTRY(a_Class) & g_aRiscVInsn ## a_Class


/** @name Decoder maps.
 * @{ */
extern DECL_HIDDEN_DATA(DISOPCODE) g_RiscVInvalidOpcode[1];

extern DECL_HIDDEN_DATA(DISRISCVDECODEMAP) g_aRiscVInsnDecodeL0;
/** @} */


/** @} */
#endif /* !VBOX_INCLUDED_SRC_DisasmInternal_riscv_h */

