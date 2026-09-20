/** @file
 * Disassembler - Opcodes
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
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL), a copy of it is provided in the "COPYING.CDDL" file included
 * in the VirtualBox distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR CDDL-1.0
 */

#ifndef VBOX_INCLUDED_disopcode_riscv_h
#define VBOX_INCLUDED_disopcode_riscv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <iprt/assert.h>

/** @defgroup grp_dis_opcodes_riscv Opcodes (DISOPCODE::uOpCode)
 * @ingroup grp_dis
 * @{
 */
enum OPCODESRISCV
{
    /** @name Full RISC-V opcode list.
     * @{ */
    OP_RISCV_INVALID = 0,
    OP_RISCV_ADD,
    OP_RISCV_ADDI,
    OP_RISCV_ADDW,
    OP_RISCV_AMOADD_D,
    OP_RISCV_AMOADD_W,
    OP_RISCV_AMOAND_D,
    OP_RISCV_AMOAND_W,
    OP_RISCV_AMOMIN_D,
    OP_RISCV_AMOMIN_W,
    OP_RISCV_AMOMINU_D,
    OP_RISCV_AMOMINU_W,
    OP_RISCV_AMOMAX_D,
    OP_RISCV_AMOMAX_W,
    OP_RISCV_AMOMAXU_D,
    OP_RISCV_AMOMAXU_W,
    OP_RISCV_AMOOR_D,
    OP_RISCV_AMOOR_W,
    OP_RISCV_AMOSWAP_D,
    OP_RISCV_AMOSWAP_W,
    OP_RISCV_AMOXOR_D,
    OP_RISCV_AMOXOR_W,
    OP_RISCV_AND,
    OP_RISCV_ANDI,
    OP_RISCV_AUIPC,
    OP_RISCV_BEQ,
    OP_RISCV_BGE,
    OP_RISCV_BGEU,
    OP_RISCV_BLT,
    OP_RISCV_BLTU,
    OP_RISCV_BNE,
    OP_RISCV_CSRRC,
    OP_RISCV_CSRRCI,
    OP_RISCV_CSRRS,
    OP_RISCV_CSRRSI,
    OP_RISCV_CSRRW,
    OP_RISCV_CSRRWI,
    OP_RISCV_DIV,
    OP_RISCV_DIVU,
    OP_RISCV_DIVUW,
    OP_RISCV_DIVW,
    OP_RISCV_EBREAK,
    OP_RISCV_ECALL,
    OP_RISCV_FENCE,
    OP_RISCV_FENCE_I,
    OP_RISCV_FENCE_TSO,
    OP_RISCV_JAL,
    OP_RISCV_JALR,
    OP_RISCV_LB,
    OP_RISCV_LBU,
    OP_RISCV_LD,
    OP_RISCV_LH,
    OP_RISCV_LHU,
    OP_RISCV_LUI,
    OP_RISCV_LR_D,
    OP_RISCV_LR_W,
    OP_RISCV_LW,
    OP_RISCV_LWU,
    OP_RISCV_MUL,
    OP_RISCV_MULH,
    OP_RISCV_MULHSU,
    OP_RISCV_MULHU,
    OP_RISCV_MULW,
    OP_RISCV_OR,
    OP_RISCV_ORI,
    OP_RISCV_PAUSE,
    OP_RISCV_REM,
    OP_RISCV_REMU,
    OP_RISCV_REMW,
    OP_RISCV_REMUW,
    OP_RISCV_SB,
    OP_RISCV_SC_D,
    OP_RISCV_SC_W,
    OP_RISCV_SD,
    OP_RISCV_SH,
    OP_RISCV_SLL,
    OP_RISCV_SLLI,
    OP_RISCV_SLLIW,
    OP_RISCV_SLLW,
    OP_RISCV_SLT,
    OP_RISCV_SLTI,
    OP_RISCV_SLTIU,
    OP_RISCV_SLTU,
    OP_RISCV_SRA,
    OP_RISCV_SRAI,
    OP_RISCV_SRAIW,
    OP_RISCV_SRAW,
    OP_RISCV_SRL,
    OP_RISCV_SRLI,
    OP_RISCV_SRLIW,
    OP_RISCV_SRLW,
    OP_RISCV_SUB,
    OP_RISCV_SUBW,
    OP_RISCV_SW,
    OP_RISCV_XOR,
    OP_RISCV_XORI,
    /** @} */

    OP_RISCV_END_OF_OPCODES
};


/** @defgroup grp_dis_opparam_riscv Opcode parameters (DISOPCODE::fParam1,
 *            DISOPCODE::fParam2, DISOPCODE::fParam3)
 * @ingroup grp_dis
 * @{
 */

/**
 * Basic parameter type.
 */
typedef enum DISRISCVOPPARM
{
    /** Parameter is not used. */
    kDisRiscVOpParmNone = 0,
    /** General purpose register. */
    kDisRiscVOpParmGpr,
    /** Immediate value. */
    kDisRiscVOpParmImm,
    /** Relative address immediate. */
    kDisRiscVOpParmImmRel,
    /** CSR (in idCsr). */
    kDisRiscVOpParmCsr,
    /** Accessing memory from address in base register + potential offset. */
    kDisRiscVOpParmAddrInGpr
} DISRISCVOPPARM;


/** @} */

/** @} */

#endif /* !VBOX_INCLUDED_disopcode_riscv_h */

