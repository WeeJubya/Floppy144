/*
 * Floppy//144 - reusable secure-cabinet access and Cabinet Interior view
 */

#include "floppy144_cabinet.h"

#include "floppy144_draw.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_site.h"
#include "floppy144_site_rooms.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static bool Floppy144CabinetStringEqual(
    const char *pszA,
    const char *pszB
)
{
    return
        pszA != NULL &&
        pszB != NULL &&
        strcmp(pszA, pszB) == 0;
}

static bool Floppy144CabinetRecordIsSecure(
    const Floppy144DataRecord *pRecord
)
{
    return
        pRecord != NULL &&
        pRecord->eKind == FLOPPY144_DATA_FURNITURE &&
        Floppy144CabinetStringEqual(
            pRecord->pszC,
            "SECURE_CABINET"
        );
}

static int32_t Floppy144CabinetOrdinalForId(
    const char *pszCabinetId
)
{
    uint32_t uRecordIndex;
    int32_t nOrdinal = 0;

    if(pszCabinetId == NULL)
        return -1;

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        if(!Floppy144CabinetRecordIsSecure(pRecord))
            continue;

        if(Floppy144CabinetStringEqual(pRecord->pszId, pszCabinetId))
            return nOrdinal;

        ++nOrdinal;
    }

    return -1;
}

static uint32_t Floppy144CabinetDistanceSquared(
    const Floppy144RunState *pRunState,
    const Floppy144DataRecord *pCabinet
)
{
    int32_t nX0;
    int32_t nX1;
    int32_t nY0;
    int32_t nY1;
    int32_t nDx = 0;
    int32_t nDy = 0;

    if(pRunState == NULL || pCabinet == NULL)
        return UINT32_MAX;

    nX0 = pCabinet->n0 * FLOPPY144_SITE_FIXED_ONE;
    nX1 = (pCabinet->n0 + pCabinet->n2) * FLOPPY144_SITE_FIXED_ONE;
    nY0 = pCabinet->n1 * FLOPPY144_SITE_FIXED_ONE;
    nY1 = (pCabinet->n1 + pCabinet->n3) * FLOPPY144_SITE_FIXED_ONE;

    if(pRunState->player_site_x < nX0)
        nDx = nX0 - pRunState->player_site_x;
    else if(pRunState->player_site_x > nX1)
        nDx = pRunState->player_site_x - nX1;

    if(pRunState->player_site_y < nY0)
        nDy = nY0 - pRunState->player_site_y;
    else if(pRunState->player_site_y > nY1)
        nDy = pRunState->player_site_y - nY1;

    return (uint32_t)(nDx * nDx + nDy * nDy);
}

static uint32_t Floppy144CabinetCodeHash(const char *pszId,uint32_t uSeed)
{
    uint32_t h=2166136261U^uSeed;
    const unsigned char *p=(const unsigned char *)(pszId?pszId:"");
    while(*p){h^=(uint32_t)*p++;h*=16777619U;}
    h^=h>>16;h*=0x7feb352dU;h^=h>>15;h*=0x846ca68bU;h^=h>>16;
    return h;
}

static void Floppy144CabinetMakeDisplayName(
    Floppy144CabinetState *pCabinet,
    const char *pszId
)
{
    uint32_t uRead = 0U;
    uint32_t uWrite = 0U;

    if(pCabinet == NULL)
        return;

    pCabinet->szDisplayName[0] = '\0';

    if(pszId == NULL)
        return;

    while(
        pszId[uRead] != '\0' &&
        uWrite + 1U < FLOPPY144_CABINET_DISPLAY_CAPACITY
    )
    {
        char ch = pszId[uRead++];
        pCabinet->szDisplayName[uWrite++] = ch == '_' ? ' ' : ch;
    }

    pCabinet->szDisplayName[uWrite] = '\0';
}

void Floppy144CabinetReset(
    Floppy144CabinetState *pCabinet
)
{
    if(pCabinet == NULL)
        return;

    memset(pCabinet, 0, sizeof(*pCabinet));
    pCabinet->uCabinetOrdinal = 0xffU;
}

