/* $Id: CPUMR3CpuId-armv8.cpp 172109 2026-01-11 19:29:08Z bird $ */
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

#include <VBox/err.h>
#include <iprt/ctype.h>
#include <iprt/mem.h>
#include <iprt/sort.h>
#include <iprt/string.h>




/*
 *
 * Init related code.
 * Init related code.
 * Init related code.
 *
 *
 */

/** @name Instruction Set Extension Options
 * @{  */
/** Configuration option type (extended boolean, really). */
typedef uint8_t CPUMISAEXTCFG;
/** Always disable the extension. */
#define CPUMISAEXTCFG_DISABLED              false
/** Enable the extension if it's supported by the host CPU. */
#define CPUMISAEXTCFG_ENABLED_SUPPORTED     true
/** Enable the extension if it's supported by the host CPU, but don't let
 * the portable CPUID feature disable it. */
#define CPUMISAEXTCFG_ENABLED_PORTABLE      UINT8_C(127)
/** Always enable the extension. */
#define CPUMISAEXTCFG_ENABLED_ALWAYS        UINT8_C(255)
/** @} */

/**
 * CPUID Configuration (from CFGM).
 *
 * @remarks  The members aren't document since we would only be duplicating the
 *           \@cfgm entries in cpumR3CpuIdReadConfig.
 */
typedef struct CPUMCPUIDCONFIG
{
    CPUMISAEXTCFG   enmAes;
    /** @todo */

    char            szCpuName[128];
} CPUMCPUIDCONFIG;
/** Pointer to CPUID config (from CFGM). */
typedef CPUMCPUIDCONFIG *PCPUMCPUIDCONFIG;


/**
 * Sanitizes and adjust the CPUID leaves.
 *
 * Drop features that aren't virtualized (or virtualizable).  Adjust information
 * and capabilities to fit the virtualized hardware.  Remove information the
 * guest shouldn't have (because it's wrong in the virtual world or because it
 * gives away host details) or that we don't have documentation for and no idea
 * what means.
 *
 * @returns VBox status code.
 * @param   pVM         The cross context VM structure (for cCpus).
 * @param   pCpum       The CPUM instance data.
 * @param   pConfig     The CPUID configuration we've read from CFGM.
 * @param   pCpumCfg    The CPUM CFGM config.
 */
