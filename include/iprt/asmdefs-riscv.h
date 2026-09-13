/** @file
 * IPRT - RISC-V Specific Assembly Macros.
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

#ifndef IPRT_INCLUDED_asmdefs_riscv_h
#define IPRT_INCLUDED_asmdefs_riscv_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <iprt/cdefs.h>

#if !defined(RT_ARCH_RISCV64) && !defined(RT_ARCH_RISCV32)
# error "Not on RV64I or RV32I"
#endif

/** @defgroup grp_rt_asmdefs_riscv  RISC-V Specific ASM (Clang and gcc) Macros
 * @ingroup grp_rt_asm
 * @{
 */

/**
 * Align code, pad with EBREAK. */
#define ALIGNCODE(alignment)    .balignl alignment, 0x00100073

/**
 * Align data, pad with ZEROs. */
#define ALIGNDATA(alignment)    .balign alignment

/**
 * Align BSS, pad with ZEROs. */
#define ALIGNBSS(alignment)     .balign alignment


/** Marks the beginning of a code section. */
#if defined(ASM_FORMAT_ELF)
# define BEGINCODE .section .text
#else
# error "Port me!"
#endif

/** Marks the end of a code section. */
#if defined(ASM_FORMAT_ELF)
# define ENDCODE
#else
# error "Port me!"
#endif


/** Marks the beginning of a data section. */
#if defined(ASM_FORMAT_ELF)
# define BEGINDATA .section .data
#else
# error "Port me!"
#endif

/** Marks the end of a data section. */
#if defined(ASM_FORMAT_ELF)
# define ENDDATA
#else
# error "Port me!"
#endif


/** Marks the beginning of a readonly data section. */
#if defined(ASM_FORMAT_ELF)
# define BEGINCONST .section .rodata
#else
# error "Port me!"
#endif

/** Marks the end of a readonly data section. */
#if defined(ASM_FORMAT_ELF)
# define ENDCONST
#else
# error "Port me!"
#endif


/** Marks the beginning of a readonly C strings section. */
#if defined(ASM_FORMAT_ELF)
# define BEGINCONSTSTRINGS .section .rodata
#else
# error "Port me!"
#endif

/** Marks the end of a readonly C strings section. */
#if defined(ASM_FORMAT_ELF)
# define ENDCONSTSTRINGS
#else
# error "Port me!"
#endif


/**
 * Mangles the name so it can be referenced using DECLASM() in the C/C++ world.
 *
 * @returns a_SymbolC with the necessary prefix/postfix.
 * @param   a_SymbolC   A C symbol name to mangle as needed.
 */
#define NAME(a_SymbolC)    a_SymbolC


/**
 * Returns the page address of the given symbol (used with the adrp instruction primarily).
 *
 * @returns Page aligned address of the given symbol
 * @param   a_Symbol    The symbol to get the page address from.
 */
#if defined(ASM_FORMAT_ELF)
# define PAGE(a_Symbol) %hi(a_Symbol)
# define PAGE_GOT(a_Symbol) %got_pcrel_hi(a_Symbol)
#else
# error "Port me!"
#endif

/**
 * Returns the offset inside the page of the given symbol.
 *
 * @returns Page offset of the given symbol inside a page.
 * @param   a_Symbol    The symbol to get the page offset from.
 */
#if defined(ASM_FORMAT_ELF)
# define PAGEOFF(a_Symbol) %lo(a_Symbol)
# define PAGEOFF_GOT(a_Symbol) %pcrel_lo(a_Symbol)
#else
# error "Port me!"
#endif


/**
 * Global marker which is DECLASM() compatible.
 */
.macro GLOBALNAME, a_Name
        .globl          NAME(\a_Name)
NAME(\a_Name):
.endm


/**
 * Global exported marker which is DECLASM() compatible.
 */
.macro EXPORTEDNAME, a_Name
#if defined(ASM_FORMAT_ELF)
        //.hidden         NAME(\a_Name)
#endif
        .globl          NAME(\a_Name)
NAME(\a_Name):
.endm


/**
 * Starts an externally visible procedure.
 *
 * @param   a_Name      The unmangled symbol name.
 */
.macro BEGINPROC, a_Name
        .globl          NAME(\a_Name)
NAME(\a_Name):
.endm


/**
 * Starts a procedure with hidden visibility.
 *
 * @param   a_Name      The unmangled symbol name.
 */
.macro BEGINPROC_HIDDEN, a_Name
#if defined(ASM_FORMAT_ELF)
        .hidden         NAME(\a_Name)
#endif
        .globl          NAME(\a_Name)
NAME(\a_Name):
.endm


/**
 * Starts an exported procedure.
 *
 * @param   a_Name      The unmangled symbol name.
 */
.macro BEGINPROC_EXPORTED, a_Name
#if defined(ASM_FORMAT_ELF)
        //.hidden         NAME(\a_Name)
#endif
        .globl          NAME(\a_Name)
NAME(\a_Name):
.endm


/**
 * Ends a procedure.
 *
 * @param   a_Name      The unmangled symbol name.
 */
.macro ENDPROC, a_Name
NAME(\a_Name)\()_EndProc:
.endm


/** @} */

#endif /* !IPRT_INCLUDED_asmdefs_riscv_h */

