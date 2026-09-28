/* $Id$ */
/** @file
 * AIA - Advanced Interrupt Architecture (AIA) Controller.
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

#ifndef VMM_INCLUDED_SRC_include_AIAInternal_h
#define VMM_INCLUDED_SRC_include_AIAInternal_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/pdmaia.h>
#include <VBox/vmm/stam.h>

/** @defgroup grp_aia_int       Internal
 * @ingroup grp_aia
 * @internal
 * @{
 */

#ifdef VBOX_INCLUDED_vmm_pdmaia_h
# if defined(RT_OS_LINUX)
/** The KVM AIA backend. */
extern const PDMAIABACKEND g_AiaKvmBackend;
# endif
#endif

#define VMCPU_TO_AIACPU(a_pVCpu)            (&(a_pVCpu)->aia.s)
#define VM_TO_AIA(a_pVM)                    (&(a_pVM)->aia.s)
#ifdef IN_RING3
# define VMCPU_TO_DEVINS(a_pVCpu)           ((a_pVCpu)->pVMR3->aia.s.pDevInsR3)
#elif defined(IN_RING0)
# error "Not implemented!"
#endif

/**
 * AIA VM Instance data.
 */
typedef struct AIA
{
    /** The ring-3 device instance. */
    PPDMDEVINSR3                pDevInsR3;
} AIA;
/** Pointer to AIA VM instance data. */
typedef AIA *PAIA;
/** Pointer to const AIA VM instance data. */
typedef AIA const *PCAIA;
AssertCompileSizeAlignment(AIA, 8);


/**
 * AIA VMCPU Instance data.
 */
typedef struct AIACPU
{
    /** @todo VBox interface. */
} AIACPU;
/** Pointer to AIA VMCPU instance data. */
typedef AIACPU *PAIACPU;
/** Pointer to a const AIA VMCPU instance data. */
typedef AIACPU const *PCAIACPU;

/** @} */

#endif /* !VMM_INCLUDED_SRC_include_AIAInternal_h */