bool Floppy144CabinetOpenNearby(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    Floppy144RoomId eRoom;
    const Floppy144DataRecord *pBest = NULL;
    uint32_t uBestDistance = UINT32_MAX;
    uint32_t uRecordIndex;
    uint32_t uRangeX16;
    uint32_t uRangeSquared;
    int32_t nOrdinal;

    if(pCabinet == NULL || pRunState == NULL)
        return false;

    eRoom = Floppy144SiteRoomAtPosition(
        pRunState->player_site_x,
        pRunState->player_site_y
    );

    if(
        eRoom >= FLOPPY144_ROOM_COUNT ||
        !Floppy144RunStateRoomReconstructed(pRunState, eRoom)
    )
    {
        return false;
    }

    uRangeX16 =
        FLOPPY144_CABINET_INTERACTION_RANGE *
        FLOPPY144_SITE_FIXED_ONE;

    uRangeSquared = uRangeX16 * uRangeX16;

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        uint32_t uDistance;

        if(
            !Floppy144CabinetRecordIsSecure(pRecord) ||
            Floppy144GameDataRoomId(pRecord->pszA) != eRoom
        )
        {
            continue;
        }

        uDistance = Floppy144CabinetDistanceSquared(pRunState, pRecord);

        if(uDistance > uRangeSquared)
            continue;

        if(pBest == NULL || uDistance < uBestDistance)
        {
            pBest = pRecord;
            uBestDistance = uDistance;
        }
    }

    if(pBest == NULL || pBest->pszId == NULL)
        return false;

    nOrdinal = Floppy144CabinetOrdinalForId(pBest->pszId);

    if(nOrdinal < 0 || nOrdinal >= (int32_t)FLOPPY144_SECURE_CABINET_MAX)
        return false;

    Floppy144CabinetReset(pCabinet);

    (void)snprintf(
        pCabinet->szCabinetId,
        sizeof(pCabinet->szCabinetId),
        "%s",
        pBest->pszId
    );

    Floppy144CabinetMakeDisplayName(pCabinet, pBest->pszId);

    pCabinet->uCabinetOrdinal = (uint8_t)nOrdinal;
    pCabinet->uRequiredDigits = (uint8_t)pBest->n5;
    if(pCabinet->uRequiredDigits!=6U&&pCabinet->uRequiredDigits!=8U)pCabinet->uRequiredDigits=6U;

    pCabinet->bInteriorOpen =
        Floppy144RunStateSecureCabinetUnlocked(
            pRunState,
            (uint32_t)pCabinet->uCabinetOrdinal
        );

    if(pCabinet->bInteriorOpen)
    {
        pCabinet->pszStatus = "CABINET ACCESS GRANTED";
    }
    else if(Floppy144CabinetCodeKnown(pCabinet, pRunState))
    {
        pCabinet->pszStatus = "RECOVERED CODE AVAILABLE";
    }
    else
    {
        pCabinet->pszStatus = "ACCESS CODE NOT RECOVERED";
    }

    return true;
}

const char *Floppy144CabinetId(
    const Floppy144CabinetState *pCabinet
)
{
    if(pCabinet == NULL)
        return "";

    return pCabinet->szCabinetId;
}

uint8_t Floppy144CabinetCodeDigits(const Floppy144CabinetState *pCabinet)
{
    return pCabinet!=NULL?pCabinet->uRequiredDigits:0U;
}

uint8_t Floppy144CabinetRequiredDigits(const Floppy144CabinetState *pCabinet)
{
    return Floppy144CabinetCodeDigits(pCabinet);
}

void Floppy144CabinetExpectedCode(
    const Floppy144CabinetState *pCabinet,
    uint32_t uRecoverySeed,
    char *pszCode,
    uint32_t uCapacity
)
{
    uint32_t h,u;bool bAnyNonZero=false;uint8_t digits=Floppy144CabinetCodeDigits(pCabinet);
    if(pszCode==NULL||uCapacity==0U){return;}
    pszCode[0]='\0';
    if(pCabinet==NULL||digits==0U||uCapacity<=(uint32_t)digits)return;
    h=Floppy144CabinetCodeHash(pCabinet->szCabinetId,uRecoverySeed);
    for(u=0U;u<(uint32_t)digits;++u){h=Floppy144CabinetCodeHash(pCabinet->szCabinetId,h^(u*0x9e3779b9U));pszCode[u]=(char)('0'+(h%10U));if(pszCode[u]!='0')bAnyNonZero=true;}
    if(!bAnyNonZero)pszCode[digits-1U]='7';
    pszCode[digits]='\0';
}

