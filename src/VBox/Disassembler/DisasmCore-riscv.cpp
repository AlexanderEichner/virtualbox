/* $Id$ */
/** @file
 * VBox Disassembler - Core Components.
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
#define LOG_GROUP LOG_GROUP_DIS
#include <VBox/dis.h>
#include <VBox/log.h>
#include <iprt/riscv.h>
#include <iprt/assert.h>
#include <iprt/errcore.h>
#include <iprt/param.h>
#include <iprt/string.h>
#include <iprt/stdarg.h>
#include "DisasmInternal-riscv.h"


/*********************************************************************************************************************************
*   Structures and Typedefs                                                                                                      *
*********************************************************************************************************************************/

/** Parser callback.
 * @remark no DECLCALLBACK() here because it's considered to be internal and
 *         there is no point in enforcing CDECL. */
typedef int FNDISPARSERISCV(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass);
/** Pointer to a disassembler parser function. */
typedef FNDISPARSERISCV *PFNDISPARSERISCV;


/*********************************************************************************************************************************
*   Internal Functions                                                                                                           *
*********************************************************************************************************************************/
/** @name Parsers
 * @{ */
static FNDISPARSERISCV disRiscVParseIllegal;
static FNDISPARSERISCV disRiscVParseReg;
static FNDISPARSERISCV disRiscVParseImm;
static FNDISPARSERISCV disRiscVParseStore;
static FNDISPARSERISCV disRiscVParseLoad;
static FNDISPARSERISCV disRiscVParseBranch;
static FNDISPARSERISCV disRiscVParseU;
static FNDISPARSERISCV disRiscVParseJmp;
static FNDISPARSERISCV disRiscVParseJal;
static FNDISPARSERISCV disRiscVParseShamt;
/** @}  */


#if 0
static int disRiscVParseInstruction(PDISSTATE pDis, uint32_t u32Insn);
static int disRiscVParseInvOpcode(PDISSTATE pDis);
#endif


/*********************************************************************************************************************************
*   Global Variables                                                                                                             *
*********************************************************************************************************************************/
/** Parser opcode table for full disassembly. */
static PFNDISPARSERISCV const g_apfnDisasm[kDisParmParseMax] =
{
    disRiscVParseIllegal,
    disRiscVParseReg,
    disRiscVParseImm,
    disRiscVParseStore,
    disRiscVParseLoad,
    disRiscVParseBranch,
    disRiscVParseU,
    disRiscVParseJmp,
    disRiscVParseJal,
    disRiscVParseShamt
};


DECLINLINE(uint32_t) disRiscVExtractBitVecFromInsn(uint32_t u32Insn, uint8_t idxBitStart, uint8_t cBits)
{
    uint32_t fMask = (uint32_t)(RT_BIT_64(idxBitStart + cBits) - 1);
    return (u32Insn & fMask) >> idxBitStart;
}


DECLINLINE(int32_t) disRiscVExtractBitVecFromInsnSignExtend(uint32_t u32Insn, uint8_t idxBitStart, uint8_t cBits)
{
    uint32_t const fMask = RT_BIT_32(cBits) - 1;
    uint32_t const fSignBit = RT_BIT_32(cBits - 1);
    uint32_t const u32 = (u32Insn >> idxBitStart) & fMask;
    return (int32_t)((u32 ^ fSignBit) - fSignBit);
}


DECLINLINE(int32_t) disRiscVExtractBitVecFromInsnSignExtendSplit(uint32_t u32Insn, uint8_t idxBitStart1, uint8_t cBits1,
                                                                 uint8_t idxBitStart2, uint8_t cBits2)
{
    uint32_t const fMask1 = RT_BIT_32(cBits1) - 1;
    uint32_t const fMask2 = RT_BIT_32(cBits2) - 1;
    uint32_t const fSignBit = RT_BIT_32(cBits1 + cBits2 - 1);
    uint32_t const u32 =   ((u32Insn >> idxBitStart1) & fMask1)
                         | (((u32Insn >> idxBitStart2) & fMask2) << cBits1);
    return (int32_t)((u32 ^ fSignBit) - fSignBit);
}


static int disRiscVParseIllegal(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pDis, u32Insn, pOp, pInsnClass);
    AssertFailed();
    return VERR_INTERNAL_ERROR;
}


static int disRiscVParseReg(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pDis, u32Insn, pOp, pInsnClass);
    AssertFailed();
    return VERR_INTERNAL_ERROR;
}


