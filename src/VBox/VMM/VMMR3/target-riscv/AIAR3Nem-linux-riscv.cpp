/* $Id$ */
/** @file
 * AIA - Advanced Interrupt Architecture (AIA) controller - KVM in kernel interface.
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
#include "NEMInternal.h" /* Need access to the VM file descriptor. */
#include "AIAInternal.h"
#include <VBox/vmm/pdmaia.h>
#include <VBox/vmm/cpum.h>
#include <VBox/vmm/hm.h>
#include <VBox/vmm/mm.h>
#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/ssm.h>
#include <VBox/vmm/vm.h>

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <linux/kvm.h>


#ifndef VBOX_DEVICE_STRUCT_TESTCASE


/*********************************************************************************************************************************
*   Defined Constants And Macros                                                                                                 *
*********************************************************************************************************************************/


/*********************************************************************************************************************************
*   Structures and Typedefs                                                                                                      *
*********************************************************************************************************************************/

/**
 * AIAKvm PDM instance data (per-VM).
 */
typedef struct AIAKVMDEV
{
    /** Pointer to the PDM device instance. */
    PPDMDEVINSR3        pDevIns;
    /** The AIA device file descriptor. */
    int                 fdAia;
    /** The VM file descriptor (for KVM_IRQ_LINE). */
    int                 fdKvmVm;
} AIAKVMDEV;
/** Pointer to a AIA KVM device. */
typedef AIAKVMDEV *PAIAKVMDEV;
/** Pointer to a const AIA KVM device. */
typedef AIAKVMDEV const *PCAIAKVMDEV;


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/

/**
 * Sets the given external interrupt inside the in-kernel KVM AIA.
 *
 * @returns VBox status code.
 * @param   pVM         The VM instance.
 * @param   uIntId      The SPI ID to update.
 * @param   fAsserted   Flag whether the interrupt is asserted (true) or not (false).
 */
static DECLCALLBACK(int) aiaR3KvmSetExternal(PVMCC pVM, uint32_t u32Irq, bool fAsserted)
{
    PAIA pAia = VM_TO_AIA(pVM);
    PPDMDEVINS pDevIns = pAia->CTX_SUFF(pDevIns);

#if 0
    /* idCpu is ignored for SPI interrupts. */
    return gicR3KvmSetIrq(pDevIns, 0 /*idCpu*/, KVM_ARM_IRQ_TYPE_SPI,
                          uIntId + GIC_INTID_RANGE_SPI_START, fAsserted);
#else
    RT_NOREF(pDevIns, u32Irq, fAsserted);
    return VINF_SUCCESS;
#endif
}


/**
 * Sets the given HART interrupt inside the in-kernel KVM AIA.
 *
 * @returns VBox status code.
 * @param   pVCpu       The vCPU for whih the PPI state is updated.
 * @param   uIntId      The PPI ID to update.
 * @param   fAsserted   Flag whether the interrupt is asserted (true) or not (false).
 */
static DECLCALLBACK(int) aiaR3KvmSetHart(PVMCPUCC pVCpu, uint32_t u32Irq, bool fAsserted)
{
    PPDMDEVINS pDevIns = VMCPU_TO_DEVINS(pVCpu);

#if 0
    return gicR3KvmSetIrq(pDevIns, pVCpu->idCpu, KVM_ARM_IRQ_TYPE_PPI,
                          uIntId + GIC_INTID_RANGE_PPI_START, fAsserted);
#else
    RT_NOREF(pDevIns, u32Irq, fAsserted);
    return VINF_SUCCESS;
#endif
}

#if 0
/**
 * Sets the given device attribute in KVM to the given value.
 *
 * @returns VBox status code.
 * @param   pThis           The KVM GIC device instance.
 * @param   u32Grp          The device attribute group being set.
 * @param   u32Attr         The actual attribute inside the group being set.
 * @param   pvAttrVal       Where the attribute value to set.
 * @param   pszAttribute    Attribute description for logging.
 */
static int gicR3KvmSetDevAttribute(PGICKVMDEV pThis, uint32_t u32Grp, uint32_t u32Attr, const void *pvAttrVal, const char *pszAttribute)
{
    struct kvm_device_attr DevAttr;

    DevAttr.flags = 0;
    DevAttr.group = u32Grp;
    DevAttr.attr  = u32Attr;
    DevAttr.addr  = (uintptr_t)pvAttrVal;
    int rcLnx = ioctl(pThis->fdGic, KVM_HAS_DEVICE_ATTR, &DevAttr);
    if (rcLnx < 0)
        return PDMDevHlpVMSetError(pThis->pDevIns, RTErrConvertFromErrno(errno), RT_SRC_POS,
                                   N_("KVM error: The in-kernel VGICv3 device doesn't support setting the attribute \"%s\" (%d)"),
                                   pszAttribute, errno);

    rcLnx = ioctl(pThis->fdGic, KVM_SET_DEVICE_ATTR, &DevAttr);
    if (rcLnx < 0)
        return PDMDevHlpVMSetError(pThis->pDevIns, RTErrConvertFromErrno(errno), RT_SRC_POS,
                                   N_("KVM error: Setting the attribute \"%s\" for the in-kernel GICv3 failed (%d)"),
                                   pszAttribute, errno);

    return VINF_SUCCESS;
}