bool Floppy144CabinetCodeKnown(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    if(pCabinet == NULL || pRunState == NULL)
        return false;

    if(
        Floppy144RunStateHasCapability(
            pRunState,
            FLOPPY144_CAPABILITY_MASTER_SECURE_CABINET_CODES
        )
    )
    {
        return true;
    }

    if(
        Floppy144CabinetStringEqual(
            pCabinet->szCabinetId,
            "SECURITY_SECURE_CABINET"
        )
    )
    {
        return Floppy144GameDataFactRecorded(
            pRunState,
            "SECURITY_CABINET_CODE"
        );
    }

    return false;
}

bool Floppy144CabinetUnlocked(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        pCabinet->uCabinetOrdinal >= FLOPPY144_SECURE_CABINET_MAX
    )
    {
        return false;
    }

    return Floppy144RunStateSecureCabinetUnlocked(
        pRunState,
        (uint32_t)pCabinet->uCabinetOrdinal
    );
}

bool Floppy144CabinetInteriorOpen(
    const Floppy144CabinetState *pCabinet
)
{
    return pCabinet != NULL && pCabinet->bInteriorOpen;
}

bool Floppy144CabinetDetailOpen(
    const Floppy144CabinetState *pCabinet
)
{
    return pCabinet != NULL && pCabinet->bDetailOpen;
}

bool Floppy144CabinetInputDigit(
    Floppy144CabinetState *pCabinet,
    char chDigit
)
{
    if(
        pCabinet == NULL ||
        pCabinet->bInteriorOpen ||
        chDigit < '0' ||
        chDigit > '9' ||
        pCabinet->uInputLength >= pCabinet->uRequiredDigits ||
        pCabinet->uInputLength >= FLOPPY144_CABINET_CODE_CAPACITY
    )
    {
        return false;
    }

    pCabinet->szInput[pCabinet->uInputLength++] = chDigit;
    pCabinet->szInput[pCabinet->uInputLength] = '\0';
    pCabinet->pszStatus = "CODE ENTRY IN PROGRESS";

    return true;
}

void Floppy144CabinetClearInput(
    Floppy144CabinetState *pCabinet
)
{
    if(pCabinet == NULL)
        return;

    pCabinet->uInputLength = 0U;
    pCabinet->szInput[0] = '\0';
}

bool Floppy144CabinetSubmitCode(
    Floppy144CabinetState *pCabinet,
    Floppy144WorldState *pWorld,
    Floppy144RunState *pRunState
)
{
    Floppy144InteractionId eInteraction=FLOPPY144_INTERACTION_COUNT;char szExpected[FLOPPY144_CABINET_CODE_CAPACITY+1U];bool bMatch;
    if(pCabinet==NULL||pRunState==NULL||pCabinet->bInteriorOpen)return false;
    Floppy144CabinetExpectedCode(pCabinet,pRunState->recovery_seed,szExpected,(uint32_t)sizeof(szExpected));
    bMatch=pCabinet->uRequiredDigits>0U&&pCabinet->uInputLength==pCabinet->uRequiredDigits&&strcmp(pCabinet->szInput,szExpected)==0;
    Floppy144CabinetClearInput(pCabinet);
    if(!bMatch){pCabinet->pszStatus="ACCESS DENIED - INCORRECT CODE";return false;}
    if(!Floppy144RunStateSecureCabinetUnlocked(pRunState,(uint32_t)pCabinet->uCabinetOrdinal))
    {
        if(!Floppy144RunStateUnlockSecureCabinet(pRunState,(uint32_t)pCabinet->uCabinetOrdinal)){pCabinet->pszStatus="ACCESS STATE ERROR";return false;}
    }
    eInteraction=Floppy144GameDataInteractionId(Floppy144CabinetStringEqual(pCabinet->szCabinetId,"SECURITY_SECURE_CABINET")?"I-038":"I-040");
    if(eInteraction<FLOPPY144_INTERACTION_COUNT&&Floppy144InteractionCanRun(pRunState,eInteraction))(void)Floppy144InteractionTryRun(pWorld,pRunState,eInteraction);
    pCabinet->bInteriorOpen=true;pCabinet->bDetailOpen=false;pCabinet->uSelectedContent=0U;pCabinet->pszStatus="CODE ACCEPTED - CABINET UNLOCKED";
    return true;
}

uint32_t Floppy144CabinetVisibleContentCount(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    uint32_t uRecordIndex;
    uint32_t uCount = 0U;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        !pCabinet->bInteriorOpen
    )
    {
        return 0U;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_PHYSICAL_ITEM ||
            !Floppy144CabinetStringEqual(
                pRecord->pszC,
                pCabinet->szCabinetId
            ) ||
            !Floppy144GameDataPhysicalItemRevealed(
                pRunState,
                pRecord->pszId
            )
        )
        {
            continue;
        }

        ++uCount;
    }

    return uCount;
}

