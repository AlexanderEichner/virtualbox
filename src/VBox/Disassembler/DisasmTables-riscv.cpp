/* $Id$ */
/** @file
 * VBox disassembler - Tables for RISC-V.
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


/*********************************************************************************************************************************
*   Header Files                                                                                                                 *
*********************************************************************************************************************************/
#include <VBox/dis.h>
#include <VBox/disopcode-riscv.h>
#include "DisasmInternal-riscv.h"


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/

#ifndef DIS_CORE_ONLY
static char g_szInvalidOpcode[] = "Invalid Opcode";
#endif

#define INVALID_OPCODE  \
    DIS_RISCV_OP_F(0, g_szInvalidOpcode,    OP_RISCV_INVALID, DISOPTYPE_INVALID)


/* Invalid opcode */
DECL_HIDDEN_CONST(DISOPCODE) g_RiscVInvalidOpcode[1] =
{
    OP(g_szInvalidOpcode, 0, 0, 0, 0, 0, 0, 0, DISOPTYPE_INVALID)
};


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Load)
    DIS_RISCV_OP(0xba000400, "lb",            OP_RISCV_LB),
    DIS_RISCV_OP(0xba000400, "lh",            OP_RISCV_LH),
    DIS_RISCV_OP(0xba000400, "lw",            OP_RISCV_LW),
    DIS_RISCV_OP(0xba000400, "ld",            OP_RISCV_LD),
    DIS_RISCV_OP(0xba000400, "lbu",           OP_RISCV_LBU),
    DIS_RISCV_OP(0xba000400, "lhu",           OP_RISCV_LHU),
    DIS_RISCV_OP(0xba000400, "lwu",           OP_RISCV_LWU),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Load, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       RT_BIT_32(14) | RT_BIT_32(13) | RT_BIT_32(12), 12);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(LoadFp)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(LoadFp, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(MiscMem)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(MiscMem, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(OpImm)
    DIS_RISCV_OP(0xba000400, "addi",    OP_RISCV_ADDI),
    DIS_RISCV_OP(0xba000400, "slli",    OP_RISCV_SLLI),
    DIS_RISCV_OP(0xba000400, "slti",    OP_RISCV_SLTI),
    DIS_RISCV_OP(0xba000400, "sltiu",   OP_RISCV_SLTIU),
    DIS_RISCV_OP(0xba000400, "xori",    OP_RISCV_XORI),
    DIS_RISCV_OP(0xba000400, "srli",    OP_RISCV_SRLI),
    DIS_RISCV_OP(0xba000400, "ori",     OP_RISCV_ORI),
    DIS_RISCV_OP(0xba000400, "andi",    OP_RISCV_ANDI),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(OpImm, 0 /*fFixedInsn*/, kDisParmParseImm,
                                       RT_BIT_32(14) | RT_BIT_32(13) | RT_BIT_32(12), 12);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Auipc)
    DIS_RISCV_OP(0xba000400, "auipc",   OP_RISCV_AUIPC),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Auipc, 0 /*fFixedInsn*/, kDisParmParseU,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(OpImm32)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(OpImm32, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Store)
    DIS_RISCV_OP(0xba000400, "sb",            OP_RISCV_SB),
    DIS_RISCV_OP(0xba000400, "sh",            OP_RISCV_SH),
    DIS_RISCV_OP(0xba000400, "sw",            OP_RISCV_SW),
    DIS_RISCV_OP(0xba000400, "sd",            OP_RISCV_SD),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Store, 0 /*fFixedInsn*/, kDisParmParseStore,
                                       RT_BIT_32(14) | RT_BIT_32(13) | RT_BIT_32(12), 12);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(StoreFp)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(StoreFp, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Amo)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Amo, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Op)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Op, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Lui)
    DIS_RISCV_OP(0xba000400, "lui",         OP_RISCV_LUI),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Lui, 0 /*fFixedInsn*/, kDisParmParseU,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Op32)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Op32, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Madd)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Madd, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Msub)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Msub, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Nmsub)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Nmsub, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Nmadd)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Nmadd, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(OpFp)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(OpFp, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(OpV)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(OpV, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Branch)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Branch, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Jalr)
    DIS_RISCV_OP(0xba000400, "jalr",            OP_RISCV_JALR),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Jalr, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       RT_BIT_32(14) | RT_BIT_32(13) | RT_BIT_32(12), 12);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(Jal)
    DIS_RISCV_OP(0xba000400, "jal",            OP_RISCV_JAL),
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(Jal, 0 /*fFixedInsn*/, kDisParmParseJmp,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(System)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(System, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);


DIS_RISCV_DECODE_INSN_CLASS_DEFINE_BEGIN(OpVe)
    INVALID_OPCODE,
DIS_RISCV_DECODE_INSN_CLASS_DEFINE_END(OpVe, 0 /*fFixedInsn*/, kDisParmParseLoad,
                                       0, 0);

/*

 */
DIS_RISCV_DECODE_MAP_DEFINE_BEGIN(DecodeL0)
    /* 0b00000 */ DIS_RISCV_DECODE_MAP_ENTRY(Load),
    /* 0b00001 */ DIS_RISCV_DECODE_MAP_ENTRY(LoadFp),
    /* 0b00010 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* custom-0 */
    /* 0b00011 */ DIS_RISCV_DECODE_MAP_ENTRY(MiscMem),
    /* 0b00100 */ DIS_RISCV_DECODE_MAP_ENTRY(OpImm),
    /* 0b00101 */ DIS_RISCV_DECODE_MAP_ENTRY(Auipc),
    /* 0b00110 */ DIS_RISCV_DECODE_MAP_ENTRY(OpImm32),
    /* 0b00111 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* reserved */

    /* 0b01000 */ DIS_RISCV_DECODE_MAP_ENTRY(Store),
    /* 0b01001 */ DIS_RISCV_DECODE_MAP_ENTRY(StoreFp),
    /* 0b01010 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* custom-1 */
    /* 0b01011 */ DIS_RISCV_DECODE_MAP_ENTRY(Amo),
    /* 0b01100 */ DIS_RISCV_DECODE_MAP_ENTRY(Op),
    /* 0b01101 */ DIS_RISCV_DECODE_MAP_ENTRY(Lui),
    /* 0b01110 */ DIS_RISCV_DECODE_MAP_ENTRY(Op32),
    /* 0b01111 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* reserved */

    /* 0b10000 */ DIS_RISCV_DECODE_MAP_ENTRY(Madd),
    /* 0b10001 */ DIS_RISCV_DECODE_MAP_ENTRY(Msub),
    /* 0b10010 */ DIS_RISCV_DECODE_MAP_ENTRY(Nmsub),
    /* 0b10011 */ DIS_RISCV_DECODE_MAP_ENTRY(Nmadd),
    /* 0b10100 */ DIS_RISCV_DECODE_MAP_ENTRY(OpFp),
    /* 0b10101 */ DIS_RISCV_DECODE_MAP_ENTRY(OpV),
    /* 0b10110 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* custom-2 */
    /* 0b10111 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* reserved */

    /* 0b11000 */ DIS_RISCV_DECODE_MAP_ENTRY(Branch),
    /* 0b11001 */ DIS_RISCV_DECODE_MAP_ENTRY(Jalr),
    /* 0b11010 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* reserved */
    /* 0b11011 */ DIS_RISCV_DECODE_MAP_ENTRY(Jal),
    /* 0b11100 */ DIS_RISCV_DECODE_MAP_ENTRY(System),
    /* 0b11101 */ DIS_RISCV_DECODE_MAP_ENTRY(OpVe),
    /* 0b11110 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY,  /* custom-3 */
    /* 0b11111 */ DIS_RISCV_DECODE_MAP_INVALID_ENTRY   /* reserved */
DIS_RISCV_DECODE_MAP_DEFINE_END_NON_STATIC(DecodeL0, RT_BIT_32(6) | RT_BIT_32(5) | RT_BIT_32(4) | RT_BIT_32(3) | RT_BIT_32(2), 2);

