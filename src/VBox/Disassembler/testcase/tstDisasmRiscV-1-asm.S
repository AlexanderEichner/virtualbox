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

#include <iprt/asmdefs-riscv.h>

.option norvc

BEGINPROC_HIDDEN TestProcRiscV

        /* Load instructions */
        lb x0, +0(x1)
        lb x0, 2047(x30)
        lb x0, -2048(x30)

        lh x0, +0(x1)
        lh x0, 2047(x30)
        lh x0, -2048(x30)

        lw x0, +0(x1)
        lw x0, 2047(x30)
        lw x0, -2048(x30)

        ld x0, +0(x1)
        ld x0, 2047(x30)
        ld x0, -2048(x30)

        lbu x0, +0(x1)
        lbu x0, 2047(x30)
        lbu x0, -2048(x30)

        lhu x0, +0(x1)
        lhu x0, 2047(x30)
        lhu x0, -2048(x30)

        lwu x0, +0(x1)
        lwu x0, 2047(x30)
        lwu x0, -2048(x30)

        /* Store instructions */
        sb x0, +0(x1)
        sb x0, 2047(x30)
        sb x0, -2048(x30)

        sh x0, +0(x1)
        sh x0, 2047(x30)
        sh x0, -2048(x30)

        sw x0, +0(x1)
        sw x0, 2047(x30)
        sw x0, -2048(x30)

        sd x0, +0(x1)
        sd x0, 2047(x30)
        sd x0, -2048(x30)

        /* AUPIC */
        auipc x1, 0x0
        auipc x1, 100
        auipc x1, 0xff000
        auipc x1, 0xfffff

        /* LUI */
        lui x1, 0x0
        lui x1, 100
        lui x1, 0xff000
        lui x1, 0xfffff

        /* Immediates */
        addi x2, x3, 0
        addi x2, x3, 2047
        addi x2, x3, -2048

        slti x2, x3, 0
        slti x2, x3, 2047
        slti x2, x3, -2048

        sltiu x2, x3, 0
        sltiu x2, x3, 2047
        sltiu x2, x3, -2048

        xori x2, x3, 0
        xori x2, x3, 2047
        xori x2, x3, -2048

        ori x2, x3, 0
        ori x2, x3, 2047
        ori x2, x3, -2048

        andi x2, x3, 0
        andi x2, x3, 2047
        andi x2, x3, -2048

        /** @todo slli, slri, srai */

        /* JALR */
        jalr x0, 0(x2)
        jalr x0, -2048(x2)
        jalr x0, 2047(x2)

        /*
         * Keep last so the testcase can catch errors in
         * the disassembly of the last instruction.
         */
        lb x0, 0(x1)

BEGINPROC_HIDDEN TestProcRiscV_EndProc