static int cpumR3CpuIdSanitize(PVM pVM, PCPUM pCpum, PCPUMCPUIDCONFIG pConfig, PCFGMNODE pCpumCfg)
{
#define PORTABLE_CLEAR_BITS_WHEN(Lvl, a_pLeafReg, FeatNm, fMask, uValue) \
        if ( pCpum->u8PortableCpuIdLevel >= (Lvl) && ((a_pLeafReg) & (fMask)) == (uValue) ) \
        { \
            LogRel(("PortableCpuId: " #a_pLeafReg "[" #FeatNm "]: %#x -> 0\n", (a_pLeafReg) & (fMask))); \
            (a_pLeafReg) &= ~(uint32_t)(fMask); \
        }
#define PORTABLE_DISABLE_FEATURE_BIT(Lvl, a_pLeafReg, FeatNm, fBitMask) \
        if ( pCpum->u8PortableCpuIdLevel >= (Lvl) && ((a_pLeafReg) & (fBitMask)) ) \
        { \
            LogRel(("PortableCpuId: " #a_pLeafReg "[" #FeatNm "]: 1 -> 0\n")); \
            (a_pLeafReg) &= ~(uint32_t)(fBitMask); \
        }
#define PORTABLE_DISABLE_FEATURE_BIT_CFG(Lvl, a_IdReg, a_FeatNm, a_IdRegValCheck, enmConfig, a_IdRegValNotSup) \
        if (   pCpum->u8PortableCpuIdLevel >= (Lvl) \
            && (RT_BF_GET(a_IdReg, a_FeatNm) >= a_IdRegValCheck) \
            && (enmConfig) != CPUMISAEXTCFG_ENABLED_PORTABLE ) \
        { \
            LogRel(("PortableCpuId: [" #a_FeatNm "]: 1 -> 0\n")); \
            (a_IdReg) = RT_BF_SET(a_IdReg, a_FeatNm, a_IdRegValNotSup); \
        }
    //Assert(pCpum->GuestFeatures.enmCpuVendor != CPUMCPUVENDOR_INVALID);

    /* The CPUID entries we start with here isn't necessarily the ones of the host, so we
       must consult HostFeatures when processing CPUMISAEXTCFG variables. */
#ifdef RT_ARCH_RISCV
    PCCPUMFEATURES pHstFeat = &pCpum->HostFeatures.s;
# define PASSTHRU_FEATURE(a_IdReg, enmConfig, fHostFeature, a_IdRegNm, a_IdRegValSup, a_IdRegValNotSup) do { \
            (a_IdReg) = (enmConfig) && ((enmConfig) == CPUMISAEXTCFG_ENABLED_ALWAYS || (fHostFeature)) \
                      ? RT_BF_SET(a_IdReg, a_IdRegNm, a_IdRegValSup) \
                      : RT_BF_SET(a_IdReg, a_IdRegNm, a_IdRegValNotSup); \
        } while (0)
#else
# define PASSTHRU_FEATURE(a_IdReg, enmConfig, fHostFeature, a_IdRegNm, a_IdRegValSup, a_IdRegValNotSup) do { \
            (a_IdReg) = (enmConfig) != CPUMISAEXTCFG_DISABLED \
                      ? RT_BF_SET(a_IdReg, a_IdRegNm, a_IdRegValSup) \
                      : RT_BF_SET(a_IdReg, a_IdRegNm, a_IdRegValNotSup); \
        } while (0)
#endif

    /** @todo Other ID and feature registers. */
    RT_NOREF(pVM, pCpum, pConfig, pCpumCfg);

#undef PORTABLE_DISABLE_FEATURE_BIT_CFG
#undef PORTABLE_DISABLE_FEATURE_BIT
#undef PORTABLE_CLEAR_BITS_WHEN
#undef PASSTHRU_FEATURE

    return VINF_SUCCESS;
}


/**
 * Reads a value in /CPUM/IsaExts/ node.
 *
 * @returns VBox status code (error message raised).
 * @param   pVM             The cross context VM structure. (For errors.)
 * @param   pIsaExts        The /CPUM/IsaExts node (can be NULL).
 * @param   pszValueName    The value / extension name.
 * @param   penmValue       Where to return the choice.
 * @param   enmDefault      The default choice.
 */
static int cpumR3CpuIdReadIsaExtCfg(PVM pVM, PCFGMNODE pIsaExts, const char *pszValueName,
                                    CPUMISAEXTCFG *penmValue, CPUMISAEXTCFG enmDefault)
{
    /*
     * Try integer encoding first.
     */
    uint64_t uValue;
    int rc = CFGMR3QueryInteger(pIsaExts, pszValueName, &uValue);
    if (RT_SUCCESS(rc))
        switch (uValue)
        {
            case 0: *penmValue = CPUMISAEXTCFG_DISABLED; break;
            case 1: *penmValue = CPUMISAEXTCFG_ENABLED_SUPPORTED; break;
            case 2: *penmValue = CPUMISAEXTCFG_ENABLED_ALWAYS; break;
            case 9: *penmValue = CPUMISAEXTCFG_ENABLED_PORTABLE; break;
            default:
                return VMSetError(pVM, VERR_CPUM_INVALID_CONFIG_VALUE, RT_SRC_POS,
                                  "Invalid config value for '/CPUM/IsaExts/%s': %llu (expected 0/'disabled', 1/'enabled', 2/'portable', or 9/'forced')",
                                  pszValueName, uValue);
        }
    /*
     * If missing, use default.
     */
    else if (rc == VERR_CFGM_VALUE_NOT_FOUND || rc == VERR_CFGM_NO_PARENT)
        *penmValue = enmDefault;
    else
    {
        if (rc == VERR_CFGM_NOT_INTEGER)
        {
            /*
             * Not an integer, try read it as a string.
             */
            char szValue[32];
            rc = CFGMR3QueryString(pIsaExts, pszValueName, szValue, sizeof(szValue));
            if (RT_SUCCESS(rc))
            {
                RTStrToLower(szValue);
                size_t cchValue = strlen(szValue);
#define EQ(a_str) (cchValue == sizeof(a_str) - 1U && memcmp(szValue, a_str, sizeof(a_str) - 1))
                if (     EQ("disabled") || EQ("disable") || EQ("off") || EQ("no"))
                    *penmValue = CPUMISAEXTCFG_DISABLED;
                else if (EQ("enabled")  || EQ("enable")  || EQ("on")  || EQ("yes"))
                    *penmValue = CPUMISAEXTCFG_ENABLED_SUPPORTED;
                else if (EQ("forced")   || EQ("force")   || EQ("always"))
                    *penmValue = CPUMISAEXTCFG_ENABLED_ALWAYS;
                else if (EQ("portable"))
                    *penmValue = CPUMISAEXTCFG_ENABLED_PORTABLE;
                else if (EQ("default")  || EQ("def"))
                    *penmValue = enmDefault;
                else
                    return VMSetError(pVM, VERR_CPUM_INVALID_CONFIG_VALUE, RT_SRC_POS,
                                      "Invalid config value for '/CPUM/IsaExts/%s': '%s' (expected 0/'disabled', 1/'enabled', 2/'portable', or 9/'forced')",
                                      pszValueName, uValue);
#undef EQ
            }
        }
        if (RT_FAILURE(rc))
            return VMSetError(pVM, rc, RT_SRC_POS, "Error reading config value '/CPUM/IsaExts/%s': %Rrc", pszValueName, rc);
    }
    return VINF_SUCCESS;
}


static int cpumR3CpuIdReadConfig(PVM pVM, PCPUMCPUIDCONFIG pConfig, PCFGMNODE pCpumCfg)
{
    /** @cfgm{/CPUM/PortableCpuIdLevel, 8-bit, 0, 3, 0}
     * When non-zero CPUID features that could cause portability issues will be
     * stripped.  The higher the value the more features gets stripped.  Higher
     * values should only be used when older CPUs are involved since it may
     * harm performance and maybe also cause problems with specific guests. */
    int rc = CFGMR3QueryU8Def(pCpumCfg, "PortableCpuIdLevel", &pVM->cpum.s.u8PortableCpuIdLevel, 0);
    AssertLogRelRCReturn(rc, rc);

    /** @cfgm{/CPUM/GuestCpuName, string}
     * The name of the CPU we're to emulate.  The default is the host CPU.
     * Note! CPUs other than "host" one is currently unsupported. */
    rc = CFGMR3QueryStringDef(pCpumCfg, "GuestCpuName", pConfig->szCpuName, sizeof(pConfig->szCpuName), "host");
    AssertLogRelRCReturn(rc, rc);

    /*
     * Instruction Set Architecture (ISA) Extensions.
     */
    PCFGMNODE pIsaExts = CFGMR3GetChild(pCpumCfg, "IsaExts");
    if (pIsaExts)
    {
        rc = CFGMR3ValidateConfig(pIsaExts, "/CPUM/IsaExts/",
                                   "AES"
                                  , "" /*pszValidNodes*/, "CPUM" /*pszWho*/, 0 /*uInstance*/);
        if (RT_FAILURE(rc))
            return rc;
    }

    /** @cfgm{/CPUM/IsaExts/AES, boolean, depends}
     * Expose FEAT_AES instruction set extension to the guest.
     */
    rc = cpumR3CpuIdReadIsaExtCfg(pVM, pIsaExts, "AES", &pConfig->enmAes, true /*enmDefault*/);
    AssertLogRelRCReturn(rc, rc);

    /** @todo Add more options for other extensions. */

    return VINF_SUCCESS;
}


#ifdef RT_ARCH_RISCV
/**
 * Populate guest feature ID registers.
 *
 * This operates in two modes:
 *      -# pfnQuery != NULL: Determin the guest feature register values and sets
 *         them in the execution manager calling us.
 *      -# pfnQuery == NULL: Enumerate the guest feature registers and set them
 *         in the execution manager calling us.
 *
 * @returns VBox status code.
 * @param   pVM         The cross context VM structure.
 * @param   pVCpu       For passing along to the callbacks.
 * @param   pfnQuery    Callback for query register values in order to establish
 *                      the guest feature ID register values. Specify NULL to
 *                      just get called back to set the value.
 * @param   pfnUpdate   Callback for update (setting) register values.
 * @param   pvUser      User specific callback argument.
 */
VMMR3_INT_DECL(int) CPUMR3PopulateGuestFeaturesViaCallbacks(PVM pVM, PVMCPU pVCpu, PFNCPUMARMCPUIDREGQUERY pfnQuery,
                                                            PFNCPUMARMCPUIDREGUPDATE pfnUpdate, void *pvUser)
{
    Assert(pfnUpdate);

    /*
     * If pfnQuery is given, we must determin the guest feature register values first.
     */
    if (pfnQuery)
    {
        Assert(pVCpu->idCpu == 0);

        /*
         * Read the configuration.
         */
        PCFGMNODE const pCpumCfg = CFGMR3GetChild(CFGMR3GetRoot(pVM), "CPUM");
        CPUMCPUIDCONFIG Config;
        RT_ZERO(Config);

        int rc = cpumR3CpuIdReadConfig(pVM, &Config, pCpumCfg);
        AssertRCReturn(rc, rc);

        /*
         * Do the common CPU ID register intialization.
         */
        rc = cpumR3InitCpuId(pVM);
        AssertLogRelRCReturn(rc, rc);
    }

    return rcRet;
}
#endif /* RT_ARCH_RISCV */


/**
 * Initalies the ARM system ID registers and features.
 *
 * This is either called via cpumR3InitTarget (exec engine == IEM) or via
 * CPUMR3PopulateGuestFeaturesViaCallbacks (exec engine == NEM).  In the former
 * case, the paIdRegsR3/cIdRegs pair is NULL/0, while it should be non-zero in
 * the latter.
 *
 * @returns VBox status code.
 * @param   pVM         The cross context VM structure.
 */
DECLHIDDEN(int) cpumR3InitCpuId(PVM pVM)
{
    /*
     * Read the configuration.
     */
    PCFGMNODE const pCpumCfg = CFGMR3GetChild(CFGMR3GetRoot(pVM), "CPUM");
    CPUMCPUIDCONFIG Config;
    RT_ZERO(Config);

    int rc = cpumR3CpuIdReadConfig(pVM, &Config, pCpumCfg);
    AssertRCReturn(rc, rc);

    /*
     * Get the guest CPU data from the database and/or the host.
     */
    rc = cpumR3DbGetCpuInfo(pVM, Config.szCpuName, &pVM->cpum.s.GuestInfo);
    if (RT_FAILURE(rc))
        return rc == VERR_CPUM_DB_CPU_NOT_FOUND
             ? VMSetError(pVM, rc, RT_SRC_POS,
                          "Info on guest CPU '%s' could not be found. Please, select a different CPU.", Config.szCpuName)
             : rc;

    /*
     * Sanitize the cpuid information passed on to the guest.
     */
    rc = cpumR3CpuIdSanitize(pVM, &pVM->cpum.s, &Config, pCpumCfg);
    AssertLogRelRCReturn(rc, rc);

    return VINF_SUCCESS;
}


/*
 *
 *
 * Saved state related code.
 * Saved state related code.
 * Saved state related code.
 *
 *
 */

/**
 * Called both in pass 0 and the final pass.
 *
 * @param   pVM                 The cross context VM structure.
 * @param   pSSM                The saved state handle.
 */
void cpumR3SaveCpuId(PVM pVM, PSSMHANDLE pSSM)
{
    RT_NOREF(pVM, pSSM);
}

