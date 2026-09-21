/* $Id$ */
/** @file
 * VBox Disassembler - RISC-V Style Formatter.
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
#include <VBox/dis.h>
#include "DisasmInternal.h"
#include "DisasmInternal-riscv.h"
#include <iprt/riscv.h>
#include <iprt/assert.h>
#include <iprt/ctype.h>
#include <iprt/errcore.h>
#include <iprt/string.h>


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/
static const char g_szSpaces[] =
"                                                                               ";
static const char g_aszRiscVRegGen[32][4] =
{
    "x0\0",  "x1\0",  "x2\0",  "x3\0",  "x4\0",  "x5\0",  "x6\0",  "x7\0",  "x8\0",  "x9\0",  "x10",  "x11",  "x12",  "x13",  "x14",  "x15",
    "x16",   "x17",   "x18",   "x19",   "x20",   "x21",   "x22",   "x23",   "x24",   "x25",   "x26",  "x27",  "x28",  "x29",  "x30",  "x31"
};


/**
 * List of known system registers.
 *
 * The list MUST be in ascending order of the system register ID!
 */
static const struct
{
    /** RISC-V CSR number. */
    uint16_t    idCsr;
    /** Name of the system register. */
    const char  *pszSysReg;
    /** Character count of the system register name. */
    size_t      cchSysReg;
} g_aRiscVCsr[] =
{
#define DIS_RISCV_CSR(a_idSysReg) { (RISCV_CSR_ ## a_idSysReg), #a_idSysReg, sizeof(#a_idSysReg) - 1 }
    { 0, "", 0 }
#undef  DIS_RISCV_CSR
};


/**
 * Gets the base register name for the given parameter.
 *
 * @returns Pointer to the register name.
 * @param   idReg       The register ID.
 * @param   pcchReg     Where to store the length of the name.
 */
DECLINLINE(const char *) disasmFormatRiscVReg(uint8_t idReg, size_t *pcchReg)
{
    AssertStmt(idReg < RT_ELEMENTS(g_aszRiscVRegGen), idReg = RT_ELEMENTS(g_aszRiscVRegGen) - 1);
    const char *psz = g_aszRiscVRegGen[idReg];
    *pcchReg = 2 + !!psz[2];
    return psz;
}


/**
 * Formats the current instruction in ARMv8 style.
 *
 *
 * @returns The number of output characters. If this is >= cchBuf, then the content
 *          of pszBuf will be truncated.
 * @param   pDis            Pointer to the disassembler state.
 * @param   pszBuf          The output buffer.
 * @param   cchBuf          The size of the output buffer.
 * @param   fFlags          Format flags, see DIS_FORMAT_FLAGS_*.
 * @param   pfnGetSymbol    Get symbol name for a jmp or call target address. Optional.
 * @param   pvUser          User argument for pfnGetSymbol.
 */
