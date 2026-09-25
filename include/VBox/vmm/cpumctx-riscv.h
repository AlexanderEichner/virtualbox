/** @file
 * CPUM - CPU Monitor(/ Manager), Context Structures for the RISC-V emulation/virtualization.
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

#ifndef VBOX_INCLUDED_vmm_cpumctx_riscv_h
#define VBOX_INCLUDED_vmm_cpumctx_riscv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#ifndef VBOX_FOR_DTRACE_LIB
# include <iprt/assertcompile.h>
# include <VBox/types.h>
#else
# pragma D depends_on library riscv.d
#endif


RT_C_DECLS_BEGIN

/** @defgroup grp_cpum_ctx  The CPUM Context Structures
 * @ingroup grp_cpum
 * @{
 */

/** A general register (union). */
typedef union CPUMCTXGREG
{
    /** 64-bit register view. */
    uint64_t            x;
    /** 32-bit register view. */
    uint32_t            w;
} CPUMCTXGREG;
#ifndef VBOX_FOR_DTRACE_LIB
AssertCompileSize(CPUMCTXGREG, 8);
#endif


/**
 * V<n> register union.
 */
typedef union CPUMCTXVREG
{
    /** V Register view. */
    RTUINT128U  v;
    /** 8-bit view. */
    uint8_t     au8[16];
    /** 16-bit view. */
    uint16_t    au16[8];
    /** 32-bit view. */
    uint32_t    au32[4];
    /** 64-bit view. */
    uint64_t    au64[2];
    /** Signed 8-bit view. */
    int8_t      ai8[16];
    /** Signed 16-bit view. */
    int16_t     ai16[8];
    /** Signed 32-bit view. */
    int32_t     ai32[4];
    /** Signed 64-bit view. */
    int64_t     ai64[2];
    /** 128-bit view. (yeah, very helpful) */
    uint128_t   au128[1];
    /** Single precision floating point view. */
    RTFLOAT32U  ar32[4];
    /** Double precision floating point view. */
    RTFLOAT64U  ar64[2];
} CPUMCTXVREG;
#ifndef VBOX_FOR_DTRACE_LIB
AssertCompileSize(CPUMCTXVREG, 16);
#endif
/** Pointer to an V<n> register state. */
typedef CPUMCTXVREG *PCPUMCTXVREG;
/** Pointer to a const V<n> register state. */
typedef CPUMCTXVREG const *PCCPUMCTXVREG;


/**
 * A system level register.
 */
typedef union CPUMCTXSYSREG
{
    /** 64-bit view. */
    uint64_t    u64;
    /** 32-bit view. */
    uint32_t    u32;
} CPUMCTXSYSREG;
#ifndef VBOX_FOR_DTRACE_LIB
AssertCompileSize(CPUMCTXSYSREG, 8);
#endif


/**
 * CPU context.
 */
typedef struct CPUMCTX
{
    /** The general purpose register array view. */
    CPUMCTXGREG         aGRegs[32];
    /** The program counter. */
    CPUMCTXSYSREG       Pc;

    /** Externalized state tracker, CPUMCTX_EXTRN_XXX. */
    uint64_t            fExtrn;
    uint64_t            au64Padding[6];
} CPUMCTX;


#ifndef VBOX_FOR_DTRACE_LIB
AssertCompileSizeAlignment(CPUMCTX, 64);
AssertCompileSizeAlignment(CPUMCTX, 32);
AssertCompileSizeAlignment(CPUMCTX, 16);
AssertCompileSizeAlignment(CPUMCTX, 8);
#endif /* !VBOX_FOR_DTRACE_LIB */


/** @name CPUMCTX_EXTRN_XXX
 * Used for parts of the CPUM state that is externalized and needs fetching
 * before use.
 *
 * @{ */
/** External state keeper: Invalid.  */
#define CPUMCTX_EXTRN_KEEPER_INVALID            UINT64_C(0x0000000000000000)
/** External state keeper: NEM. */
#define CPUMCTX_EXTRN_KEEPER_NEM                UINT64_C(0x0000000000000001)
/** External state keeper mask. */
#define CPUMCTX_EXTRN_KEEPER_MASK               UINT64_C(0x0000000000000003)

/** The PC register value is kept externally. */
#define CPUMCTX_EXTRN_PC                        UINT64_C(0x0000000000000004)
/** The RA register is kept externally. */
#define CPUMCTX_EXTRN_RA                        UINT64_C(0x0000000000000008)
/** The SP register is kept externally. */
#define CPUMCTX_EXTRN_SP                        UINT64_C(0x0000000000000010)
/** The X3 through X31 register values are kept externally. */
#define CPUMCTX_EXTRN_X3_X31                    UINT64_C(0x0000000000000020)
/** General purpose registers mask. */
#define CPUMCTX_EXTRN_GPRS_MASK                 UINT64_C(0x000000000000003c)

/** Mask of bits the keepers can use for state tracking. */
#define CPUMCTX_EXTRN_KEEPER_STATE_MASK         UINT64_C(0xffff000000000000)

/** All CPUM state bits, not including keeper specific ones. */
#define CPUMCTX_EXTRN_ALL                       UINT64_C(0x0000000000fe7ffc)
/** All CPUM state bits, including keeper specific ones. */
#define CPUMCTX_EXTRN_ABSOLUTELY_ALL            UINT64_C(0xfffffffffffffffc)
/** @} */

/** @}  */

RT_C_DECLS_END

#endif /* !VBOX_INCLUDED_vmm_cpumctx_armv8_h */