/**
 * Queries the value of the given device attribute from KVM.
 *
 * @returns VBox status code.
 * @param   pThis           The KVM GIC device instance.
 * @param   u32Grp          The device attribute group being queried.
 * @param   u32Attr         The actual attribute inside the group being queried.
 * @param   pvAttrVal       Where the attribute value should be stored upon success.
 * @param   pszAttribute    Attribute description for logging.
 */
static int gicR3KvmQueryDevAttribute(PGICKVMDEV pThis, uint32_t u32Grp, uint32_t u32Attr, void *pvAttrVal, const char *pszAttribute)
{
    struct kvm_device_attr DevAttr;

    DevAttr.flags = 0;
    DevAttr.group = u32Grp;
    DevAttr.attr  = u32Attr;
    DevAttr.addr  = (uintptr_t)pvAttrVal;
    int rcLnx = ioctl(pThis->fdGic, KVM_GET_DEVICE_ATTR, &DevAttr);
    if (rcLnx < 0)
        return PDMDevHlpVMSetError(pThis->pDevIns, RTErrConvertFromErrno(errno), RT_SRC_POS,
                                   N_("KVM error: Failed to query attribute \"%s\" from the in-kernel VGICv3 (%d)"),
                                   pszAttribute, errno);

    return VINF_SUCCESS;
}
#endif

/**
 * @interface_method_impl{PDMDEVREG,pfnReset}
 */
DECLCALLBACK(void) aiaR3KvmReset(PPDMDEVINS pDevIns)
{
    PVM pVM = PDMDevHlpGetVM(pDevIns);
    VM_ASSERT_EMT0(pVM);
    VM_ASSERT_IS_NOT_RUNNING(pVM);

    RT_NOREF(pVM);

    LogFlow(("AIA: aiaR3KvmReset\n"));
}


/**
 * @interface_method_impl{PDMDEVREG,pfnDestruct}
 */
DECLCALLBACK(int) aiaR3KvmDestruct(PPDMDEVINS pDevIns)
{
    LogFlowFunc(("pDevIns=%p\n", pDevIns));
    PDMDEV_CHECK_VERSIONS_RETURN_QUIET(pDevIns);

    PAIAKVMDEV pThis = PDMDEVINS_2_DATA(pDevIns, PAIAKVMDEV);

    close(pThis->fdAia);
    pThis->fdAia = 0;

    return VINF_SUCCESS;
}


/**
 * @interface_method_impl{PDMDEVREG,pfnConstruct}
 */
DECLCALLBACK(int) aiaR3KvmConstruct(PPDMDEVINS pDevIns, int iInstance, PCFGMNODE pCfg)
{
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);
    PAIAKVMDEV      pThis    = PDMDEVINS_2_DATA(pDevIns, PAIAKVMDEV);
    PCPDMDEVHLPR3   pHlp     = pDevIns->pHlpR3;
    PVM             pVM      = PDMDevHlpGetVM(pDevIns);
    PAIA            pAia     = VM_TO_AIA(pVM);
    Assert(iInstance == 0); NOREF(iInstance);

    /*
     * Init the data.
     */
    pAia->pDevInsR3 = pDevIns;
    pThis->pDevIns  = pDevIns;
    pThis->fdKvmVm  = pVM->nem.s.fdVm;

    /*
     * Validate AIA settings.
     */
    PDMDEV_VALIDATE_CONFIG_RETURN(pDevIns, "AplicMmioBase"
                                            "|ImsicMmioBase"
                                         , "");

    /*
     * Disable automatic PDM locking for this device.
     */
    int rc = PDMDevHlpSetDeviceCritSect(pDevIns, PDMDevHlpCritSectGetNop(pDevIns));
    AssertRCReturn(rc, rc);

    /*
     * Register the AIA with PDM.
     */
    rc = PDMDevHlpIcRegister(pDevIns);
    AssertLogRelRCReturn(rc, rc);

    rc = PDMAiaRegisterBackend(pVM, PDMAIABACKENDTYPE_KVM, &g_AiaKvmBackend);
    AssertLogRelRCReturn(rc, rc);

    /*
     * Query the MMIO ranges.
     */
    RTGCPHYS GCPhysMmioBaseAplic = 0;
    rc = pHlp->pfnCFGMQueryU64(pCfg, "AplicMmioBase", &GCPhysMmioBaseAplic);
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, rc,
                                N_("Configuration error: Failed to get the \"AplicMmioBase\" value"));

    RTGCPHYS GCPhysMmioBaseImsic = 0;
    rc = pHlp->pfnCFGMQueryU64(pCfg, "ImsicMmioBase", &GCPhysMmioBaseImsic);
    if (RT_FAILURE(rc))
        return PDMDEV_SET_ERROR(pDevIns, rc,
                                N_("Configuration error: Failed to get the \"ImsicMmioBase\" value"));

    /*
     * Create the device.
     */
    struct kvm_create_device DevCreate;
    DevCreate.type  = KVM_DEV_TYPE_RISCV_AIA;
    DevCreate.fd    = 0;
    DevCreate.flags = 0;
    int rcLnx = ioctl(pThis->fdKvmVm, KVM_CREATE_DEVICE, &DevCreate);
    if (rcLnx < 0)
        return PDMDevHlpVMSetError(pDevIns, RTErrConvertFromErrno(errno), RT_SRC_POS,
                                   N_("KVM error: Creating the in-kernel AIA device failed (%d)"), errno);

    pThis->fdAia = DevCreate.fd;