const Floppy144DataRecord *Floppy144CabinetVisibleContentAt(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    uint32_t uVisibleIndex
)
{
    uint32_t uRecordIndex;
    uint32_t uVisible = 0U;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        !pCabinet->bInteriorOpen
    )
    {
        return NULL;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uRecordIndex);

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_PHYSICAL_ITEM ||
            !Floppy144CabinetStringEqual(
                pRecord->pszC,
                pCabinet->szCabinetId
            ) ||
            !Floppy144GameDataPhysicalItemRevealed(
                pRunState,
                pRecord->pszId
            )
        )
        {
            continue;
        }

        if(uVisible == uVisibleIndex)
            return pRecord;

        ++uVisible;
    }

    return NULL;
}

void Floppy144CabinetMoveSelection(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    int32_t nDelta
)
{
    uint32_t uCount;
    int32_t nNext;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        !pCabinet->bInteriorOpen ||
        pCabinet->bDetailOpen
    )
    {
        return;
    }

    uCount = Floppy144CabinetVisibleContentCount(pCabinet, pRunState);

    if(uCount == 0U)
    {
        pCabinet->uSelectedContent = 0U;
        return;
    }

    nNext = (int32_t)pCabinet->uSelectedContent + nDelta;

    if(nNext < 0)
        nNext = (int32_t)uCount - 1;
    else if((uint32_t)nNext >= uCount)
        nNext = 0;

    pCabinet->uSelectedContent = (uint32_t)nNext;
}

bool Floppy144CabinetInspectSelected(
    Floppy144CabinetState *pCabinet,
    Floppy144WorldState *pWorld,
    Floppy144RunState *pRunState
)
{
    const Floppy144DataRecord *pItem;
    Floppy144InteractionId eInteraction;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        !pCabinet->bInteriorOpen ||
        pCabinet->bDetailOpen
    )
    {
        return false;
    }

    pItem = Floppy144CabinetVisibleContentAt(
        pCabinet,
        pRunState,
        pCabinet->uSelectedContent
    );

    if(pItem == NULL)
        return false;

    eInteraction = Floppy144InteractionForPhysicalSource(pItem->pszId);

    if(
        eInteraction < FLOPPY144_INTERACTION_COUNT &&
        Floppy144InteractionCanRun(pRunState, eInteraction)
    )
    {
        (void)Floppy144InteractionTryRun(
            pWorld,
            pRunState,
            eInteraction
        );

        pCabinet->pszStatus = "INSPECTION RECORDED";
    }
    else
    {
        pCabinet->pszStatus = "ITEM INSPECTED";
    }

    pCabinet->bDetailOpen = true;
    return true;
}

bool Floppy144CabinetBackspace(
    Floppy144CabinetState *pCabinet
)
{
    if(pCabinet == NULL)
        return false;

    if(pCabinet->bDetailOpen)
    {
        pCabinet->bDetailOpen = false;
        pCabinet->pszStatus = "CABINET INTERIOR";
        return true;
    }

    if(!pCabinet->bInteriorOpen && pCabinet->uInputLength > 0U)
    {
        --pCabinet->uInputLength;
        pCabinet->szInput[pCabinet->uInputLength] = '\0';
        pCabinet->pszStatus = "CODE ENTRY IN PROGRESS";
        return true;
    }

    return false;
}

static void Floppy144CabinetCopyForDisplay(
    char *pszDestination,
    uint32_t uCapacity,
    const char *pszSource,
    uint32_t uMaxCharacters
)
{
    uint32_t uIndex = 0U;

    if(pszDestination == NULL || uCapacity == 0U)
        return;

    pszDestination[0] = '\0';

    if(pszSource == NULL)
        return;

    while(
        pszSource[uIndex] != '\0' &&
        uIndex < uMaxCharacters &&
        uIndex + 1U < uCapacity
    )
    {
        pszDestination[uIndex] = pszSource[uIndex];
        ++uIndex;
    }

    if(
        pszSource[uIndex] != '\0' &&
        uIndex + 4U < uCapacity
    )
    {
        pszDestination[uIndex++] = '.';
        pszDestination[uIndex++] = '.';
        pszDestination[uIndex++] = '.';
    }

    pszDestination[uIndex] = '\0';
}

