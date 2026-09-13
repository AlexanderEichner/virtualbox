/* $Id: CPUMR3-armv8.cpp 172109 2026-01-11 19:29:08Z bird $ */
/** @file
 * CPUM - CPU Monitor / Manager (RISC-V variant).
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

/** @page pg_cpum CPUM - CPU Monitor / Manager
 *
 * The CPU Monitor / Manager keeps track of all the CPU registers.
 * This is the RISC-V variant which is doing much less than its x86/AMD6464
 * counterpart due to the fact that we currently only support the NEM backends
 * for running RISC-V guests. It might become complex iff we decide to implement our
 * own hypervisor.
 *
 * @section sec_cpum_logging_riscv      Logging Level Assignments.
 *
 * Following log level assignments:
 *      - @todo
 *
 */


/*********************************************************************************************************************************
*   Header Files                                                                                                                 *
*********************************************************************************************************************************/
#define LOG_GROUP LOG_GROUP_CPUM
#define CPUM_WITH_NONCONST_HOST_FEATURES
#include <VBox/vmm/cpum.h>
#include <VBox/vmm/pgm.h>
#include <VBox/vmm/mm.h>
#include <VBox/vmm/em.h>
#include <VBox/vmm/iem.h>
#include <VBox/vmm/dbgf.h>
#include <VBox/vmm/ssm.h>
#include "CPUMInternal.h"
#include <VBox/vmm/vm.h>

#include <VBox/param.h>
#include <VBox/dis.h>
#include <VBox/err.h>
#include <VBox/log.h>
#include <iprt/assert.h>
#include <iprt/cpuset.h>
#include <iprt/mem.h>
#include <iprt/mp.h>
#include <iprt/string.h>
#include <iprt/riscv.h>


/*********************************************************************************************************************************
*   Defined Constants And Macros                                                                                                 *
*********************************************************************************************************************************/

/** Internal form used by the macros. */
#ifdef VBOX_WITH_STATISTICS
# define RINT(a_uFirst, a_uLast, a_enmRdFn, a_enmWrFn, a_offCpumCpu, a_uInitOrReadValue, a_fWrIgnMask, a_fWrGpMask, a_szName) \
    { a_uFirst, a_uLast, a_enmRdFn, a_enmWrFn, a_offCpumCpu, 0, 0, a_uInitOrReadValue, a_fWrIgnMask, a_fWrGpMask, a_szName, \
      { 0 }, { 0 }, { 0 }, { 0 } }
#else
# define RINT(a_uFirst, a_uLast, a_enmRdFn, a_enmWrFn, a_offCpumCpu, a_uInitOrReadValue, a_fWrIgnMask, a_fWrGpMask, a_szName) \
    { a_uFirst, a_uLast, a_enmRdFn, a_enmWrFn, a_offCpumCpu, 0, 0, a_uInitOrReadValue, a_fWrIgnMask, a_fWrGpMask, a_szName }
#endif