static int disRiscVParseImm(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);
    int32_t const i32Imm   = disRiscVExtractBitVecFromInsnSignExtend(u32Insn, 20, 12);
    uint8_t const uGprDest = disRiscVExtractBitVecFromInsn(u32Insn,  7, 5);
    uint8_t const uGprSrc  = disRiscVExtractBitVecFromInsn(u32Insn, 15, 5);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = uGprDest;

    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[1].riscv.Op.u8Gpr  = uGprSrc;

    pDis->aParams[2].riscv.enmType   = kDisRiscVOpParmImm;
    pDis->aParams[2].riscv.cb        = sizeof(uint32_t);
    pDis->aParams[2].uValue          = (int64_t)i32Imm;
    pDis->aParams[2].fUse            = pDis->uCpuMode == DISCPUMODE_RISCV_RV32
                                       ? DISUSE_IMMEDIATE32
                                       : DISUSE_IMMEDIATE64;
    return VINF_SUCCESS;
}


static int disRiscVParseStore(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);
    int32_t const offBase = disRiscVExtractBitVecFromInsnSignExtendSplit(u32Insn, 7, 5, 25, 7);
    uint8_t const uGprBase = disRiscVExtractBitVecFromInsn(u32Insn, 15, 5);
    uint8_t const uGprSrc  = disRiscVExtractBitVecFromInsn(u32Insn, 20, 5);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = uGprSrc;
    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmAddrInGpr;
    pDis->aParams[1].riscv.Op.u8Gpr  = uGprBase;
    pDis->aParams[1].riscv.u.offBase = offBase;
    return VINF_SUCCESS;
}


static int disRiscVParseLoad(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);
    int32_t const offBase = disRiscVExtractBitVecFromInsnSignExtend(u32Insn, 20, 12);
    uint8_t const uGprBase = disRiscVExtractBitVecFromInsn(u32Insn, 15, 5);
    uint8_t const uGprDest = disRiscVExtractBitVecFromInsn(u32Insn,  7, 5);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = uGprDest;
    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmAddrInGpr;
    pDis->aParams[1].riscv.Op.u8Gpr  = uGprBase;
    pDis->aParams[1].riscv.u.offBase = offBase;
    return VINF_SUCCESS;
}


static int disRiscVParseBranch(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pDis, u32Insn, pOp, pInsnClass);
    AssertFailed();
    return VERR_INTERNAL_ERROR;
}


static int disRiscVParseU(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = disRiscVExtractBitVecFromInsn(u32Insn,  7, 5);

    pDis->aParams[1].riscv.enmType   =   pOp->Opc.uOpcode == OP_RISCV_AUIPC
                                       ? kDisRiscVOpParmImmRel
                                       : kDisRiscVOpParmImm;
    pDis->aParams[1].riscv.cb        = sizeof(uint32_t);
    pDis->aParams[1].uValue          = (int64_t)disRiscVExtractBitVecFromInsn(u32Insn, 12, 20);
    pDis->aParams[1].fUse            =   pOp->Opc.uOpcode == OP_RISCV_AUIPC
                                       ? DISUSE_IMMEDIATE32_REL
                                       : DISUSE_IMMEDIATE32;
    return VINF_SUCCESS;
}


static int disRiscVParseJal(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);
    uint8_t  const uGprDest    = disRiscVExtractBitVecFromInsn(u32Insn,  7,  5);
    uint32_t const fSignBit    = RT_BIT_32(20);
    uint32_t const u32Imm20    = disRiscVExtractBitVecFromInsn(u32Insn, 31,  1);
    uint32_t const u32Imm10_1  = disRiscVExtractBitVecFromInsn(u32Insn, 21, 10);
    uint32_t const u32Imm11    = disRiscVExtractBitVecFromInsn(u32Insn, 20,  1);
    uint32_t const u32Imm19_12 = disRiscVExtractBitVecFromInsn(u32Insn, 12,  8);

    uint32_t const u32 =   (u32Imm10_1  <<  1)
                         | (u32Imm11    << 11)
                         | (u32Imm19_12 << 12)
                         | (u32Imm20    << 20);
    int32_t off = ((u32 ^ fSignBit) - fSignBit);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = uGprDest;

    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmImmRel;
    pDis->aParams[1].riscv.cb        = sizeof(int32_t);
    pDis->aParams[1].uValue          = (int64_t)off;
    pDis->aParams[1].fUse            = DISUSE_IMMEDIATE32_REL;
    return VINF_SUCCESS;
}


