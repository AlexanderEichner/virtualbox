/* $Id$ */
/** @file
 * CPUM - CPU ID part for RISC-V hypervisor.
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
#define LOG_GROUP LOG_GROUP_CPUM
#define CPUM_WITH_NONCONST_HOST_FEATURES /* required for initializing parts of the g_CpumHostFeatures structure here. */
#include <VBox/vmm/cpum.h>
#include <VBox/vmm/dbgf.h>
#include <VBox/vmm/nem.h>
#include <VBox/vmm/ssm.h>
#include "CPUMInternal-armv8.h"
#include <VBox/vmm/vmcc.h>
#include <VBox/gic.h>

#include <iprt/ctype.h>
#include <iprt/mem.h>
#include <iprt/string.h>



/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/
#ifdef VBOX_VMM_TARGET_RISCV
/**
 * Display the guest CPU features.
 *
 * @param   pVM         The cross context VM structure.
 * @param   pHlp        The info helper functions.
 * @param   pszArgs     "default" or "verbose".
 */
DECLCALLBACK(void) cpumR3CpuFeatInfo(PVM pVM, PCDBGFINFOHLP pHlp, const char *pszArgs)
{
    /*
     * Parse the argument.
     */
    bool fVerbose = false;
    if (pszArgs)
    {
        pszArgs = RTStrStripL(pszArgs);
        if (!strcmp(pszArgs, "verbose"))
            fVerbose = true;
    }

    /*
     * Call generated code to do the actual printing.
     */
    /** @todo */
# if 0
# ifdef RT_ARCH_RISCV
    if (fVerbose)
        CPUMR3CpuIdInfoRiscv(pHlp, 80, &pVM->cpum.s.GuestFeatures, "guest", &pVM->cpum.s.HostFeatures.s, "host");
    else
# endif
        CPUMR3CpuIdInfoRiscv(pHlp, 80, &pVM->cpum.s.GuestFeatures, "guest", NULL, NULL);
# else
    RT_NOREF(pVM, pHlp, fVerbose);
# endif
}
#endif /* VBOX_VMM_TARGET_ARMV8 */
