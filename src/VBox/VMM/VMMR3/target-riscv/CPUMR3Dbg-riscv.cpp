/* $Id: CPUMR3Dbg-armv8.cpp 172109 2026-01-11 19:29:08Z bird $ */
/** @file
 * CPUM - CPU Monitor / Manager, Debugger & Debugging APIs.
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
#define LOG_GROUP LOG_GROUP_DBGF
#include <VBox/vmm/cpum.h>
#include <VBox/vmm/dbgf.h>
#include <VBox/vmm/pdmapic.h>
#include "CPUMInternal.h"
#include <VBox/vmm/vm.h>
#include <VBox/param.h>
#include <VBox/err.h>
#include <VBox/log.h>
#include <iprt/thread.h>
#include <iprt/string.h>
#include <iprt/uint128.h>


/**
 * @interface_method_impl{DBGFREGDESC,pfnGet}
 */
static DECLCALLBACK(int) cpumR3RegGet_Generic(void *pvUser, PCDBGFREGDESC pDesc, PDBGFREGVAL pValue)
{
    PVMCPU      pVCpu   = (PVMCPU)pvUser;
    void const *pv      = (uint8_t const *)&pVCpu->cpum + pDesc->offRegister;

    VMCPU_ASSERT_EMT(pVCpu);

    switch (pDesc->enmType)
    {
        case DBGFREGVALTYPE_U8:        pValue->u8   = *(uint8_t  const *)pv; return VINF_SUCCESS;
        case DBGFREGVALTYPE_U16:       pValue->u16  = *(uint16_t const *)pv; return VINF_SUCCESS;
        case DBGFREGVALTYPE_U32:       pValue->u32  = *(uint32_t const *)pv; return VINF_SUCCESS;
        case DBGFREGVALTYPE_U64:       pValue->u64  = *(uint64_t const *)pv; return VINF_SUCCESS;
        case DBGFREGVALTYPE_U128:      pValue->u128 = *(PCRTUINT128U    )pv; return VINF_SUCCESS;
        case DBGFREGVALTYPE_U256:      pValue->u256 = *(PCRTUINT256U    )pv; return VINF_SUCCESS;
        case DBGFREGVALTYPE_U512:      pValue->u512 = *(PCRTUINT512U    )pv; return VINF_SUCCESS;
        default:
            AssertMsgFailedReturn(("%d %s\n", pDesc->enmType, pDesc->pszName), VERR_IPE_NOT_REACHED_DEFAULT_CASE);
    }
}


/**
 * @interface_method_impl{DBGFREGDESC,pfnSet}
 */
static DECLCALLBACK(int) cpumR3RegSet_Generic(void *pvUser, PCDBGFREGDESC pDesc, PCDBGFREGVAL pValue, PCDBGFREGVAL pfMask)
{
    PVMCPU      pVCpu = (PVMCPU)pvUser;
    void       *pv    = (uint8_t *)&pVCpu->cpum + pDesc->offRegister;

    VMCPU_ASSERT_EMT(pVCpu);

    switch (pDesc->enmType)
    {
        case DBGFREGVALTYPE_U8:
            *(uint8_t *)pv &= ~pfMask->u8;
            *(uint8_t *)pv |= pValue->u8 & pfMask->u8;
            return VINF_SUCCESS;

        case DBGFREGVALTYPE_U16:
            *(uint16_t *)pv &= ~pfMask->u16;
            *(uint16_t *)pv |= pValue->u16 & pfMask->u16;
            return VINF_SUCCESS;

        case DBGFREGVALTYPE_U32:
            *(uint32_t *)pv &= ~pfMask->u32;
            *(uint32_t *)pv |= pValue->u32 & pfMask->u32;
            return VINF_SUCCESS;

        case DBGFREGVALTYPE_U64:
            *(uint64_t *)pv &= ~pfMask->u64;
            *(uint64_t *)pv |= pValue->u64 & pfMask->u64;
            return VINF_SUCCESS;

        case DBGFREGVALTYPE_U128:
        {
            RTUINT128U Val;
            RTUInt128AssignAnd((PRTUINT128U)pv, RTUInt128AssignBitwiseNot(RTUInt128Assign(&Val, &pfMask->u128)));
            RTUInt128AssignOr((PRTUINT128U)pv, RTUInt128AssignAnd(RTUInt128Assign(&Val, &pValue->u128), &pfMask->u128));
            return VINF_SUCCESS;
        }

        default:
            AssertMsgFailedReturn(("%d %s\n", pDesc->enmType, pDesc->pszName), VERR_IPE_NOT_REACHED_DEFAULT_CASE);
    }
}


/*
 * Set up aliases.
 */
/** @todo */
#define CPUMREGALIAS_STD(Name, psz32)  \
    static DBGFREGALIAS const g_aCpumRegAliases_##Name[] = \
    { \
        { psz32, DBGFREGVALTYPE_U32     }, \
        { NULL,  DBGFREGVALTYPE_INVALID } \
    }
