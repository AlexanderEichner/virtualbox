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
/** Global pointer register. */
#define RISCV_REG_GP                            RISCV_REG_X3
/** Thread pointer register. */
#define RISCV_REG_TP                            RISCV_REG_X4
/** Temporary register 0. */
#define RISCV_REG_T0                            RISCV_REG_X5
/** Temporary register 1. */
#define RISCV_REG_T1                            RISCV_REG_X6
/** Temporary register 2. */
#define RISCV_REG_T2                            RISCV_REG_X7
/** Saved register 0. */
#define RISCV_REG_S0                            RISCV_REG_X8
/** Frame pointer register. */
#define RISCV_REG_FP                            RISCV_REG_X8
/** Saved register 1. */
#define RISCV_REG_S1                            RISCV_REG_X9
/** Argument and return value register 0. */
#define RISCV_REG_A0                            RISCV_REG_X10
/** Argument and return value register 1. */
#define RISCV_REG_A1                            RISCV_REG_X11
/** Argument and return value register 2. */
#define RISCV_REG_A2                            RISCV_REG_X12
/** Argument and return value register 3. */
#define RISCV_REG_A3                            RISCV_REG_X13
/** Argument and return value register 4. */
#define RISCV_REG_A4                            RISCV_REG_X14
/** Argument and return value register 5. */
#define RISCV_REG_A5                            RISCV_REG_X15
/** Argument and return value register 6. */
#define RISCV_REG_A6                            RISCV_REG_X16
/** Argument and return value register 7. */
#define RISCV_REG_A7                            RISCV_REG_X17
/** Saved register 2. */
#define RISCV_REG_S2                            RISCV_REG_X18
/** Saved register 3. */
#define RISCV_REG_S3                            RISCV_REG_X19
/** Saved register 4. */
#define RISCV_REG_S4                            RISCV_REG_X20
/** Saved register 5. */
#define RISCV_REG_S5                            RISCV_REG_X21
/** Saved register 6. */
#define RISCV_REG_S6                            RISCV_REG_X22
/** Saved register 7. */
#define RISCV_REG_S7                            RISCV_REG_X23
/** Saved register 8. */
#define RISCV_REG_S8                            RISCV_REG_X24
/** Saved register 9. */
#define RISCV_REG_S9                            RISCV_REG_X25
/** Saved register 10. */
#define RISCV_REG_S10                           RISCV_REG_X26
/** Saved register 11. */
#define RISCV_REG_S11                           RISCV_REG_X27
/** Temporary register 3. */
#define RISCV_REG_T3                            RISCV_REG_X28
/** Temporary register 4. */
#define RISCV_REG_T4                            RISCV_REG_X29
/** Temporary register 5. */
#define RISCV_REG_T5                            RISCV_REG_X30
/** Temporary register 6. */
#define RISCV_REG_T6                            RISCV_REG_X31
/** @} */

/** @name The RISC-V floating point register encoding.
 * @{ */
#define RISCV_REG_F0                            0
#define RISCV_REG_F1                            1
#define RISCV_REG_F2                            2
#define RISCV_REG_F3                            3
#define RISCV_REG_F4                            4
#define RISCV_REG_F5                            5
#define RISCV_REG_F6                            6
#define RISCV_REG_F7                            7
#define RISCV_REG_F8                            8
#define RISCV_REG_F9                            9
#define RISCV_REG_F10                           10
#define RISCV_REG_F11                           11
#define RISCV_REG_F12                           12
#define RISCV_REG_F13                           13
#define RISCV_REG_F14                           14
#define RISCV_REG_F15                           15
#define RISCV_REG_F16                           16
#define RISCV_REG_F17                           17
#define RISCV_REG_F18                           18
#define RISCV_REG_F19                           19
#define RISCV_REG_F20                           20
#define RISCV_REG_F21                           21
#define RISCV_REG_F22                           22
#define RISCV_REG_F23                           23
#define RISCV_REG_F24                           24
#define RISCV_REG_F25                           25
#define RISCV_REG_F26                           26
#define RISCV_REG_F27                           27
#define RISCV_REG_F28                           28
#define RISCV_REG_F29                           29
#define RISCV_REG_F30                           30
#define RISCV_REG_F31                           31
/** @} */

