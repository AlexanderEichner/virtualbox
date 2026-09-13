/** @file
 * IPRT - RISC-V Specific Assembly Functions.
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

#ifndef IPRT_INCLUDED_asm_riscv_h
#define IPRT_INCLUDED_asm_rsicv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <iprt/types.h>
#if !defined(RT_ARCH_RISCV64) && !defined(RT_ARCH_RISCV32)
# error "Not on RV64I or RV32I"
#endif

/** @defgroup grp_rt_asm_riscv  RISC-V Specific ASM Routines
 * @ingroup grp_rt_asm
 * @{
 */

/**
 * Gets the content of the CNTVCT_EL0 (or CNTPCT) register.
 *
 * @returns cacle MSR value.
 * @note    We call this TSC to better fit in with existing x86/amd64 based code.
 */
#if RT_INLINE_ASM_EXTERNAL
DECLASM(uint64_t) ASMReadTSC(void);
#else
DECLINLINE(uint64_t) ASMReadTSC(void)
{
# if RT_INLINE_ASM_GNU_STYLE
    uint64_t u64;
#  ifdef RT_ARCH_RISCV64
    __asm__ __volatile__("Lstart_ASMReadTSC_%=:\n\t"
                         "rdcycle %0\n\t"
                         : "=r" (u64));
#  else
#   error "Port me"
#  endif
    return u64;

# else
#  error "Unsupported compiler"
# endif
}
#endif


/** @} */
#endif /* !IPRT_INCLUDED_asm_riscv_h */

