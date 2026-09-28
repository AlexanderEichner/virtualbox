/* $Id$ */
/** @file
 * PLIC - Platform Level Interrupt Controller (PLIC) - VBox PLIC emulation.
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
#define LOG_GROUP LOG_GROUP_DEV_GIC
#include <VBox/log.h>
#include "PLICInternal.h"
#include <VBox/vmm/pdmaia.h>
#include <VBox/vmm/cpum.h>
#include <VBox/vmm/hm.h>
#include <VBox/vmm/mm.h>
#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/ssm.h>
#include <VBox/vmm/vm.h>

#ifndef VBOX_DEVICE_STRUCT_TESTCASE


/*********************************************************************************************************************************
*   Defined Constants And Macros                                                                                                 *
*********************************************************************************************************************************/

/** The PLIC MMIO region size in bytes. */
#define PLIC_MMIO_SIZE _64M


/*********************************************************************************************************************************
*   Structures and Typedefs                                                                                                      *
*********************************************************************************************************************************/


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/

/**
 * @callback_method_impl{FNIOMMMIONEWREAD}
 */
DECLCALLBACK(VBOXSTRICTRC) plicMmioRead(PPDMDEVINS pDevIns, void *pvUser, RTGCPHYS off, void *pv, unsigned cb)
{
    NOREF(pvUser);
    Assert(!(off & 0x3));
    Assert(cb == 4); RT_NOREF_PV(cb);

    PVMCPUCC pVCpu    = PDMDevHlpGetVMCPU(pDevIns);
    uint32_t uValue   = 0;

    *(uint32_t *)pv = uValue;

    Log2(("PLIC%u: plicReadMmio: offReg=%#RX64 uValue=%#RX32\n", pVCpu->idCpu, off, uValue));
    return VINF_SUCCESS;
}


/**
 * @callback_method_impl{FNIOMMMIONEWWRITE}
 */
DECLCALLBACK(VBOXSTRICTRC) plicMmioWrite(PPDMDEVINS pDevIns, void *pvUser, RTGCPHYS off, void const *pv, unsigned cb)
{
    NOREF(pvUser);
    Assert(!(off & 0x3));
    Assert(cb == 4); RT_NOREF_PV(cb);

    PVMCPUCC pVCpu    = PDMDevHlpGetVMCPU(pDevIns);
    uint32_t uValue   = *(uint32_t *)pv;

    Log2(("PLIC%u: plicWriteMmio: offReg=%#RX64 uValue=%#RX32\n", pVCpu->idCpu, off, uValue));

    return VINF_SUCCESS;
}


/**
 * Sets the given external interrupt inside the in-kernel KVM AIA.
 *
 * @returns VBox status code.
 * @param   pVM         The VM instance.
 * @param   uIntId      The SPI ID to update.
 * @param   fAsserted   Flag whether the interrupt is asserted (true) or not (false).
 */
static DECLCALLBACK(int) plicR3SetExternal(PVMCC pVM, uint32_t u32Irq, bool fAsserted)
{
    PPLIC pPlic = VM_TO_PLIC(pVM);
    PPDMDEVINS pDevIns = pPlic->CTX_SUFF(pDevIns);

    RT_NOREF(pDevIns, u32Irq, fAsserted);
    return VINF_SUCCESS;
}


/**
 * Sets the given HART interrupt for the PLIC.
 *
 * @returns VBox status code.
 * @param   pVCpu       The vCPU for whih the PPI state is updated.
 * @param   uIntId      The PPI ID to update.
 * @param   fAsserted   Flag whether the interrupt is asserted (true) or not (false).
 */
static DECLCALLBACK(int) plicR3SetHart(PVMCPUCC pVCpu, uint32_t u32Irq, bool fAsserted)
{
    RT_NOREF(pVCpu, u32Irq, fAsserted);
    AssertFailedReturn(VERR_NOT_SUPPORTED);
}


/**
 * @interface_method_impl{PDMDEVREG,pfnReset}
 */
DECLCALLBACK(void) plicR3Reset(PPDMDEVINS pDevIns)
{
    PVM pVM = PDMDevHlpGetVM(pDevIns);
    VM_ASSERT_EMT0(pVM);
    VM_ASSERT_IS_NOT_RUNNING(pVM);

    RT_NOREF(pVM);

    LogFlow(("PLIC: plicR3Reset\n"));
}


/**
 * @interface_method_impl{PDMDEVREG,pfnDestruct}
 */
DECLCALLBACK(int) plicR3Destruct(PPDMDEVINS pDevIns)
{
    LogFlowFunc(("pDevIns=%p\n", pDevIns));
    PDMDEV_CHECK_VERSIONS_RETURN_QUIET(pDevIns);

    return VINF_SUCCESS;
}


/**
 * @interface_method_impl{PDMDEVREG,pfnConstruct}
 */