/** @name The RISC-V vector register encoding.
 * @{ */
#define RISCV_REG_V0                            0
#define RISCV_REG_V1                            1
#define RISCV_REG_V2                            2
#define RISCV_REG_V3                            3
#define RISCV_REG_V4                            4
#define RISCV_REG_V5                            5
#define RISCV_REG_V6                            6
#define RISCV_REG_V7                            7
#define RISCV_REG_V8                            8
#define RISCV_REG_V9                            9
#define RISCV_REG_V10                           10
#define RISCV_REG_V11                           11
#define RISCV_REG_V12                           12
#define RISCV_REG_V13                           13
#define RISCV_REG_V14                           14
#define RISCV_REG_V15                           15
#define RISCV_REG_V16                           16
#define RISCV_REG_V17                           17
#define RISCV_REG_V18                           18
#define RISCV_REG_V19                           19
#define RISCV_REG_V20                           20
#define RISCV_REG_V21                           21
#define RISCV_REG_V22                           22
#define RISCV_REG_V23                           23
#define RISCV_REG_V24                           24
#define RISCV_REG_V25                           25
#define RISCV_REG_V26                           26
#define RISCV_REG_V27                           27
#define RISCV_REG_V28                           28
#define RISCV_REG_V29                           29
#define RISCV_REG_V30                           30
#define RISCV_REG_V31                           31
/** @} */

/** @name The RISC-V Control and Status Register (CSR) encodings.
 * @{ */
/** @} */
/** FFLAGS - Floating-Point Accrued Exceptions register - URW. */
#define RISCV_CSR_URW_FFLAGS                    UINT16_C(0x0001)
/** FRM - Floating-Point Dynamic Rounding Mode register - URW. */
#define RISCV_CSR_URW_FRM                       UINT16_C(0x0002)
/** FCSR - Floating-Point Control and Status register - URW. */
#define RISCV_CSR_URW_FCSR                      UINT16_C(0x0003)

/** VSTART - Vector start position register - URW. */
#define RISCV_CSR_URW_VSTART                    UINT16_C(0x0008)
/** VXSAT - Fixed-point accrued saturation flag register - URW. */
#define RISCV_CSR_URW_VXSAT                     UINT16_C(0x0009)
/** VXRM - Fixed-point rounding mode register - URW. */
#define RISCV_CSR_URW_VXRM                      UINT16_C(0x000a)
/** VXCSR - Vector control and status register - URW. */
#define RISCV_CSR_URW_VXCSR                     UINT16_C(0x000f)

/** SSP - Shadow stack pointer register - URW. */
#define RISCV_CSR_URW_SSP                       UINT16_C(0x0011)
/** SEED - Seed for cryptographic random bit generators register - URW. */
#define RISCV_CSR_URW_SEED                      UINT16_C(0x0015)
/** JVT - Table jump base vector and control register - URW. */
#define RISCV_CSR_URW_JVT                       UINT16_C(0x0017)

/** SSTATUS - Supervisor Status register - SRW. */
#define RISCV_CSR_SRW_SSTATUS                   UINT16_C(0x0100)
/** SIE - Supervisor interrupt enable register - SRW. */
#define RISCV_CSR_SRW_SIE                       UINT16_C(0x0104)
/** STVEC - Supervisor trap handler base address register - SRW. */
#define RISCV_CSR_SRW_STVEC                     UINT16_C(0x0105)
/** SCOUNTEREN - Supervisor counter enable register - SRW. */
#define RISCV_CSR_SRW_SCOUNTEREN                UINT16_C(0x0106)
/** SENVCFG - Supervisor environment configuration register - SRW. */
#define RISCV_CSR_SRW_SENVCFG                   UINT16_C(0x010a)
/** SSTATEEN0 - Supervisor state enable 0 register - SRW. */
#define RISCV_CSR_SRW_SSTATEEN0                 UINT16_C(0x010c)
/** SSTATEEN1 - Supervisor state enable 1 register - SRW. */
#define RISCV_CSR_SRW_SSTATEEN1                 UINT16_C(0x010d)
/** SSTATEEN2 - Supervisor state enable 2 register - SRW. */
#define RISCV_CSR_SRW_SSTATEEN2                 UINT16_C(0x010e)
/** SSTATEEN3 - Supervisor state enable 3 register - SRW. */
#define RISCV_CSR_SRW_SSTATEEN3                 UINT16_C(0x010f)

