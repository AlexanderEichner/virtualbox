/** @file
 * DIS - The VirtualBox Disassembler.
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

#ifndef VBOX_INCLUDED_dis_riscv_h
#define VBOX_INCLUDED_dis_riscv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <VBox/types.h>
#include <VBox/disopcode-riscv.h>
#include <iprt/assert.h>


RT_C_DECLS_BEGIN

/** @addtogroup grp_dis   VBox Disassembler
 * @{ */

/**
 * Opcode parameter (operand) details.
 */
typedef struct
{
    /** Parameter type (Actually DISRISCVOPPARM). */
    uint8_t                         enmType;
    /** Parameter size. */
    uint8_t                         cb;
    /** The operand. */
    union
    {
        /** General register index (DISGREG_XXX), applicable if DISUSE_REG_GEN32
         * or DISUSE_REG_GEN64 is set in fUse. */
        uint8_t                     u8Gpr;
        /** IPRT CSR ID. */
        uint16_t                    idCsr;
    } Op;
    union
    {
        /** Offset from the base register. */
        int32_t                     offBase;
    } u;
} DIS_OP_PARAM_RISCV_T;
AssertCompile(sizeof(DIS_OP_PARAM_RISCV_T) <= 16);
/** Pointer to opcode parameter. */
typedef DIS_OP_PARAM_RISCV_T *PDIS_OP_PARAM_RISCV_T;
/** Pointer to opcode parameter. */
typedef const DIS_OP_PARAM_RISCV_T *PCDIS_OP_PARAM_RISCV_T;


/**
 * The RISC-V specific disassembler state and result.
 */
typedef struct
{
    /** Operand size (for loads/stores primarily). */
    uint8_t                     cbOperand;
} DIS_STATE_RISCV_T;
AssertCompile(sizeof(DIS_STATE_RISCV_T) <= 32);


/** @} */

RT_C_DECLS_END

#endif /* !VBOX_INCLUDED_dis_riscv_h */