static int disRiscVParseJmp(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);
    uint8_t  const uGprSrc1   = disRiscVExtractBitVecFromInsn(u32Insn, 15,  5);
    uint8_t  const uGprSrc2   = disRiscVExtractBitVecFromInsn(u32Insn, 20,  5);
    uint32_t const fSignBit   = RT_BIT_32(12);
    uint32_t const u32Imm12   = disRiscVExtractBitVecFromInsn(u32Insn, 31,  1);
    uint32_t const u32Imm10_5 = disRiscVExtractBitVecFromInsn(u32Insn, 25,  6);
    uint32_t const u32Imm4_1  = disRiscVExtractBitVecFromInsn(u32Insn,  8,  4);
    uint32_t const u32Imm11   = disRiscVExtractBitVecFromInsn(u32Insn,  7,  1);

    uint32_t const u32 =   (u32Imm4_1  << 1)
                         | (u32Imm10_5 << 5)
                         | (u32Imm11   << 11)
                         | (u32Imm12   << 12);
    int32_t off = ((u32 ^ fSignBit) - fSignBit);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = uGprSrc1;

    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[1].riscv.Op.u8Gpr  = uGprSrc2;

    pDis->aParams[2].riscv.enmType   = kDisRiscVOpParmImmRel;
    pDis->aParams[2].riscv.cb        = sizeof(int32_t);
    pDis->aParams[2].uValue          = (int64_t)off;
    pDis->aParams[2].fUse            = DISUSE_IMMEDIATE32_REL;
    return VINF_SUCCESS;
}


static int disRiscVParseShamt(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(pOp, pInsnClass);
    uint8_t const u8Shamt  = disRiscVExtractBitVecFromInsn(u32Insn, 20, pDis->uCpuMode == DISCPUMODE_RISCV_RV32 ? 5 : 6);
    uint8_t const uGprDest = disRiscVExtractBitVecFromInsn(u32Insn,  7, 5);
    uint8_t const uGprSrc  = disRiscVExtractBitVecFromInsn(u32Insn, 15, 5);

    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[0].riscv.Op.u8Gpr  = uGprDest;

    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmGpr;
    pDis->aParams[1].riscv.Op.u8Gpr  = uGprSrc;

    pDis->aParams[2].riscv.enmType   = kDisRiscVOpParmImm;
    pDis->aParams[2].riscv.cb        = sizeof(uint32_t);
    pDis->aParams[2].uValue          = (int64_t)u8Shamt;
    pDis->aParams[2].fUse            = DISUSE_IMMEDIATE8;
    return VINF_SUCCESS;
}


/**
 * Looks for possible alias conversions for the given disassembler state.
 *
 * @param   pDis        The disassembler state to process.
 */
static void disRiscVInsnAliasesProcess(PDISSTATE pDis)
{
    RT_NOREF(pDis);
}


static int disRiscVParseInstruction(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVOPCODE pOp, PCDISRISCVINSNCLASS pInsnClass)
{
    RT_NOREF(u32Insn);
    AssertPtr(pDis);
#if 0
    //Assert((u32Insn & pInsnClass->fFixedInsn) == pOp->fValue);
    if ((u32Insn & pInsnClass->fFixedInsn) != pOp->fValue)
        return VERR_DIS_INVALID_OPCODE;
#endif

    /* Should contain the parameter type on input. */
    pDis->aParams[0].fUse            = 0;
    pDis->aParams[1].fUse            = 0;
    pDis->aParams[2].fUse            = 0;
    pDis->aParams[3].fUse            = 0;
    pDis->aParams[0].riscv.enmType   = kDisRiscVOpParmNone;
    pDis->aParams[1].riscv.enmType   = kDisRiscVOpParmNone;
    pDis->aParams[2].riscv.enmType   = kDisRiscVOpParmNone;
    pDis->aParams[3].riscv.enmType   = kDisRiscVOpParmNone;
    pDis->riscv.cbOperand            = 0;

    pDis->pCurInstr = &pOp->Opc;
    Assert(&pOp->Opc != &g_RiscVInvalidOpcode[0]);

    int rc = VINF_SUCCESS;
    Assert(pInsnClass->enmOpcDecode < kDisParmParseMax);
    Assert(g_apfnDisasm[pInsnClass->enmOpcDecode]);
    rc = g_apfnDisasm[pInsnClass->enmOpcDecode](pDis, u32Insn, pOp, pInsnClass);

    /* If parameter parsing returned an invalid opcode error the encoding is invalid. */
    if (RT_SUCCESS(rc)) /** @todo Introduce flag to switch alias conversion on/off. */
        disRiscVInsnAliasesProcess(pDis);
    else if (rc == VERR_DIS_INVALID_OPCODE)
    {
        pDis->pCurInstr = &g_RiscVInvalidOpcode[0];

        pDis->aParams[0].riscv.enmType = kDisRiscVOpParmNone;
        pDis->aParams[1].riscv.enmType = kDisRiscVOpParmNone;
        pDis->aParams[2].riscv.enmType = kDisRiscVOpParmNone;
        pDis->aParams[3].riscv.enmType = kDisRiscVOpParmNone;
    }
    pDis->rc = rc;
    return rc;
}