DISDECL(size_t) DISFormatRiscVEx(PCDISSTATE pDis, char *pszBuf, size_t cchBuf, uint32_t fFlags,
                                 PFNDISGETSYMBOL pfnGetSymbol, void *pvUser)
{
    /*
     * Input validation and massaging.
     */
    AssertPtr(pDis);
    AssertPtrNull(pszBuf);
    Assert(pszBuf || !cchBuf);
    AssertPtrNull(pfnGetSymbol);
    AssertMsg(DIS_FMT_FLAGS_IS_VALID(fFlags), ("%#x\n", fFlags));
    if (fFlags & DIS_FMT_FLAGS_ADDR_COMMENT)
        fFlags = (fFlags & ~DIS_FMT_FLAGS_ADDR_LEFT) | DIS_FMT_FLAGS_ADDR_RIGHT;
    if (fFlags & DIS_FMT_FLAGS_BYTES_COMMENT)
        fFlags = (fFlags & ~DIS_FMT_FLAGS_BYTES_LEFT) | DIS_FMT_FLAGS_BYTES_RIGHT;

    PCDISOPCODE const pOp = pDis->pCurInstr;

    /*
     * Output macros
     */
    char           *pszDst = pszBuf;
    size_t          cchDst = cchBuf;
    size_t          cchOutput = 0;
#define PUT_C(ch)       \
            do { \
                cchOutput++; \
                if (cchDst > 1) \
                { \
                    cchDst--; \
                    *pszDst++ = (ch); \
                } \
            } while (0)
#define PUT_STR(pszSrc, cchSrc) \
            do { \
                cchOutput += (cchSrc); \
                if (cchDst > (cchSrc)) \
                { \
                    memcpy(pszDst, (pszSrc), (cchSrc)); \
                    pszDst += (cchSrc); \
                    cchDst -= (cchSrc); \
                } \
                else if (cchDst > 1) \
                { \
                    memcpy(pszDst, (pszSrc), cchDst - 1); \
                    pszDst += cchDst - 1; \
                    cchDst = 1; \
                } \
            } while (0)
#define PUT_SZ(sz) \
            PUT_STR((sz), sizeof(sz) - 1)
#define PUT_SZ_STRICT(szStrict, szRelaxed) \
            do { if (fFlags & DIS_FMT_FLAGS_STRICT) PUT_SZ(szStrict); else PUT_SZ(szRelaxed); } while (0)
#define PUT_PSZ(psz) \
            do { const size_t cchTmp = strlen(psz); PUT_STR((psz), cchTmp); } while (0)
#define PUT_NUM(cch, fmt, num) \
            do { \
                 cchOutput += (cch); \
                 if (cchDst > 1) \
                 { \
                    const size_t cchTmp = RTStrPrintf(pszDst, cchDst, fmt, (num)); \
                    pszDst += cchTmp; \
                    cchDst -= cchTmp; \
                    Assert(cchTmp == (cch) || cchDst == 1); \
                 } \
            } while (0)
/** @todo add two flags for choosing between %X / %x and h / 0x. */
#define PUT_NUM_8(num)  PUT_NUM(4,  "0x%02x", (uint8_t)(num))
#define PUT_NUM_16(num) PUT_NUM(6,  "0x%04x", (uint16_t)(num))
#define PUT_NUM_32(num) PUT_NUM(10, "0x%08x", (uint32_t)(num))
#define PUT_NUM_64(num) PUT_NUM(18, "0x%016RX64", (uint64_t)(num))

#define PUT_NUM_DECIMAL_XX(num) PUT_NUM((unsigned)(num) < 10 ? 1U : 2U,  "%u", (unsigned)(num))

#define PUT_NUM_SIGN(cch, fmt, num, stype, utype) \
            do { \
                if ((stype)(num) >= 0) \
                { \
                    PUT_C('+'); \
                    PUT_NUM(cch, fmt, (utype)(num)); \
                } \
                else \
                { \
                    PUT_C('-'); \
                    PUT_NUM(cch, fmt, (utype)-(stype)(num)); \
                } \
            } while (0)
#define PUT_NUM_S8(num)  PUT_NUM_SIGN(4,  "0x%02x", num, int8_t,  uint8_t)
#define PUT_NUM_S16(num) PUT_NUM_SIGN(6,  "0x%04x", num, int16_t, uint16_t)
#define PUT_NUM_S32(num) PUT_NUM_SIGN(10, "0x%08x", num, int32_t, uint32_t)
#define PUT_NUM_S64(num) PUT_NUM_SIGN(18, "0x%016RX64", num, int64_t, uint64_t)

#define PUT_SYMBOL_TWO(a_rcSym, a_szStart, a_chEnd) \
        do { \
            if (RT_SUCCESS(a_rcSym)) \
            { \
                PUT_SZ(a_szStart); \
                PUT_PSZ(szSymbol); \
                if (off != 0) \
                { \
                    if ((int8_t)off == off) \
                        PUT_NUM_S8(off); \
                    else if ((int16_t)off == off) \
                        PUT_NUM_S16(off); \
                    else if ((int32_t)off == off) \
                        PUT_NUM_S32(off); \
                    else \
                        PUT_NUM_S64(off); \
                } \
                PUT_C(a_chEnd); \
            } \
        } while (0)

#define PUT_SYMBOL(a_uSeg, a_uAddr, a_szStart, a_chEnd) \
        do { \
            if (pfnGetSymbol) \
            { \
                int rcSym = pfnGetSymbol(pDis, a_uSeg, a_uAddr, szSymbol, sizeof(szSymbol), &off, pvUser); \
                PUT_SYMBOL_TWO(rcSym, a_szStart, a_chEnd); \
            } \
        } while (0)


    /*
     * The address?
     */
    if (fFlags & DIS_FMT_FLAGS_ADDR_LEFT)
    {
        if (pDis->uInstrAddr >= _4G)
            PUT_NUM(9, "%08x`", (uint32_t)(pDis->uInstrAddr >> 32));
        PUT_NUM(8, "%08x", (uint32_t)pDis->uInstrAddr);
        PUT_C(' ');
    }

    /*
     * The opcode bytes?
     */
    if (fFlags & DIS_FMT_FLAGS_BYTES_LEFT)
    {
        size_t cchTmp = disFormatBytes(pDis, pszDst, cchDst, fFlags);
        cchOutput += cchTmp;
        if (cchDst > 1)
        {
            if (cchTmp <= cchDst)
            {
                cchDst -= cchTmp;
                pszDst += cchTmp;
            }
            else
            {
                pszDst += cchDst - 1;
                cchDst = 1;
            }
        }

        /* Some padding to align the instruction. */
        size_t cchPadding = (7 * (2 + !!(fFlags & DIS_FMT_FLAGS_BYTES_SPACED)))
                          + !!(fFlags & DIS_FMT_FLAGS_BYTES_BRACKETS) * 2
                          + 2;
        cchPadding = cchTmp + 1 >= cchPadding ? 1 : cchPadding - cchTmp;
        PUT_STR(g_szSpaces, cchPadding);
    }


    /*
     * Filter out invalid opcodes first as they need special
     * treatment. UDF is an exception and should be handled normally.
     */
    size_t const offInstruction = cchOutput;
    if (pOp->uOpcode == OP_INVALID)
        PUT_SZ("Illegal opcode");
    else
    {
        /* Start with the instruction. */
        PUT_PSZ(pOp->pszOpcode);

        /*
         * Format the parameters.
         */
        RTINTPTR off;
        char szSymbol[128];
        RT_NOREF(pvUser);
        uint32_t cSkippedParams = 0;
        for (uint32_t i = 0; i < RT_ELEMENTS(pDis->aParams); i++)
        {
            PCDISOPPARAM pParam = &pDis->aParams[i];

            /* First None parameter marks end of parameters. */
            if (pParam->riscv.enmType == kDisRiscVOpParmNone)
                break;

            if (i > cSkippedParams)
                PUT_C(',');
            PUT_C(' '); /** @todo Make the indenting configurable. */

            switch (pParam->riscv.enmType)
            {
                case kDisRiscVOpParmImm:
                {
                    switch (pParam->fUse & (  DISUSE_IMMEDIATE8 | DISUSE_IMMEDIATE16 | DISUSE_IMMEDIATE32 | DISUSE_IMMEDIATE64
                                            | DISUSE_IMMEDIATE16_SX8 | DISUSE_IMMEDIATE32_SX8 | DISUSE_IMMEDIATE64_SX8))
                    {
                        case DISUSE_IMMEDIATE8:
                            PUT_NUM_8(pParam->uValue);
                            /** @todo Symbols */
                            break;
                        case DISUSE_IMMEDIATE32:
                            PUT_NUM_S32(pParam->uValue);
                            /** @todo Symbols */
                            break;
                        case DISUSE_IMMEDIATE64:
                            PUT_NUM_S64(pParam->uValue);
                            /** @todo Symbols */
                            break;
                        default:
                            AssertFailed();
                            break;
                    }
                    break;
                }
                case kDisRiscVOpParmImmRel:
                {
                    RTUINTPTR uTrgAddr = pDis->uInstrAddr;
                    int64_t offDisplacement;

                    /*
                     * Explicit immediate constants are required to be prepended by a dot with gas or llvm,
                     * or they get treated as either relative or absolute addresses and get interfered with
                     * by the linker.
                     *
                     * However auipc seems to be different here.
                     */
                    if (pOp->uOpcode != OP_RISCV_AUIPC)
                        PUT_C('.');

                    if (pParam->fUse & DISUSE_IMMEDIATE8_REL)
                    {
                        offDisplacement = (int8_t)pParam->uValue;
                        if (fFlags & DIS_FMT_FLAGS_RELATIVE_BRANCH)
                            PUT_NUM_S8(offDisplacement);
                    }
                    else if (pParam->fUse & DISUSE_IMMEDIATE16_REL)
                    {
                        offDisplacement = (int16_t)pParam->uValue;
                        if (fFlags & DIS_FMT_FLAGS_RELATIVE_BRANCH)
                            PUT_NUM_S16(offDisplacement);
                    }
                    else if (pParam->fUse & (DISUSE_IMMEDIATE32_REL | DISUSE_IMMEDIATE32 /*adr*/))
                    {
                        offDisplacement = (int32_t)pParam->uValue;
                        if (fFlags & DIS_FMT_FLAGS_RELATIVE_BRANCH)
                            PUT_NUM_S32(offDisplacement);
                    }
                    if (fFlags & DIS_FMT_FLAGS_RELATIVE_BRANCH)
                        PUT_SZ(" ; (");

                    uTrgAddr += offDisplacement;
                    if (pDis->uCpuMode == DISCPUMODE_RISCV_RV32)
                        PUT_NUM_32(uTrgAddr);
                    else if (pDis->uCpuMode == DISCPUMODE_RISCV_RV64)
                        PUT_NUM_64(uTrgAddr);
                    else
                        AssertReleaseFailed();

                    if (fFlags & DIS_FMT_FLAGS_RELATIVE_BRANCH)
                    {
                        PUT_SYMBOL(DIS_FMT_SEL_FROM_REG(DISSELREG_CS), uTrgAddr, " = ", ' ');
                        PUT_C(')');
                    }
                    else
                        PUT_SYMBOL(DIS_FMT_SEL_FROM_REG(DISSELREG_CS), uTrgAddr, " (", ')');
                    break;
                }
#if 0
                case kDisArmv8OpParmImmFp:
                {
                    PUT_C('#');
                    switch (pParam->fUse & DISUSE_IMMEDIATE)
                    {
                        /** @todo floating point formatting. */
                        case DISUSE_IMMEDIATE16:
                            PUT_NUM_16(pParam->uValue);
                            break;
                        case DISUSE_IMMEDIATE32:
                            PUT_NUM_32(pParam->uValue);
                            break;
                        case DISUSE_IMMEDIATE64:
                            PUT_NUM_64(pParam->uValue);
                            break;
                        default:
                            AssertFailed();
                            break;
                    }
                    break;
                }
#endif
                case kDisRiscVOpParmGpr:
                {
                    Assert(!(pParam->fUse & (DISUSE_DISPLACEMENT8 | DISUSE_DISPLACEMENT16 | DISUSE_DISPLACEMENT32 | DISUSE_DISPLACEMENT64 | DISUSE_RIPDISPLACEMENT32)));

                    size_t cchTmp;
                    const char *pszTmp = disasmFormatRiscVReg(pParam->riscv.Op.u8Gpr, &cchTmp);
                    PUT_STR(pszTmp, cchTmp);
                    break;
                }
#if 0
                case kDisArmv8OpParmSysReg:
                {
                    Assert(pParam->fUse == DISUSE_REG_SYSTEM);

                    size_t cchReg;
                    char achTmp[32];
                    const char *pszReg = disasmFormatArmV8SysReg(pDis, pParam, &achTmp[0], &cchReg);
                    PUT_STR(pszReg, cchReg);
                    break;
                }
#endif
                case kDisRiscVOpParmAddrInGpr:
                {
                    Assert(   (pParam->fUse & (DISUSE_PRE_INDEXED | DISUSE_POST_INDEXED))
                           != (DISUSE_PRE_INDEXED | DISUSE_POST_INDEXED));

                    PUT_NUM_S32(pParam->riscv.u.offBase);
                    PUT_C('(');

                    size_t cchReg;
                    const char *pszReg = disasmFormatRiscVReg(pParam->riscv.Op.u8Gpr, &cchReg);
                    PUT_STR(pszReg, cchReg);

                    PUT_C(')');
                    break;
                }
                default:
                    AssertFailed();
            }

        }
    }

    /*
     * Any additional output to the right of the instruction?
     */
    if (fFlags & (DIS_FMT_FLAGS_BYTES_RIGHT | DIS_FMT_FLAGS_ADDR_RIGHT))
    {
        /* some up front padding. */
        size_t cchPadding = cchOutput - offInstruction;
        cchPadding = cchPadding + 1 >= 42 ? 1 : 42 - cchPadding;
        PUT_STR(g_szSpaces, cchPadding);

        /* comment? */
        PUT_SZ(";");

        /*
         * The address?
         */
        if (fFlags & DIS_FMT_FLAGS_ADDR_RIGHT)
        {
            PUT_C(' ');
            if (pDis->uInstrAddr >= _4G)
                PUT_NUM(9, "%08x`", (uint32_t)(pDis->uInstrAddr >> 32));
            PUT_NUM(8, "%08x", (uint32_t)pDis->uInstrAddr);
        }

        /*
         * Opcode bytes?
         */
        if (fFlags & DIS_FMT_FLAGS_BYTES_RIGHT)
        {
            PUT_C(' ');
            size_t cchTmp = disFormatBytes(pDis, pszDst, cchDst, fFlags);
            cchOutput += cchTmp;
            if (cchTmp >= cchDst)
                cchTmp = cchDst - (cchDst != 0);
            cchDst -= cchTmp;
            pszDst += cchTmp;
        }
    }

    /*
     * Terminate it - on overflow we'll have reserved one byte for this.
     */
    if (cchDst > 0)
        *pszDst = '\0';
    else
        Assert(!cchBuf);

    /* clean up macros */
#undef PUT_PSZ
#undef PUT_SZ
#undef PUT_STR
#undef PUT_C
    return cchOutput;
}


/**
 * Formats the current instruction in RISC-V style.
 *
 * This is a simplified version of DISFormatRiscVEx() provided for your convenience.
 *
 *
 * @returns The number of output characters. If this is >= cchBuf, then the content
 *          of pszBuf will be truncated.
 * @param   pDis    Pointer to the disassembler state.
 * @param   pszBuf  The output buffer.
 * @param   cchBuf  The size of the output buffer.
 */
DISDECL(size_t) DISFormatRiscV(PCDISSTATE pDis, char *pszBuf, size_t cchBuf)
{
    return DISFormatRiscVEx(pDis, pszBuf, cchBuf, 0 /* fFlags */, NULL /* pfnGetSymbol */, NULL /* pvUser */);
}