CPUMREGALIAS_STD(x0,  "w0");
CPUMREGALIAS_STD(x1,  "w1");
CPUMREGALIAS_STD(x2,  "w2");
CPUMREGALIAS_STD(x3,  "w3");
CPUMREGALIAS_STD(x4,  "w4");
CPUMREGALIAS_STD(x5,  "w5");
CPUMREGALIAS_STD(x6,  "w6");
CPUMREGALIAS_STD(x7,  "w7");
CPUMREGALIAS_STD(x8,  "w8");
CPUMREGALIAS_STD(x9,  "w9");
CPUMREGALIAS_STD(x10, "w10");
CPUMREGALIAS_STD(x11, "w11");
CPUMREGALIAS_STD(x12, "w12");
CPUMREGALIAS_STD(x13, "w13");
CPUMREGALIAS_STD(x14, "w14");
CPUMREGALIAS_STD(x15, "w15");
CPUMREGALIAS_STD(x16, "w16");
CPUMREGALIAS_STD(x17, "w17");
CPUMREGALIAS_STD(x18, "w18");
CPUMREGALIAS_STD(x19, "w19");
CPUMREGALIAS_STD(x20, "w20");
CPUMREGALIAS_STD(x21, "w21");
CPUMREGALIAS_STD(x22, "w22");
CPUMREGALIAS_STD(x23, "w23");
CPUMREGALIAS_STD(x24, "w24");
CPUMREGALIAS_STD(x25, "w25");
CPUMREGALIAS_STD(x26, "w26");
CPUMREGALIAS_STD(x27, "w27");
CPUMREGALIAS_STD(x28, "w28");
CPUMREGALIAS_STD(x29, "w29");
CPUMREGALIAS_STD(x30, "w30");
CPUMREGALIAS_STD(x31, "w31");
#undef CPUMREGALIAS_STD


/** @name Macros for producing register descriptor table entries.
 * @{ */
#define CPU_REG_EX_AS(a_szName, a_RegSuff, a_TypeSuff, a_offRegister, a_pfnGet, a_pfnSet, a_paAliases, a_paSubFields) \
    { a_szName, DBGFREG_RISCV_##a_RegSuff, DBGFREGVALTYPE_##a_TypeSuff, 0 /*fFlags*/, a_offRegister, a_pfnGet, a_pfnSet, a_paAliases, a_paSubFields }

#define CPU_GREG_REG(n) \
    CPU_REG_RW_AS("x" #n,      GREG_X##n,             U64, aGRegs[n], cpumR3RegGet_Generic, cpumR3RegSet_Generic, g_aCpumRegAliases_x##n,  NULL)

/** @} */


/**
 * The guest register descriptors.
 */
static DBGFREGDESC const g_aCpumRegGstDescs[] =
{
#define CPU_REG_RW_AS(a_szName, a_RegSuff, a_TypeSuff, a_CpumCtxMemb, a_pfnGet, a_pfnSet, a_paAliases, a_paSubFields) \
    { a_szName, DBGFREG_RISCV_##a_RegSuff, DBGFREGVALTYPE_##a_TypeSuff, 0 /*fFlags*/,            (uint32_t)RT_UOFFSETOF(CPUMCPU, Guest.a_CpumCtxMemb), a_pfnGet, a_pfnSet, a_paAliases, a_paSubFields }
#define CPU_REG_RO_AS(a_szName, a_RegSuff, a_TypeSuff, a_CpumCtxMemb, a_pfnGet, a_pfnSet, a_paAliases, a_paSubFields) \
    { a_szName, DBGFREG_RISCV_##a_RegSuff, DBGFREGVALTYPE_##a_TypeSuff, DBGFREG_FLAGS_READ_ONLY, (uint32_t)RT_UOFFSETOF(CPUMCPU, Guest.a_CpumCtxMemb), a_pfnGet, a_pfnSet, a_paAliases, a_paSubFields }

    CPU_GREG_REG(0),
    CPU_GREG_REG(1),
    CPU_GREG_REG(2),
    CPU_GREG_REG(3),
    CPU_GREG_REG(4),
    CPU_GREG_REG(5),
    CPU_GREG_REG(6),
    CPU_GREG_REG(7),
    CPU_GREG_REG(8),
    CPU_GREG_REG(9),
    CPU_GREG_REG(10),
    CPU_GREG_REG(11),
    CPU_GREG_REG(12),
    CPU_GREG_REG(13),
    CPU_GREG_REG(14),
    CPU_GREG_REG(15),
    CPU_GREG_REG(16),
    CPU_GREG_REG(17),
    CPU_GREG_REG(18),
    CPU_GREG_REG(19),
    CPU_GREG_REG(20),
    CPU_GREG_REG(21),
    CPU_GREG_REG(22),
    CPU_GREG_REG(23),
    CPU_GREG_REG(24),
    CPU_GREG_REG(25),
    CPU_GREG_REG(26),
    CPU_GREG_REG(27),
    CPU_GREG_REG(28),
    CPU_GREG_REG(29),
    CPU_GREG_REG(30),
    CPU_GREG_REG(31),

    CPU_REG_RW_AS("pc",             PC,             U64, Pc,              cpumR3RegGet_Generic,         cpumR3RegSet_Generic,         NULL,                       NULL                    ),

    DBGFREGDESC_TERMINATOR()

#undef CPU_REG_RW_AS
#undef CPU_REG_RO_AS
};


DECLHIDDEN(int) cpumR3DbgInitTarget(PVM pVM)
{
    for (VMCPUID idCpu = 0; idCpu < pVM->cCpus; idCpu++)
    {
        int rc = DBGFR3RegRegisterCpu(pVM, pVM->apCpusR3[idCpu], g_aCpumRegGstDescs, true /*fGuestRegs*/);
        AssertLogRelRCReturn(rc, rc);
    }

    return VINF_SUCCESS;
}