static void Floppy144CabinetDrawKeypad(
    Floppy144Surface *pSurface,
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    const uint32_t uPanel = FLOPPY144_RGB(17, 27, 23);
    const uint32_t uBorder = FLOPPY144_RGB(70, 111, 86);
    const uint32_t uText = FLOPPY144_RGB(151, 214, 170);
    const uint32_t uMuted = FLOPPY144_RGB(78, 120, 94);
    const uint32_t uBright = FLOPPY144_RGB(196, 238, 203);
    const uint32_t uAmber = FLOPPY144_RGB(206, 164, 77);
    uint32_t uIndex;
    char szDigitCount[48];

    Floppy144DrawText(
        pSurface,
        32U,
        24U,
        "GDR SECURE STORAGE ACCESS",
        2U,
        uBright
    );

    Floppy144DrawText(
        pSurface,
        32U,
        52U,
        pCabinet->szDisplayName,
        1U,
        uText
    );

    Floppy144DrawFillRect(pSurface, 88U, 88U, 464U, 184U, uPanel);
    Floppy144DrawRect(pSurface, 88U, 88U, 464U, 184U, uBorder);

    snprintf(
        szDigitCount,
        sizeof(szDigitCount),
        "REQUIRED ENTRY: %u DIGITS",
        (unsigned)pCabinet->uRequiredDigits
    );

    Floppy144DrawText(pSurface, 118U, 112U, szDigitCount, 1U, uText);

    for(uIndex = 0U; uIndex < FLOPPY144_CABINET_CODE_CAPACITY; ++uIndex)
    {
        uint32_t uX = 118U + uIndex * 48U;
        bool bActive = uIndex < pCabinet->uRequiredDigits;
        bool bFilled = uIndex < pCabinet->uInputLength;
        char szCell[2] = { bFilled ? '*' : ' ', '\0' };

        Floppy144DrawRect(
            pSurface,
            uX,
            144U,
            34U,
            38U,
            bActive ? uBorder : FLOPPY144_RGB(42, 58, 49)
        );

        if(bActive)
        {
            Floppy144DrawText(
                pSurface,
                uX + 14U,
                158U,
                szCell,
                1U,
                bFilled ? uBright : uMuted
            );
        }
    }

    if(Floppy144CabinetCodeKnown(pCabinet,pRunState))
    {
        char szExpected[FLOPPY144_CABINET_CODE_CAPACITY+1U];char szKnown[48];
        Floppy144CabinetExpectedCode(pCabinet,pRunState->recovery_seed,szExpected,(uint32_t)sizeof(szExpected));
        (void)snprintf(szKnown,sizeof(szKnown),"RECOVERED CODE: %s",szExpected);
        Floppy144DrawText(pSurface,118U,204U,szKnown,1U,uText);
    }
    else
    {
        Floppy144DrawText(pSurface,118U,204U,pCabinet->pszStatus!=NULL?pCabinet->pszStatus:"CODE ENTRY REQUIRED",1U,uAmber);
    }

    Floppy144DrawText(
        pSurface,
        118U,
        232U,
        "0-9 ENTER CODE  ENTER SUBMIT  BACKSPACE RETURN",
        1U,
        uMuted
    );
}