/** Function handlers, extended version. */
#define MFX(a_uMsr, a_szName, a_enmRdFnSuff, a_enmWrFnSuff, a_uValue, a_fWrIgnMask, a_fWrGpMask) \
    RINT(a_uMsr, a_uMsr, kCpumSysRegRdFn_##a_enmRdFnSuff, kCpumSysRegWrFn_##a_enmWrFnSuff, 0, a_uValue, a_fWrIgnMask, a_fWrGpMask, a_szName)
/** Function handlers, read-only. */
#define MFO(a_uMsr, a_szName, a_enmRdFnSuff) \
    RINT(a_uMsr, a_uMsr, kCpumSysRegRdFn_##a_enmRdFnSuff, kCpumSysRegWrFn_ReadOnly, 0, 0, 0, UINT64_MAX, a_szName)
/** Read-only fixed value, ignores all writes. */
#define MVI(a_uMsr, a_szName, a_uValue) \
    RINT(a_uMsr, a_uMsr, kCpumSysRegRdFn_FixedValue, kCpumSysRegWrFn_IgnoreWrite, 0, a_uValue, UINT64_MAX, 0, a_szName)
/** Read/Write value from/to CPUMCTX. */
#define MVRW(a_uMsr, a_szName, a_offCpum) \
    RINT(a_uMsr, a_uMsr, kCpumSysRegRdFn_ReadCpumOff, kCpumSysRegWrFn_WriteCpumOff, (uint32_t)a_offCpum, 0, UINT64_MAX, 0, a_szName)


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/
/** Saved state field descriptors for CPUMCTX. */
static const SSMFIELD g_aCpumCtxFields[] =
{
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[0].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[1].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[2].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[3].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[4].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[5].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[6].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[7].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[8].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[9].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[10].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[11].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[12].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[13].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[14].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[15].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[16].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[17].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[18].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[19].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[20].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[21].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[22].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[23].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[24].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[25].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[26].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[27].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[28].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[29].x),
    SSMFIELD_ENTRY(         CPUMCTX, aGRegs[30].x),
    SSMFIELD_ENTRY(         CPUMCTX, Pc.u64),

    SSMFIELD_ENTRY_TERM()
};



/*********************************************************************************************************************************
*   Initialization, Reset & Termination                                                                                          *
*********************************************************************************************************************************/


DECLHIDDEN(int) cpumR3InitTarget(PVM pVM)
{
    LogFlow(("cpumR3InitTarget\n"));

    PCFGMNODE   pCpumCfg = CFGMR3GetChild(CFGMR3GetRoot(pVM), "CPUM");

    /** @cfgm{/CPUM/ResetPcValue, string}
     * Program counter value after a reset, sets the address of the first instruction to execute. */
    int rc = CFGMR3QueryU64Def(pCpumCfg, "ResetPcValue", &pVM->cpum.s.u64ResetPc, 0);
    AssertLogRelRCReturn(rc, rc);

    /** @cfgm{/CPUM/ResetX1Value, string}
     * X1 register value after a reset, sets the address of the first instruction to execute. */
    rc = CFGMR3QueryU64Def(pCpumCfg, "ResetX1Value", &pVM->cpum.s.u64ResetX1, 0);
    AssertLogRelRCReturn(rc, rc);

    /*
     * Initialize the Guest CPU ID stuff.
     */
    Assert(pVM->bMainExecutionEngine != VM_EXEC_ENGINE_NOT_SET);
    /** @todo */

#if 0
    /*
     * Register RISC-V specific info items
     */
    DBGFR3InfoRegisterInternal(pVM, "cpufeat", "Displays the guest features.", &cpumR3CpuFeatInfo);
#endif

    return VINF_SUCCESS;
}


DECLHIDDEN(int) cpumR3InitCompletedRing3Target(PVM pVM)
{
    RT_NOREF(pVM);
    return VINF_SUCCESS;
}


DECLHIDDEN(int) cpumR3TermTarget(PVM pVM)
{
    RT_NOREF(pVM);
    return VINF_SUCCESS;
}


/**
 * Resets a virtual CPU.
 *
 * Used by CPUMR3Reset and CPU hot plugging.
 *
 * @param   pVM     The cross context VM structure.
 * @param   pVCpu   The cross context virtual CPU structure of the CPU that is
 *                  being reset.  This may differ from the current EMT.
 */
VMMR3DECL(void) CPUMR3ResetCpu(PVM pVM, PVMCPU pVCpu)
{
    RT_NOREF(pVM);

    /** @todo anything different for VCPU > 0? */
    PCPUMCTX pCtx = &pVCpu->cpum.s.Guest;

    /*
     * Initialize everything to ZERO first.
     */
    RT_BZERO(pCtx, sizeof(*pCtx));

    /* Start in Supervisor mode. */
    pCtx->Pc.u64                 = pVM->cpum.s.u64ResetPc;
    pCtx->aGRegs[RISCV_REG_X0].x = pVCpu->idCpu;
    pCtx->aGRegs[RISCV_REG_X1].x = pVM->cpum.s.u64ResetX1;
    /** @todo */
}



/*********************************************************************************************************************************
*   Saved State                                                                                                                  *
*********************************************************************************************************************************/

DECLCALLBACK(int) cpumR3LiveExecTarget(PVM pVM, PSSMHANDLE pSSM, uint32_t uPass)
{
    AssertReturn(uPass == 0, VERR_SSM_UNEXPECTED_PASS);
    cpumR3SaveCpuId(pVM, pSSM);
    return VINF_SSM_DONT_CALL_AGAIN;
}


DECLCALLBACK(int) cpumR3SaveExecTarget(PVM pVM, PSSMHANDLE pSSM)
{
    /*
     * Save.
     */
    SSMR3PutU32(pSSM, pVM->cCpus);
    for (VMCPUID idCpu = 0; idCpu < pVM->cCpus; idCpu++)
    {
        PVMCPU const   pVCpu   = pVM->apCpusR3[idCpu];
        PCPUMCTX const pGstCtx = &pVCpu->cpum.s.Guest;

        SSMR3PutStructEx(pSSM, pGstCtx, sizeof(*pGstCtx), 0, g_aCpumCtxFields, NULL);

        SSMR3PutU32(pSSM, pVCpu->cpum.s.fChanged);
    }

    cpumR3SaveCpuId(pVM, pSSM);
    return VINF_SUCCESS;
}


DECLCALLBACK(int) cpumR3LoadExecTarget(PVM pVM, PSSMHANDLE pSSM, uint32_t uVersion, uint32_t uPass)
{
    /*
     * Validate version.
     */
    AssertMsgReturn(uVersion == CPUM_SAVED_STATE_VERSION_RISCV,
                    ("cpumR3LoadExec: Invalid version uVersion=%d!\n", uVersion),
                    VERR_SSM_UNSUPPORTED_DATA_UNIT_VERSION);

    if (uPass == SSM_PASS_FINAL)
    {
        uint32_t cCpus;
        int rc = SSMR3GetU32(pSSM, &cCpus); AssertRCReturn(rc, rc);
        AssertLogRelMsgReturn(cCpus == pVM->cCpus, ("Mismatching CPU counts: saved: %u; configured: %u \n", cCpus, pVM->cCpus),
                              VERR_SSM_UNEXPECTED_DATA);

        /*
         * Do the per-CPU restoring.
         */
        for (VMCPUID idCpu = 0; idCpu < pVM->cCpus; idCpu++)
        {
            PVMCPU   pVCpu   = pVM->apCpusR3[idCpu];
            PCPUMCTX pGstCtx = &pVCpu->cpum.s.Guest;

            /*
             * Restore the CPUMCTX structure.
             */
            rc = SSMR3GetStructEx(pSSM, pGstCtx, sizeof(*pGstCtx), 0, g_aCpumCtxFields, NULL);
            AssertRCReturn(rc, rc);

            /*
             * Restore a couple of flags.
             */
            SSMR3GetU32(pSSM, &pVCpu->cpum.s.fChanged);
        }
    }

    pVM->cpum.s.fPendingRestore = false;

    return VINF_SUCCESS;
}


/**
 * @callback_method_impl{FNSSMINTLOADDONE}
 */
DECLHIDDEN(int) cpumR3LoadDoneTarget(PVM pVM, PSSMHANDLE pSSM)
{
    RT_NOREF(pVM, pSSM);
    /** @todo */
    return VINF_SUCCESS;
}


DECLHIDDEN(void) cpumR3InfoOneTarget(PVM pVM, PCVMCPU pVCpu, PCDBGFINFOHLP pHlp, CPUMDUMPTYPE enmType)
{
    PCCPUMCTX const pCtx = &pVCpu->cpum.s.Guest;
    RT_NOREF(pVM);

    /*
     * Format the registers.
     */
    switch (enmType)
    {
        case CPUMDUMPTYPE_TERSE:
            if (CPUMIsGuestIn64BitCodeEx(pCtx))
                pHlp->pfnPrintf(pHlp,
                    " x0=%016RX64  x1=%016RX64  x2=%016RX64  x3=%016RX64\n"
                    " x4=%016RX64  x5=%016RX64  x6=%016RX64  x7=%016RX64\n"
                    " x8=%016RX64  x9=%016RX64 x10=%016RX64 x11=%016RX64\n"
                    "x12=%016RX64 x13=%016RX64 x14=%016RX64 x15=%016RX64\n"
                    "x16=%016RX64 x17=%016RX64 x18=%016RX64 x19=%016RX64\n"
                    "x20=%016RX64 x21=%016RX64 x22=%016RX64 x23=%016RX64\n"
                    "x24=%016RX64 x25=%016RX64 x26=%016RX64 x27=%016RX64\n"
                    "x28=%016RX64 x29=%016RX64 x30=%016RX64\n"
                    " pc=%016RX64\n",
                    pCtx->aGRegs[0],  pCtx->aGRegs[1],  pCtx->aGRegs[2],  pCtx->aGRegs[3],
                    pCtx->aGRegs[4],  pCtx->aGRegs[5],  pCtx->aGRegs[6],  pCtx->aGRegs[7],
                    pCtx->aGRegs[8],  pCtx->aGRegs[9],  pCtx->aGRegs[10], pCtx->aGRegs[11],
                    pCtx->aGRegs[12], pCtx->aGRegs[13], pCtx->aGRegs[14], pCtx->aGRegs[15],
                    pCtx->aGRegs[16], pCtx->aGRegs[17], pCtx->aGRegs[18], pCtx->aGRegs[19],
                    pCtx->aGRegs[20], pCtx->aGRegs[21], pCtx->aGRegs[22], pCtx->aGRegs[23],
                    pCtx->aGRegs[24], pCtx->aGRegs[25], pCtx->aGRegs[26], pCtx->aGRegs[27],
                    pCtx->aGRegs[28], pCtx->aGRegs[29], pCtx->aGRegs[30],
                    pCtx->Pc.u64);
            else
                AssertFailed();
            break;

        case CPUMDUMPTYPE_DEFAULT:
            if (CPUMIsGuestIn64BitCodeEx(pCtx))
                pHlp->pfnPrintf(pHlp,
                    " x0=%016RX64  x1=%016RX64  x2=%016RX64  x3=%016RX64\n"
                    " x4=%016RX64  x5=%016RX64  x6=%016RX64  x7=%016RX64\n"
                    " x8=%016RX64  x9=%016RX64 x10=%016RX64 x11=%016RX64\n"
                    "x12=%016RX64 x13=%016RX64 x14=%016RX64 x15=%016RX64\n"
                    "x16=%016RX64 x17=%016RX64 x18=%016RX64 x19=%016RX64\n"
                    "x20=%016RX64 x21=%016RX64 x22=%016RX64 x23=%016RX64\n"
                    "x24=%016RX64 x25=%016RX64 x26=%016RX64 x27=%016RX64\n"
                    "x28=%016RX64 x29=%016RX64 x30=%016RX64\n"
                    " pc=%016RX64\n",
                    pCtx->aGRegs[0],  pCtx->aGRegs[1],  pCtx->aGRegs[2],  pCtx->aGRegs[3],
                    pCtx->aGRegs[4],  pCtx->aGRegs[5],  pCtx->aGRegs[6],  pCtx->aGRegs[7],
                    pCtx->aGRegs[8],  pCtx->aGRegs[9],  pCtx->aGRegs[10], pCtx->aGRegs[11],
                    pCtx->aGRegs[12], pCtx->aGRegs[13], pCtx->aGRegs[14], pCtx->aGRegs[15],
                    pCtx->aGRegs[16], pCtx->aGRegs[17], pCtx->aGRegs[18], pCtx->aGRegs[19],
                    pCtx->aGRegs[20], pCtx->aGRegs[21], pCtx->aGRegs[22], pCtx->aGRegs[23],
                    pCtx->aGRegs[24], pCtx->aGRegs[25], pCtx->aGRegs[26], pCtx->aGRegs[27],
                    pCtx->aGRegs[28], pCtx->aGRegs[29], pCtx->aGRegs[30],
                    pCtx->Pc.u64);
            else
                AssertFailed();
            break;

        case CPUMDUMPTYPE_VERBOSE:
            if (CPUMIsGuestIn64BitCodeEx(pCtx))
                pHlp->pfnPrintf(pHlp,
                    " x0=%016RX64  x1=%016RX64  x2=%016RX64  x3=%016RX64\n"
                    " x4=%016RX64  x5=%016RX64  x6=%016RX64  x7=%016RX64\n"
                    " x8=%016RX64  x9=%016RX64 x10=%016RX64 x11=%016RX64\n"
                    "x12=%016RX64 x13=%016RX64 x14=%016RX64 x15=%016RX64\n"
                    "x16=%016RX64 x17=%016RX64 x18=%016RX64 x19=%016RX64\n"
                    "x20=%016RX64 x21=%016RX64 x22=%016RX64 x23=%016RX64\n"
                    "x24=%016RX64 x25=%016RX64 x26=%016RX64 x27=%016RX64\n"
                    "x28=%016RX64 x29=%016RX64 x30=%016RX64\n"
                    " pc=%016RX64\n",
                    pCtx->aGRegs[0],  pCtx->aGRegs[1],  pCtx->aGRegs[2],  pCtx->aGRegs[3],
                    pCtx->aGRegs[4],  pCtx->aGRegs[5],  pCtx->aGRegs[6],  pCtx->aGRegs[7],
                    pCtx->aGRegs[8],  pCtx->aGRegs[9],  pCtx->aGRegs[10], pCtx->aGRegs[11],
                    pCtx->aGRegs[12], pCtx->aGRegs[13], pCtx->aGRegs[14], pCtx->aGRegs[15],
                    pCtx->aGRegs[16], pCtx->aGRegs[17], pCtx->aGRegs[18], pCtx->aGRegs[19],
                    pCtx->aGRegs[20], pCtx->aGRegs[21], pCtx->aGRegs[22], pCtx->aGRegs[23],
                    pCtx->aGRegs[24], pCtx->aGRegs[25], pCtx->aGRegs[26], pCtx->aGRegs[27],
                    pCtx->aGRegs[28], pCtx->aGRegs[29], pCtx->aGRegs[30],
                    pCtx->Pc.u64);
            else
                AssertFailed();

            /** @todo */
            break;
    }
}


DECLHIDDEN(void) cpumR3LogCpuIdAndMsrFeaturesTarget(PVM pVM)
{
    LogRel(("******************** CPU feature dump ***********************\n"));
    DBGFR3Info(pVM->pUVM, "cpufeat", "verbose", DBGFR3InfoLogRelHlp());
    LogRel(("\n"));
    DBGFR3_INFO_LOG_SAFE(pVM, "cpufeat", "verbose"); /* macro */
    LogRel(("***************** End of CPU feature dump *******************\n"));
}

