/* $Id$ */
/** @file
 * PDM - Advanced Interrupt Architecture (AIA) Interface.
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
#define LOG_GROUP LOG_GROUP_PDM_GIC
#include "PDMInternal.h"
#include <VBox/vmm/vm.h>
#include <VBox/vmm/gvm.h>


/**
 * Gets the PDM AIA backend name.
 *
 * @returns The backend name.
 * @param   enmBackendType      The PDM AIA backend type.
 */
VMM_INT_DECL(const char *) PDMAiaGetBackendName(PDMAIABACKENDTYPE enmBackendType)
{
    switch (enmBackendType)
    {
        case PDMAIABACKENDTYPE_NONE:        return "None";
        case PDMAIABACKENDTYPE_VBOX:        return "VirtualBox";
        case PDMAIABACKENDTYPE_VBOX_PLIC:   return "VirtualBox PLIC";
        case PDMAIABACKENDTYPE_KVM:         return "KVM";
        default:
            break;
    }
    AssertFailed();
    return "Invalid";
}


/**
 * Reads a AIA CSR register.
 *
 * @returns Strict VBox status code.
 * @param   pVCpu           The cross context virtual CPU structure.
 * @param   u32Csr          The CSR being read (IPRT system register identifier).
 * @param   pu64Value       Where to store the read value.
 */
VMM_INT_DECL(VBOXSTRICTRC) PDMAiaReadCsr(PVMCPUCC pVCpu, uint32_t u32Csr, uint64_t *pu64Value)
{
    AssertReturn(PDMCPU_TO_AIABACKEND(pVCpu)->pfnReadSysReg, VERR_INVALID_POINTER);
    return PDMCPU_TO_AIABACKEND(pVCpu)->pfnReadSysReg(pVCpu, u32Csr, pu64Value);
}


/**
 * Writes an AIA CSR register.
 *
 * @returns Strict VBox status code.
 * @param   pVCpu           The cross context virtual CPU structure.
 * @param   u32Csr          The CSR being written (IPRT system register identifier).
 * @param   u64Value        The value to write.
 */
VMM_INT_DECL(VBOXSTRICTRC) PDMAiaWriteCsr(PVMCPUCC pVCpu, uint32_t u32Csr, uint64_t u64Value)
{
    AssertReturn(PDMCPU_TO_AIABACKEND(pVCpu)->pfnWriteSysReg, VERR_INVALID_POINTER);
    return PDMCPU_TO_AIABACKEND(pVCpu)->pfnWriteSysReg(pVCpu, u32Csr, u64Value);
}


/**
 * Sets the specified external interrupt.
 *
 * @returns VBox status code.
 * @param   pVM         The cross context virtual machine structure.
 * @param   u32Irq      The interrupt ID to assert/de-assert.
 * @param   fAsserted   Flag whether to mark the interrupt as asserted/de-asserted.
 */
VMM_INT_DECL(int) PDMAiaSetExternal(PVMCC pVM, uint32_t u32Irq, bool fAsserted)
{
    AssertReturn(PDMCPU_TO_AIABACKEND(pVM)->pfnSetExternal, VERR_INVALID_POINTER);
    return PDMCPU_TO_AIABACKEND(pVM)->pfnSetExternal(pVM, u32Irq, fAsserted);
}


/**
 * Sets the specified per HART interrupt.
 *
 * @returns VBox status code.
 * @param   pVCpu       The cross context virtual CPU structure.
 * @param   u32Irq      The major interrupt class to assert/de-assert.
 * @param   fAsserted   Flag whether to mark the interrupt as asserted/de-asserted.
 */
VMM_INT_DECL(int) PDMAiaSetHart(PVMCPUCC pVCpu, uint32_t u32Irq, bool fAsserted)
{
    AssertReturn(PDMCPU_TO_AIABACKEND(pVCpu)->pfnSetHart, VERR_INVALID_POINTER);
    return PDMCPU_TO_AIABACKEND(pVCpu)->pfnSetHart(pVCpu, u32Irq, fAsserted);
}


/**
 * Registers a PDM AIA backend.
 *
 * @returns VBox status code.
 * @param   pVM             The cross context VM structure.
 * @param   enmBackendType  The PDM AIA backend type.
 * @param   pBackend        The PDM AIA backend.
 */
VMM_INT_DECL(int) PDMAiaRegisterBackend(PVMCC pVM, PDMAIABACKENDTYPE enmBackendType, PCPDMAIABACKEND pBackend)
{
    /*
     * Validate.
     */
    AssertPtrReturn(pVM, VERR_INVALID_PARAMETER);
    AssertPtrReturn(pBackend, VERR_INVALID_PARAMETER);
    AssertReturn(   enmBackendType > PDMAIABACKENDTYPE_NONE
                 && enmBackendType < PDMAIABACKENDTYPE_END, VERR_INVALID_PARAMETER);

    AssertPtrReturn(pBackend->pfnSetExternal,   VERR_INVALID_POINTER);
    AssertPtrReturn(pBackend->pfnSetHart,       VERR_INVALID_POINTER);

    /*
     * Register the backend.
     */
    pVM->pdm.s.Ic.u.riscv.enmKind = enmBackendType;
#ifdef IN_RING3
    pVM->pdm.s.Ic.u.riscv.AiaBackend = *pBackend;
#else
# error "No AIA ring-0 support in this host target"
#endif

#ifdef IN_RING3
    LogRel(("PDM: %s AIA backend registered\n", PDMAiaGetBackendName(enmBackendType)));
#endif
    return VINF_SUCCESS;
}