#if 0
    /*
     * Set the distributor and re-distributor base.
     */
    rc = gicR3KvmSetDevAttribute(pThis, KVM_DEV_ARM_VGIC_GRP_ADDR, KVM_VGIC_V3_ADDR_TYPE_DIST, &GCPhysMmioBaseDist,
                                 "Distributor MMIO base");
    AssertRCReturn(rc, rc);

    rc = gicR3KvmSetDevAttribute(pThis, KVM_DEV_ARM_VGIC_GRP_ADDR, KVM_VGIC_V3_ADDR_TYPE_REDIST, &GCPhysMmioBaseReDist,
                                 "Re-Distributor MMIO base");
    AssertRCReturn(rc, rc);

    /* Query and log the number of IRQ lines this GIC supports. */
    uint32_t cIrqs = 0;
    rc = gicR3KvmQueryDevAttribute(pThis, KVM_DEV_ARM_VGIC_GRP_NR_IRQS, 0, &cIrqs,
                                   "IRQ line count");
    AssertRCReturn(rc, rc);
    LogRel(("GICR3Kvm: Supports %u IRQs\n", cIrqs));

    /*
     * Init the controller.
     */
    rc = gicR3KvmSetDevAttribute(pThis, KVM_DEV_ARM_VGIC_GRP_CTRL, KVM_DEV_ARM_VGIC_CTRL_INIT, NULL,
                                 "VGIC init");
    AssertRCReturn(rc, rc);

    gicR3KvmReset(pDevIns);
#endif
    return VINF_SUCCESS;
}


/**
 * AIA device registration structure.
 */
const PDMDEVREG g_DeviceAIANem =
{
    /* .u32Version = */             PDM_DEVREG_VERSION,
    /* .uReserved0 = */             0,
    /* .szName = */                 "aia-nem",
    /* .fFlags = */                 PDM_DEVREG_FLAGS_DEFAULT_BITS | PDM_DEVREG_FLAGS_NEW_STYLE,
    /* .fClass = */                 PDM_DEVREG_CLASS_PIC,
    /* .cMaxInstances = */          1,
    /* .uSharedVersion = */         42,
    /* .cbInstanceShared = */       sizeof(AIAKVMDEV),
    /* .cbInstanceCC = */           0,
    /* .cbInstanceRC = */           0,
    /* .cMaxPciDevices = */         0,
    /* .cMaxMsixVectors = */        0,
    /* .pszDescription = */         "Advanced Interrupt Architecture Controller - NEM/KVM",
#if defined(IN_RING3)
    /* .szRCMod = */                "VMMRC.rc",
    /* .szR0Mod = */                "VMMR0.r0",
    /* .pfnConstruct = */           aiaR3KvmConstruct,
    /* .pfnDestruct = */            aiaR3KvmDestruct,
    /* .pfnRelocate = */            NULL,
    /* .pfnMemSetup = */            NULL,
    /* .pfnPowerOn = */             NULL,
    /* .pfnReset = */               aiaR3KvmReset,
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
 * The KVM AIA backend.
 */
const PDMAIABACKEND g_AiaKvmBackend =
{
    /* .pfnReadSysReg = */  NULL,
    /* .pfnWriteSysReg = */ NULL,
    /* .pfnSetExternal = */ aiaR3KvmSetExternal,
    /* .pfnSetHart = */     aiaR3KvmSetHart,
    /* .pfnSendMsi = */     NULL,
};

#endif /* !VBOX_DEVICE_STRUCT_TESTCASE */