/** SCOUNTINHIBIT - Supervisor counter inhibit register - SRW. */
#define RISCV_CSR_SRW_SCOUNTINHIBIT             UINT16_C(0x0120)

/** SSCRATCH - Supervisor scratch register - SRW. */
#define RISCV_CSR_SRW_SSCRATCH                  UINT16_C(0x0140)
/** SSCRATCH - Supervisor exception program counter register - SRW. */
#define RISCV_CSR_SRW_SEPC                      UINT16_C(0x0141)
/** SCAUSE - Supervisor trap cause register - SRW. */
#define RISCV_CSR_SRW_SCAUSE                    UINT16_C(0x0142)
/** STVAL - Supervisor trap value register - SRW. */
#define RISCV_CSR_SRW_STVAL                     UINT16_C(0x0143)
/** SIP - Supervisor interrupt pending register - SRW. */
#define RISCV_CSR_SRW_SIP                       UINT16_C(0x0144)
/** STIMECMP - Supervisor timer compare - SRW. */
#define RISCV_CSR_SRW_STIMECMP                  UINT16_C(0x014d)
/** SCTRCTL - Supervisor control transfer records control register - SRW. */
#define RISCV_CSR_SRW_SCTRCTL                   UINT16_C(0x014e)
/** SCTRSTATUS - Supervisor control transfer records status register - SRW. */
#define RISCV_CSR_SRW_SCTRSTATUS                UINT16_C(0x014f)

/** SISELECT - Supervisor indirect register select - SRW. */
#define RISCV_CSR_SRW_SISELECT                  UINT16_C(0x0150)
/** SIREG - Supervisor indirect register alias - SRW. */
#define RISCV_CSR_SRW_SIREG                     UINT16_C(0x0151)
/** SIREG2 - Supervisor indirect register alias 2 - SRW. */
#define RISCV_CSR_SRW_SIREG2                    UINT16_C(0x0152)
/** SIREG3 - Supervisor indirect register alias 3 - SRW. */
#define RISCV_CSR_SRW_SIREG3                    UINT16_C(0x0153)
/** SIREG4 - Supervisor indirect register alias 4 - SRW. */
#define RISCV_CSR_SRW_SIREG4                    UINT16_C(0x0155)
/** SIREG5 - Supervisor indirect register alias 5 - SRW. */
#define RISCV_CSR_SRW_SIREG5                    UINT16_C(0x0156)
/** SIREG6 - Supervisor indirect register alias 6 - SRW. */
#define RISCV_CSR_SRW_SIREG6                    UINT16_C(0x0157)
/** SCTRDEPTH - Supervisor control transfer records depth register - SRW. */
#define RISCV_CSR_SRW_SCTRDEPTH                 UINT16_C(0x015f)

/** SATP - Supervisor address translation and protection register - SRW. */
#define RISCV_CSR_SRW_SATP                      UINT16_C(0x0180)
/** SRMCFG - Supervisor resource management register - SRW. */
#define RISCV_CSR_SRW_SRMCFG                    UINT16_C(0x0181)

/** SCONTEXT - Supervisor mode context register - SRW. */
#define RISCV_CSR_SRW_SCONTEXT                  UINT16_C(0x05a8)