static void Floppy144CabinetDrawInterior(
    Floppy144Surface *pSurface,
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    const uint32_t uPanel = FLOPPY144_RGB(20, 26, 24);
    const uint32_t uMetal = FLOPPY144_RGB(71, 82, 76);
    const uint32_t uEdge = FLOPPY144_RGB(115, 132, 122);
    const uint32_t uText = FLOPPY144_RGB(170, 213, 184);
    const uint32_t uMuted = FLOPPY144_RGB(91, 124, 102);
    const uint32_t uBright = FLOPPY144_RGB(216, 239, 220);
    uint32_t uCount;
    uint32_t uIndex;

    Floppy144DrawText(pSurface, 28U, 20U, "CABINET INTERIOR", 2U, uBright);
    Floppy144DrawText(pSurface, 28U, 48U, pCabinet->szDisplayName, 1U, uText);

    /* Front-on reusable metal cabinet body. */
    Floppy144DrawFillRect(pSurface, 42U, 80U, 250U, 224U, uMetal);
    Floppy144DrawRect(pSurface, 42U, 80U, 250U, 224U, uEdge);

    for(uIndex = 1U; uIndex < 4U; ++uIndex)
    {
        uint32_t uY = 80U + (224U * uIndex) / 4U;
        Floppy144DrawFillRect(pSurface, 48U, uY, 238U, 2U, uEdge);
        Floppy144DrawFillRect(pSurface, 150U, uY - 9U, 34U, 4U, uEdge);
    }

    Floppy144DrawText(
        pSurface,
        318U,
        82U,
        "RECOVERED CONTENTS - INSPECTION ONLY",
        1U,
        uMuted
    );

    uCount = Floppy144CabinetVisibleContentCount(pCabinet, pRunState);

    if(uCount == 0U)
    {
        Floppy144DrawText(
            pSurface,
            318U,
            112U,
            "NO RECOVERED CONTENTS",
            1U,
            uMuted
        );
    }
    else
    {
        uint32_t uFirst = 0U;
        uint32_t uVisibleRows = 10U;

        if(pCabinet->uSelectedContent >= uVisibleRows)
            uFirst = pCabinet->uSelectedContent - uVisibleRows + 1U;

        for(
            uIndex = uFirst;
            uIndex < uCount && uIndex < uFirst + uVisibleRows;
            ++uIndex
        )
        {
            const Floppy144DataRecord *pItem =
                Floppy144CabinetVisibleContentAt(
                    pCabinet,
                    pRunState,
                    uIndex
                );

            char szName[44];
            char szLine[48];

            if(pItem == NULL)
                continue;

            Floppy144CabinetCopyForDisplay(
                szName,
                sizeof(szName),
                pItem->pszA,
                34U
            );

            snprintf(
                szLine,
                sizeof(szLine),
                "%c %s",
                uIndex == pCabinet->uSelectedContent ? '>' : ' ',
                szName
            );

            Floppy144DrawText(
                pSurface,
                318U,
                108U + (uIndex - uFirst) * 18U,
                szLine,
                1U,
                uIndex == pCabinet->uSelectedContent ? uBright : uText
            );
        }
    }

    Floppy144DrawText(
        pSurface,
        318U,
        294U,
        "UP/DOWN SELECT  I INSPECT  BACKSPACE SITE",
        1U,
        uMuted
    );

    if(pCabinet->bDetailOpen)
    {
        const Floppy144DataRecord *pItem =
            Floppy144CabinetVisibleContentAt(
                pCabinet,
                pRunState,
                pCabinet->uSelectedContent
            );

        Floppy144DrawFillRect(pSurface, 70U, 74U, 500U, 224U, uPanel);
        Floppy144DrawRect(pSurface, 70U, 74U, 500U, 224U, uEdge);
        Floppy144DrawText(pSurface, 94U, 94U, "RECOVERED PHYSICAL ITEM", 2U, uBright);

        if(pItem != NULL)
        {
            char szId[80];
            char szName[68];
            char szRole[68];

            snprintf(szId, sizeof(szId), "ID: %s", pItem->pszId != NULL ? pItem->pszId : "-");
            snprintf(szName, sizeof(szName), "ITEM: %.52s", pItem->pszA != NULL ? pItem->pszA : "RECOVERED ITEM");
            snprintf(szRole, sizeof(szRole), "ROLE: %.52s", pItem->pszD != NULL ? pItem->pszD : "CONTEXT");

            Floppy144DrawText(pSurface, 94U, 138U, szId, 1U, uText);
            Floppy144DrawText(pSurface, 94U, 160U, szName, 1U, uText);
            Floppy144DrawText(pSurface, 94U, 182U, szRole, 1U, uText);
        }

        Floppy144DrawText(
            pSurface,
            94U,
            226U,
            pCabinet->pszStatus != NULL ? pCabinet->pszStatus : "ITEM INSPECTED",
            1U,
            uBright
        );

        Floppy144DrawText(
            pSurface,
            94U,
            266U,
            "BACKSPACE: CABINET",
            1U,
            uMuted
        );
    }
}

void Floppy144CabinetDraw(
    F144Runtime *pRuntime,
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    Floppy144Surface sSurface;
    const uint32_t uBackground = FLOPPY144_RGB(8, 13, 11);

    if(
        pRuntime == NULL ||
        pCabinet == NULL ||
        pRunState == NULL ||
        pRuntime->backbuffer.data == NULL
    )
    {
        return;
    }

    sSurface.pixels = (uint32_t *)pRuntime->backbuffer.data;
    sSurface.width = pRuntime->backbuffer.width;
    sSurface.height = pRuntime->backbuffer.height;

    Floppy144DrawClear(&sSurface, uBackground);

    if(pCabinet->bInteriorOpen)
        Floppy144CabinetDrawInterior(&sSurface, pCabinet, pRunState);
    else
        Floppy144CabinetDrawKeypad(&sSurface, pCabinet, pRunState);
}
