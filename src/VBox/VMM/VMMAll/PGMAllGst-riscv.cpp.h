/* $Id$ */
/** @file
 * PGM - Page Manager, RISC-V Guest Paging Template - All context code.
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


/*
 * Common helpers.
 * Common helpers.
 * Common helpers.
 */

DECLINLINE(int) pgmGstWalkReturnNotPresent(PVMCPUCC pVCpu, PPGMPTWALK pWalk, uint8_t uLevel)
{
    NOREF(pVCpu);
    pWalk->fSucceeded      = false;
    pWalk->fNotPresent     = true;
    pWalk->uLevel          = uLevel;
    pWalk->fFailed         = PGM_WALKFAIL_NOT_PRESENT
                           | ((uint32_t)uLevel << PGM_WALKFAIL_LEVEL_SHIFT);
    return VERR_PAGE_TABLE_NOT_PRESENT;
}

DECLINLINE(int) pgmGstWalkReturnBadPhysAddr(PVMCPUCC pVCpu, PPGMPTWALK pWalk, uint8_t uLevel, int rc)
{
    AssertMsg(rc == VERR_PGM_INVALID_GC_PHYSICAL_ADDRESS, ("%Rrc\n", rc)); NOREF(rc); NOREF(pVCpu);
    pWalk->fSucceeded      = false;
    pWalk->fBadPhysAddr    = true;
    pWalk->uLevel          = uLevel;
    pWalk->fFailed         = PGM_WALKFAIL_BAD_PHYSICAL_ADDRESS
                           | ((uint32_t)uLevel << PGM_WALKFAIL_LEVEL_SHIFT);
    return VERR_PAGE_TABLE_NOT_PRESENT;
}


DECLINLINE(int) pgmGstWalkReturnRsvdError(PVMCPUCC pVCpu, PPGMPTWALK pWalk, uint8_t uLevel)
{
    NOREF(pVCpu);
    pWalk->fSucceeded      = false;
    pWalk->fRsvdError      = true;
    pWalk->uLevel          = uLevel;
    pWalk->fFailed         = PGM_WALKFAIL_RESERVED_BITS
                           | ((uint32_t)uLevel << PGM_WALKFAIL_LEVEL_SHIFT);
    return VERR_PAGE_TABLE_NOT_PRESENT;
}


DECLINLINE(int) pgmGstWalkFastReturnNotPresent(PVMCPUCC pVCpu, PPGMPTWALKFAST pWalk, uint8_t uLevel)
{
    RT_NOREF(pVCpu);
    pWalk->fFailed = PGM_WALKFAIL_NOT_PRESENT           | ((uint32_t)uLevel << PGM_WALKFAIL_LEVEL_SHIFT);
    return VERR_PAGE_TABLE_NOT_PRESENT;
}


DECLINLINE(int) pgmGstWalkFastReturnBadPhysAddr(PVMCPUCC pVCpu, PPGMPTWALKFAST pWalk, uint8_t uLevel, int rc)
{
    AssertMsg(rc == VERR_PGM_INVALID_GC_PHYSICAL_ADDRESS, ("%Rrc\n", rc)); RT_NOREF(pVCpu, rc);
    pWalk->fFailed = PGM_WALKFAIL_BAD_PHYSICAL_ADDRESS  | ((uint32_t)uLevel << PGM_WALKFAIL_LEVEL_SHIFT);
    return VERR_PGM_INVALID_GC_PHYSICAL_ADDRESS;
}


DECLINLINE(int) pgmGstWalkFastReturnRsvdError(PVMCPUCC pVCpu, PPGMPTWALKFAST pWalk, uint8_t uLevel)
{
    RT_NOREF(pVCpu);
    pWalk->fFailed = PGM_WALKFAIL_RESERVED_BITS         | ((uint32_t)uLevel << PGM_WALKFAIL_LEVEL_SHIFT);
    return VERR_RESERVED_PAGE_TABLE_BITS;
}


/*
 * Special no paging variant.
 * Special no paging variant.
 * Special no paging variant.
 */

static PGM_CTX_DECL(int) PGM_CTX(pgm,GstNoneGetPage)(PVMCPUCC pVCpu, RTGCPTR GCPtr, PPGMPTWALK pWalk)
{
    RT_NOREF(pVCpu);

    RT_ZERO(*pWalk);
    pWalk->fSucceeded = true;
    pWalk->GCPtr      = GCPtr;
    pWalk->GCPhys     = GCPtr;
    pWalk->fEffective = 0; /** @todo */
    return VINF_SUCCESS;
}


static PGM_CTX_DECL(int) PGM_CTX(pgm,GstNoneQueryPageFast)(PVMCPUCC pVCpu, RTGCPTR GCPtr, uint32_t fFlags, PPGMPTWALKFAST pWalk)
{
    RT_NOREF(pVCpu, fFlags);

    pWalk->GCPtr        = GCPtr;
    pWalk->GCPhys       = GCPtr;
    pWalk->GCPhysNested = 0;
    pWalk->fInfo        = PGM_WALKINFO_SUCCEEDED;
    pWalk->fFailed      = PGM_WALKFAIL_SUCCESS;
    pWalk->fEffective   = 0; /** @todo */
    return VINF_SUCCESS;
}


static PGM_CTX_DECL(int) PGM_CTX(pgm,GstNoneModifyPage)(PVMCPUCC pVCpu, RTGCPTR GCPtr, size_t cb, uint64_t fFlags, uint64_t fMask)
{
    /* Ignore. */
    RT_NOREF(pVCpu, GCPtr, cb, fFlags, fMask);
    return VINF_SUCCESS;
}


static PGM_CTX_DECL(int) PGM_CTX(pgm,GstNoneWalk)(PVMCPUCC pVCpu, RTGCPTR GCPtr, PPGMPTWALK pWalk, PPGMPTWALKGST pGstWalk)
{
    RT_NOREF(pVCpu, GCPtr, pWalk);
    pGstWalk->enmType = PGMPTWALKGSTTYPE_INVALID;
    return VERR_PGM_NOT_USED_IN_MODE;
}


static PGM_CTX_DECL(int) PGM_CTX(pgm,GstNoneEnter)(PVMCPUCC pVCpu)
{
    /* Nothing to do. */
    RT_NOREF(pVCpu);
    return VINF_SUCCESS;
}


static PGM_CTX_DECL(int) PGM_CTX(pgm,GstNoneExit)(PVMCPUCC pVCpu)
{
    /* Nothing to do. */
    RT_NOREF(pVCpu);
    return VINF_SUCCESS;
}


/** @todo Templating for actual paging modes like we do for ARM-v8. */

/**
 * Guest mode data array.
 */
PGMMODEDATAGST const g_aPgmGuestModeData[PGM_GUEST_MODE_DATA_ARRAY_SIZE] =
{
    { UINT32_MAX, NULL, NULL, NULL, NULL, NULL }, /* 0 */
    {
        PGM_TYPE_NONE,
        PGM_CTX(pgm,GstNoneGetPage),
        PGM_CTX(pgm,GstNoneQueryPageFast),
        PGM_CTX(pgm,GstNoneModifyPage),
        PGM_CTX(pgm,GstNoneWalk),
        PGM_CTX(pgm,GstNoneEnter),
        PGM_CTX(pgm,GstNoneExit),
    },
};


DECLINLINE(uintptr_t) pgmR3DeduceTypeFromSatp(uint64_t u64CsrSatp)
{
    uintptr_t idxNewGst = 0;

    /** @todo */
    RT_NOREF(u64CsrSatp);
    idxNewGst = PGM_TYPE_NONE;

    return idxNewGst;
}