static int disRiscVParseInvOpcode(PDISSTATE pDis)
{
    pDis->pCurInstr = &g_RiscVInvalidOpcode[0];
    pDis->rc = VERR_DIS_INVALID_OPCODE;
    return VERR_DIS_INVALID_OPCODE;
}


static int disInstrRiscVDecodeWorker(PDISSTATE pDis, uint32_t u32Insn, PCDISRISCVDECODEMAP pMap)
{
    uint32_t idxNext = (u32Insn & pMap->fMask) >> pMap->cShift;
    if (RT_LIKELY(idxNext < pMap->cClasses))
    {
        PCDISRISCVINSNCLASS pInsnClass = pMap->papInsnClass[idxNext];

        for (;;)
        {
            uint32_t uOpcRaw = 0;

            if (pInsnClass->cOpcodes > 1)
                uOpcRaw = (u32Insn & pInsnClass->fMask) >> pInsnClass->cShift;
            if (uOpcRaw < pInsnClass->cOpcodes)
            {
                PCDISRISCVOPCORMAP pOp = &pInsnClass->paOpcodes[uOpcRaw];
                if (pOp->fOpCode)
                    return disRiscVParseInstruction(pDis, u32Insn, &pOp->Op, pInsnClass);

                pInsnClass = pOp->pInsnClass;
            }
            else
                break;
        }
    }

    return disRiscVParseInvOpcode(pDis);
}


/**
 * Internal worker for DISInstrEx and DISInstrWithPrefetchedBytes.
 *
 * @returns VBox status code.
 * @param   pDis            Initialized disassembler state.
 * @param   paOneByteMap    The one byte opcode map to use.
 * @param   pcbInstr        Where to store the instruction size. Can be NULL.
 */
DECLHIDDEN(int) disInstrWorkerRiscV(PDISSTATE pDis, PCDISOPCODE paOneByteMap, uint32_t *pcbInstr)
{
    RT_NOREF(paOneByteMap);

    if (pDis->uCpuMode == DISCPUMODE_RISCV_RV64)
    {
        /*
         * Instructions are either 32-bit or 16-bit if the instruction is compressed.
         * This is determined based on bits [1:0] of the first instruction byte.
         * A RISC-V instruction stream is always stored little endian in memory.
         */
        /** @todo Read 16-bit first and only read the remaining 16-bit if not compressed to avoid
         *        errors if the instruction is the last one in a page, followed by a memory hole.
         */
        uint32_t u32Insn = disReadDWord(pDis, 0 /*offInstr*/);
        if (RT_FAILURE(pDis->rc))
            return pDis->rc;

        if ((u32Insn & 0x3) == 0x3)
        {
            pDis->Instr.u32 = RT_LE2H_U32(u32Insn);
            pDis->cbInstr   = sizeof(uint32_t);

            if (pcbInstr)
                *pcbInstr = sizeof(uint32_t);

            return disInstrRiscVDecodeWorker(pDis, u32Insn, &g_aRiscVInsnDecodeL0);
        }
        else
            AssertFailed(); /** @todo */
    }

    AssertReleaseFailed();
    return VERR_NOT_IMPLEMENTED;
}


/**
 * Inlined worker that initializes the disassembler state.
 *
 * @returns The primary opcode map to use.
 * @param   pDis            The disassembler state.
 * @param   uInstrAddr      The instruction address.
 * @param   enmCpuMode      The CPU mode.
 * @param   fFilter         The instruction filter settings.
 * @param   pfnReadBytes    The byte reader, can be NULL.
 * @param   pvUser          The user data for the reader.
 */
DECLHIDDEN(PCDISOPCODE) disInitializeStateRiscV(PDISSTATE pDis, DISCPUMODE enmCpuMode, uint32_t fFilter)
{
    RT_NOREF(pDis, enmCpuMode, fFilter);
    return NULL;
}
