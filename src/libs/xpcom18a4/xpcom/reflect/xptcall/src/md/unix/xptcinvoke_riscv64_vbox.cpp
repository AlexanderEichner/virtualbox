/* $Id$ */
/** @file
 * XPCOM - Implementation XPTC_InvokeByIndex for riscv64.
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
#include "xptcprivate.h"
#include <iprt/cdefs.h>
#include <iprt/alloca.h>
#include <iprt/assert.h>

#include "xptc_riscv64_vbox.h"


/*********************************************************************************************************************************
*   Defined Constants And Macros                                                                                                 *
*********************************************************************************************************************************/
#define MY_MAX_ARGS         64  /**< Limit ourselves to 64 arguments. */

#define NSXPTC_VARIANT_SIZE    (8 + 8 + 8)
#define NSXPTC_VARIANT_PTR_OFF 8

AssertCompileSize(nsXPTCVariant, NSXPTC_VARIANT_SIZE);
AssertCompileMemberOffset(nsXPTCVariant, ptr, NSXPTC_VARIANT_PTR_OFF);
AssertCompileMemberOffset(nsXPTCVariant, val, 0);


/*
 * gcc seems to support naked on riscv64 unlike with arm64, but as this is copied from the arm64
 * variant we try to keep the differences as minimal as possible.
 */
extern "C" nsresult
riscv64AsmInvoker(uintptr_t pfnMethod /*a0*/, uint32_t cParams /*a1*/, nsXPTCVariant *paParams /*a2*/, uint64_t cbStack /*a3*/,
                uint8_t *acbStackArgs /*a4*/, uint64_t *pauGprArgs /*a5*/, uint64_t *pauFprArgs /*a6*/, uint32_t cFprArgs /*a7*/);

__asm__ (
"BEGINCODE\n"
"BEGINPROC_HIDDEN riscv64AsmInvoker\n"
        ".cfi_startproc\n"
        /* Prologue - create the frame. */ "\
        addi    sp, sp, -16 \n\
        sd      ra, 0(sp) \n\
        sd      fp, 8(sp) \n\
        mv      fp, sp \n\
        .cfi_def_cfa        fp, 16 \n\
        .cfi_rel_offset     ra, -8 \n\
        .cfi_rel_offset     fp, -16 \n\
\n\
"       /* Move pfnMethod to t0 and pauGprArgs to t1 free up a0 and a5: */ "\
        mv      t0, a0 \n\
        mv      t1, a5 \n\
\n\
"       /* Load FPU registers first so we free up a6 & a7 early: */ "\
        beqz    a7, Lno_fprs \n\
        fld     fa0,   (a6)\n\
        fld     fa1,  8(a6)\n\
        fld     fa2, 16(a6)\n\
        fld     fa3, 24(a6)\n\
        fld     fa4, 32(a6)\n\
        fld     fa5, 40(a6)\n\
        fld     fa6, 48(a6)\n\
        fld     fa7, 54(a6)\n\
Lno_fprs:\n\
\n\
"       /* Do argument passing by stack (if any).  We align the stack to 16 bytes.  */ "\
        beqz    a3, Lno_stack_args \n\
        sub     a3, sp, a3 \n\
        andi    a3, a3, -15 \n\
        mv      sp, a3 \n\
"
"\
Lnext_parameter: \n\
        lbu     a7, (a4) \n\
        beqz    a7, Ladvance \n\
\n\
        li      t2, 4 \n\
        bgt     a7, t2, Lstore_64bits\n\
        li      t2, 1 \n\
        beq     a7, t2, Lstore_8bits\n\
        li      t2, 2 \n\
        beq     a7, t2, Lstore_16bits\n\
\n\
Lstore_32bits:\n\
        lw      a0, (a2) \n\
        addi    a3, a3, 3 \n\
        andi    a3, a3, -4 \n\
        sw      a0, (a3) \n\
        addi    a3, a3, 4 \n"
"       j       Ladvance \n\
\n\
Lstore_8bits:\n\
        lb      a0, (a2) \n\
        sb      a0, (a3) \n\
        addi    a3, a3, 1 \n"
"       j       Ladvance \n\
\n\
Lstore_16bits:\n\
        lh      a0, (a2) \n\
        add     a3, a3, 2 \n\
        andi    a3, a3, -3 \n\
        sh      a0, (a3) \n\
        addi    a3, a3, 2 \n"
"       j       Ladvance \n\
\n\
Lstore_64bits_ptr:\n\
        ld      a0, " RT_XSTR(NSXPTC_VARIANT_PTR_OFF) "(a2) \n\
        j       Lstore_64bits_common \n\
Lstore_64bits:\n\
        add     a3, a3, 7 \n\
        andi    a3, a3, -8 \n\
        li      t2, 0x80 \n\
        bge     a7, t2, Lstore_64bits_ptr \n\
        ld      a0, (a2) \n\
Lstore_64bits_common:\n\
        sd      a0, (a3) \n\
        addi    a3, a3, 8 \n"
"\n\
Ladvance:\n"
"       addi    a4, a4, 1 \n\
        addi    a2, a2, " RT_XSTR(NSXPTC_VARIANT_SIZE) " \n\
        addi    a1, a1, -1 \n\
        bnez    a1, Lnext_parameter \n\
\n\
"       /* reserve stack space for the integer and floating point registers and save them: */ "\
Lno_stack_args: \n\
\n\
"       /* Load general purpose argument registers: */ "\
        ld      a0, (t1) \n\
        ld      a1, 8(t1) \n\
        ld      a2, 16(t1) \n\
        ld      a3, 24(t1) \n\
        ld      a4, 32(t1) \n\
        ld      a5, 40(t1) \n\
        ld      a6, 48(t1) \n\
        ld      a7, 56(t1) \n\
\n\
"       /* Make the call: */ "\
        jalr    ra, t0, 0 \n\
\n\
"       /* Epilogue (clang does not emit the .cfi's here, so drop them too?): */ "\
        mv      sp, fp \n\
        ld      fp, 8(sp)\n\
        ld      ra, 0(sp)\n\
        addi    sp, sp, 16 \n\
        .cfi_def_cfa sp, 0 \n\
        .cfi_restore fp \n\
        .cfi_restore ra \n\
        ret \n"
        ".cfi_endproc\n"
);