/** CYCLE - Cycle counter register (for rdcycle) - URO. */
#define RISCV_CSR_URO_CYCLE                     UINT16_C(0x0c00)
/** TIME -  Time register (for rdtime) - URO. */
#define RISCV_CSR_URO_TIME                      UINT16_C(0x0c01)
/** INSTRET -  Instructions retired counter register (for rdinstret) - URO. */
#define RISCV_CSR_URO_INSTRET                   UINT16_C(0x0c02)
/** HPMCOUNTER3 -  Performance monitoring counter register 3 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER3               UINT16_C(0x0c03)
/** HPMCOUNTER4 -  Performance monitoring counter register 4 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER4               UINT16_C(0x0c04)
/** HPMCOUNTER5 -  Performance monitoring counter register 5 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER5               UINT16_C(0x0c05)
/** HPMCOUNTER6 -  Performance monitoring counter register 6 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER6               UINT16_C(0x0c06)
/** HPMCOUNTER7 -  Performance monitoring counter register 7 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER7               UINT16_C(0x0c07)
/** HPMCOUNTER8 -  Performance monitoring counter register 8 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER8               UINT16_C(0x0c08)
/** HPMCOUNTER9 -  Performance monitoring counter register 9 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER9               UINT16_C(0x0c09)
/** HPMCOUNTER10 -  Performance monitoring counter register 10 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER10              UINT16_C(0x0c0a)
/** HPMCOUNTER11 -  Performance monitoring counter register 11 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER11              UINT16_C(0x0c0b)
/** HPMCOUNTER12 -  Performance monitoring counter register 12 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER12              UINT16_C(0x0c0c)
/** HPMCOUNTER13 -  Performance monitoring counter register 13 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER13              UINT16_C(0x0c0d)
/** HPMCOUNTER14 -  Performance monitoring counter register 14 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER14              UINT16_C(0x0c0e)
/** HPMCOUNTER15 -  Performance monitoring counter register 15 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER15              UINT16_C(0x0c0f)
/** HPMCOUNTER16 -  Performance monitoring counter register 16 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER16              UINT16_C(0x0c10)
/** HPMCOUNTER17 -  Performance monitoring counter register 17 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER17              UINT16_C(0x0c11)
/** HPMCOUNTER18 -  Performance monitoring counter register 18 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER18              UINT16_C(0x0c12)
/** HPMCOUNTER19 -  Performance monitoring counter register 19 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER19              UINT16_C(0x0c13)
/** HPMCOUNTER20 -  Performance monitoring counter register 20 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER20              UINT16_C(0x0c14)
/** HPMCOUNTER21 -  Performance monitoring counter register 21 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER21              UINT16_C(0x0c15)
/** HPMCOUNTER22 -  Performance monitoring counter register 22 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER22              UINT16_C(0x0c16)
/** HPMCOUNTER23 -  Performance monitoring counter register 23 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER23              UINT16_C(0x0c17)
/** HPMCOUNTER24 -  Performance monitoring counter register 24 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER24              UINT16_C(0x0c18)
/** HPMCOUNTER25 -  Performance monitoring counter register 25 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER25              UINT16_C(0x0c19)
/** HPMCOUNTER26 -  Performance monitoring counter register 26 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER26              UINT16_C(0x0c1a)
/** HPMCOUNTER27 -  Performance monitoring counter register 27 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER27              UINT16_C(0x0c1b)
/** HPMCOUNTER28 -  Performance monitoring counter register 28 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER28              UINT16_C(0x0c1c)
/** HPMCOUNTER29 -  Performance monitoring counter register 29 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER29              UINT16_C(0x0c1d)
/** HPMCOUNTER30 -  Performance monitoring counter register 30 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER30              UINT16_C(0x0c1e)
/** HPMCOUNTER31 -  Performance monitoring counter register 31 - URO. */
#define RISCV_CSR_URO_HPMCOUNTER31              UINT16_C(0x0c1f)

/** VL - Vector length register - URO. */
#define RISCV_CSR_URO_VL                        UINT16_C(0x0c20)
/** VTYPE - Vector data type register - URO. */
#define RISCV_CSR_URO_VTYPE                     UINT16_C(0x0c21)
/** VLENB - Vector register length in bytes - URO. */
#define RISCV_CSR_URO_VLENB                     UINT16_C(0x0c22)

/** SCOUNTOVF - Supervisor count overflow register - SRW. */
#define RISCV_CSR_SRW_SCOUNTEROVF               UINT16_C(0x0da0)

/** @} */

#endif /* !IPRT_INCLUDED_riscv_h */