DECLCALLBACK(int) plicR3Construct(PPDMDEVINS pDevIns, int iInstance, PCFGMNODE pCfg)
{
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);
    PPLICDEV        pThis    = PDMDEVINS_2_DATA(pDevIns, PPLICDEV);
    PCPDMDEVHLPR3   pHlp     = pDevIns->pHlpR3;
    PVM             pVM      = PDMDevHlpGetVM(pDevIns);
    PPLIC           pPlic    = VM_TO_PLIC(pVM);
    Assert(iInstance == 0); NOREF(iInstance);

    /*
     * Init the data.
     */
    pPlic->pDevInsR3 = pDevIns;

    /*
     * Validate AIA settings.
     */
    PDMDEV_VALIDATE_CONFIG_RETURN(pDevIns, "PlicMmioBase", "");

    /*
     * Disable automatic PDM locking for this device.
     */
    int rc = PDMDevHlpSetDeviceCritSect(pDevIns, PDMDevHlpCritSectGetNop(pDevIns));
    AssertRCReturn(rc, rc);

    /*
     * Register the PLIC with PDM.
     */
    rc = PDMDevHlpIcRegister(pDevIns);
    AssertLogRelRCReturn(rc, rc);

    rc = PDMAiaRegisterBackend(pVM, PDMAIABACKENDTYPE_VBOX_PLIC, &g_AiaPlicBackend);
    AssertLogRelRCReturn(rc, rc);

    /*
     * Query the MMIO ranges.
     */
    RTGCPHYS GCPhysMmioBasePlic = 0;
    rc = pHlp->pfnCFGMQueryU64(pCfg, "PlicMmioBase", &GCPhysMmioBasePlic);
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, rc,
                                N_("Configuration error: Failed to get the \"PlicMmioBase\" value"));

    rc = PDMDevHlpMmioCreateAndMap(pDevIns, GCPhysMmioBasePlic, PLIC_MMIO_SIZE, plicMmioWrite, plicMmioRead,
                                   IOMMMIO_FLAGS_READ_DWORD | IOMMMIO_FLAGS_WRITE_DWORD_ZEROED, "PLIC", &pThis->hMmio);
    AssertRCReturn(rc, rc);


    return VINF_SUCCESS;
}


/**
 * AIA device registration structure.
 */
const PDMDEVREG g_DevicePLIC =
{
    /* .u32Version = */             PDM_DEVREG_VERSION,
    /* .uReserved0 = */             0,
    /* .szName = */                 "plic",
    /* .fFlags = */                 PDM_DEVREG_FLAGS_DEFAULT_BITS | PDM_DEVREG_FLAGS_NEW_STYLE,
    /* .fClass = */                 PDM_DEVREG_CLASS_PIC,
    /* .cMaxInstances = */          1,
    /* .uSharedVersion = */         42,
    /* .cbInstanceShared = */       sizeof(PLIC),
    /* .cbInstanceCC = */           0,
    /* .cbInstanceRC = */           0,
    /* .cMaxPciDevices = */         0,
    /* .cMaxMsixVectors = */        0,
    /* .pszDescription = */         "Platform Level Interrupt Controller (PLIC)",
#if defined(IN_RING3)
    /* .szRCMod = */                "VMMRC.rc",
    /* .szR0Mod = */                "VMMR0.r0",
    /* .pfnConstruct = */           plicR3Construct,
    /* .pfnDestruct = */            plicR3Destruct,
    /* .pfnRelocate = */            NULL,
    /* .pfnMemSetup = */            NULL,
    /* .pfnPowerOn = */             NULL,
    /* .pfnReset = */               plicR3Reset,
    /* .pfnSuspend = */             NULL,
    /* .pfnResume = */              NULL,
    /* .pfnAttach = */              NULL,
    /* .pfnDetach = */              NULL,
    /* .pfnQueryInterface = */      NULL,
    /* .pfnInitComplete = */        NULL,
    /* .pfnPowerOff = */            NULL,
    /* .pfnSoftReset = */           NULL,
    /* .pfnReserved0 = */           NULL,
    /* .pfnReserved1 = */           NULL,
    /* .pfnReserved2 = */           NULL,
    /* .pfnReserved3 = */           NULL,
    /* .pfnReserved4 = */           NULL,
    /* .pfnReserved5 = */           NULL,
    /* .pfnReserved6 = */           NULL,
    /* .pfnReserved7 = */           NULL,
#else
# error "Not in IN_RING3!"
#endif
    /* .u32VersionEnd = */          PDM_DEVREG_VERSION
};


/**
 * The PLIC AIA backend.
 */
const PDMAIABACKEND g_AiaPlicBackend =
{
    /* .pfnReadSysReg = */  NULL,
    /* .pfnWriteSysReg = */ NULL,
    /* .pfnSetExternal = */ plicR3SetExternal,
    /* .pfnSetHart = */     plicR3SetHart,
    /* .pfnSendMsi = */     NULL,
};

#endif /* !VBOX_DEVICE_STRUCT_TESTCASE */