XPTC_PUBLIC_API(nsresult)
XPTC_InvokeByIndex(nsISupports *pThis, PRUint32 idxMethod, PRUint32 cParams, nsXPTCVariant *paParams)
{
    AssertMsgReturn(cParams <= MY_MAX_ARGS, ("cParams=%#x idxMethod=%#x\n", cParams, idxMethod), NS_ERROR_UNEXPECTED);

    /*
     * Prepare
     */
    uint64_t auGprArgs[NUM_ARGS_IN_GPRS] = {0};
    uint64_t auFprArgs[NUM_ARGS_IN_GPRS] = {0};
    uint8_t  acbStackArgs[MY_MAX_ARGS]; /* The number of value bytes to copy onto the stack. Zero if in register. */
    uint32_t cbStackArgs = 0;
    uint32_t cFprArgs    = 0;
    uint32_t cGprArgs    = 0;

    /* First argument is always 'pThis'. The 'pThis' argument is not accounted
       for in cParams or acbStackArgs. */
    auGprArgs[cGprArgs++] = (uintptr_t)pThis;

    /* Do the other arguments. */
    for (PRUint32 i = 0; i < cParams; i++)
    {
        if (paParams[i].IsPtrData())
        {
            if (cGprArgs < NUM_ARGS_IN_GPRS)
            {
                auGprArgs[cGprArgs++] = (uintptr_t)paParams[i].ptr;
                acbStackArgs[i] = 0;
            }
            else
            {
                acbStackArgs[i] = sizeof(paParams[i].ptr) | UINT8_C(0x80);
                cbStackArgs    += sizeof(uint64_t);
            }
        }
        else
        {
            if (   paParams[i].type != nsXPTType::T_FLOAT
                 && paParams[i].type != nsXPTType::T_DOUBLE)
            {
                if (cGprArgs < NUM_ARGS_IN_GPRS)
                {
                    switch (paParams[i].type)
                    {
                        case nsXPTType::T_I8:       auGprArgs[cGprArgs++] = paParams[i].val.i8; break;
                        case nsXPTType::T_I16:      auGprArgs[cGprArgs++] = paParams[i].val.i16; break;
                        case nsXPTType::T_I32:      auGprArgs[cGprArgs++] = paParams[i].val.i32; break;
                        case nsXPTType::T_I64:      auGprArgs[cGprArgs++] = paParams[i].val.i64; break;
                        case nsXPTType::T_U8:       auGprArgs[cGprArgs++] = paParams[i].val.u8; break;
                        case nsXPTType::T_U16:      auGprArgs[cGprArgs++] = paParams[i].val.u16; break;
                        case nsXPTType::T_U32:      auGprArgs[cGprArgs++] = paParams[i].val.u32; break;
                        default:
                        case nsXPTType::T_U64:      auGprArgs[cGprArgs++] = paParams[i].val.u64; break;
                        case nsXPTType::T_BOOL:     auGprArgs[cGprArgs++] = paParams[i].val.b; break;
                        case nsXPTType::T_CHAR:     auGprArgs[cGprArgs++] = paParams[i].val.c; break;
                        case nsXPTType::T_WCHAR:    auGprArgs[cGprArgs++] = paParams[i].val.wc; break;
                    }
                    acbStackArgs[i] = 0;
                }
                else
                {
                    uint8_t cbStack;
                    switch (paParams[i].type)
                    {
                        case nsXPTType::T_I8:       cbStack = sizeof(paParams[i].val.i8); break;
                        case nsXPTType::T_I16:      cbStack = sizeof(paParams[i].val.i16); break;
                        case nsXPTType::T_I32:      cbStack = sizeof(paParams[i].val.i32); break;
                        case nsXPTType::T_I64:      cbStack = sizeof(paParams[i].val.i64); break;
                        case nsXPTType::T_U8:       cbStack = sizeof(paParams[i].val.u8); break;
                        case nsXPTType::T_U16:      cbStack = sizeof(paParams[i].val.u16); break;
                        case nsXPTType::T_U32:      cbStack = sizeof(paParams[i].val.u32); break;
                        default:
                        case nsXPTType::T_U64:      cbStack = sizeof(paParams[i].val.u64); break;
                        case nsXPTType::T_BOOL:     cbStack = sizeof(paParams[i].val.b); break;
                        case nsXPTType::T_CHAR:     cbStack = sizeof(paParams[i].val.c); break;
                        case nsXPTType::T_WCHAR:    cbStack = sizeof(paParams[i].val.wc); break;
                    }
                    acbStackArgs[i] = cbStack;
                    cbStackArgs    += sizeof(uint64_t);
                }
            }
            else if (cFprArgs < NUM_ARGS_IN_FPRS)
            {
                AssertCompile(sizeof(paParams[i].val.f) == 4);
                AssertCompile(sizeof(paParams[i].val.d) == 8);
                if (paParams[i].type == nsXPTType::T_FLOAT)
                    auFprArgs[cFprArgs++] = paParams[i].val.u32;
                else
                    auFprArgs[cFprArgs++] = paParams[i].val.u64;
                acbStackArgs[i] = 0;
            }
            else
            {
                uint8_t cbStack;
                if (paParams[i].type == nsXPTType::T_FLOAT)
                    cbStack = sizeof(paParams[i].val.f);
                else
                    cbStack = sizeof(paParams[i].val.d);
                acbStackArgs[i] = cbStack;
                cbStackArgs    += sizeof(uint64_t);
            }
        }
    }

    /*
     * Pass it on to a naked wrapper function that does the nitty gritty work.
     */
    uintptr_t *pauVtable = *(uintptr_t **)pThis;
    return riscv64AsmInvoker(pauVtable[idxMethod], cParams, paParams, cbStackArgs, acbStackArgs, auGprArgs, auFprArgs, cFprArgs);
}

