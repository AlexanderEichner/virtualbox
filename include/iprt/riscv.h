/** @file
 * IPRT - RISC-V (RV64 and RV32) Structures and Definitions.
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

#ifndef IPRT_INCLUDED_riscv_h
#define IPRT_INCLUDED_riscv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#ifndef VBOX_FOR_DTRACE_LIB
# include <iprt/cdefs.h>
# ifndef RT_IN_ASSEMBLER
#  include <iprt/types.h>
#  include <iprt/assert.h>
# endif
# include <iprt/assertcompile.h>
#else
# pragma D depends_on library vbox-types.d
#endif

/** @defgroup grp_rt_riscv   RISC-V Types and Definitions
 * @ingroup grp_rt
 * @{
 */

/** @name The RISC-V general purpose register encoding.
 * @{ */
#define RISCV_REG_X0                            0
#define RISCV_REG_X1                            1
#define RISCV_REG_X2                            2
#define RISCV_REG_X3                            3
#define RISCV_REG_X4                            4
#define RISCV_REG_X5                            5
#define RISCV_REG_X6                            6
#define RISCV_REG_X7                            7
#define RISCV_REG_X8                            8
#define RISCV_REG_X9                            9
#define RISCV_REG_X10                           10
#define RISCV_REG_X11                           11
#define RISCV_REG_X12                           12
#define RISCV_REG_X13                           13
#define RISCV_REG_X14                           14
#define RISCV_REG_X15                           15
#define RISCV_REG_X16                           16
#define RISCV_REG_X17                           17
#define RISCV_REG_X18                           18
#define RISCV_REG_X19                           19
#define RISCV_REG_X20                           20
#define RISCV_REG_X21                           21
#define RISCV_REG_X22                           22
#define RISCV_REG_X23                           23
#define RISCV_REG_X24                           24
#define RISCV_REG_X25                           25
#define RISCV_REG_X26                           26
#define RISCV_REG_X27                           27
#define RISCV_REG_X28                           28
#define RISCV_REG_X29                           29
#define RISCV_REG_X30                           30
#define RISCV_REG_X31                           31
/** @} */

/** @name RISC-V register ABI aliases
 * @{ */
/** Hardwired zero register, always returns 0, writes are ignored. */
#define RISCV_REG_ZERO                          RISCV_REG_X0
/** Return address register. */
#define RISCV_REG_RA                            RISCV_REG_X1
/** Stack pointer register. */
#define RISCV_REG_SP                            RISCV_REG_X2
/** @} */

/** @} */

#endif /* !IPRT_INCLUDED_riscv_h */

