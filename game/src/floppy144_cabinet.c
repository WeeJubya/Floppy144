/*
 * Floppy//144 - reusable parent-contents view with secure-cabinet access
 */

#include "floppy144_cabinet.h"

#include "floppy144_draw.h"
#include "floppy144_interaction_engine.h"
#include "floppy144_site.h"
#include "floppy144_site_object.h"
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

static const Floppy144DataRecord *Floppy144CabinetParentRecord(
    const char *pszParentId
)
{
    const Floppy144DataRecord *pRecord;

    if(pszParentId == NULL)
        return NULL;

    pRecord =
        Floppy144GameDataFind(
            FLOPPY144_DATA_FURNITURE,
            pszParentId
        );

    if(pRecord != NULL)
        return pRecord;

    return
        Floppy144GameDataFind(
            FLOPPY144_DATA_FIXTURE,
            pszParentId
        );
}

static void Floppy144CabinetSetContainerType(
    Floppy144CabinetState *pCabinet,
    const Floppy144DataRecord *pParent
)
{
    const char *pszType;

    if(pCabinet == NULL)
        return;

    pszType =
        pParent != NULL && pParent->pszB != NULL
            ? pParent->pszB
            : "CONTAINER";

    (void)snprintf(
        pCabinet->szContainerType,
        sizeof(pCabinet->szContainerType),
        "%s",
        pszType
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

static void Floppy144CabinetRoomDisplayName(
    const char *pszRoomId,
    char *pszOutput,
    uint32_t uCapacity
)
{
    const char *pszKnown = NULL;
    uint32_t uRead = 0U;
    uint32_t uWrite = 0U;
    bool bWordStart = true;

    if(pszOutput == NULL || uCapacity == 0U)
        return;

    pszOutput[0] = '\0';

    if(pszRoomId == NULL)
        return;

    if(strcmp(pszRoomId, "RECEPTION") == 0)
        pszKnown = "Reception";
    else if(strcmp(pszRoomId, "CORRIDOR") == 0)
        pszKnown = "Corridor";
    else if(strcmp(pszRoomId, "MAIN_OFFICE") == 0)
        pszKnown = "Main Office";
    else if(strcmp(pszRoomId, "FACILITIES") == 0)
        pszKnown = "Facilities";
    else if(strcmp(pszRoomId, "RECORDS_OFFICE") == 0)
        pszKnown = "Records Office";
    else if(strcmp(pszRoomId, "IT_SUPPORT") == 0)
        pszKnown = "IT Support";
    else if(strcmp(pszRoomId, "STAFF_ROOM") == 0)
        pszKnown = "Staff Room";
    else if(strcmp(pszRoomId, "SECURITY") == 0)
        pszKnown = "Security Office";
    else if(strcmp(pszRoomId, "SERVER_ROOM") == 0)
        pszKnown = "Server Room";
    else if(strcmp(pszRoomId, "SECRETARY_OFFICE") == 0)
        pszKnown = "Secretary's Office";
    else if(strcmp(pszRoomId, "DIRECTOR_OFFICE") == 0)
        pszKnown = "Director's Office";
    else if(strcmp(pszRoomId, "OUTSIDE") == 0)
        pszKnown = "Outside";

    if(pszKnown != NULL)
    {
        (void)snprintf(
            pszOutput,
            uCapacity,
            "%s",
            pszKnown
        );
        return;
    }

    while(
        pszRoomId[uRead] != '\0' &&
        uWrite + 1U < uCapacity
    )
    {
        char ch = pszRoomId[uRead++];

        if(ch == '_')
        {
            pszOutput[uWrite++] = ' ';
            bWordStart = true;
            continue;
        }

        if(ch >= 'A' && ch <= 'Z' && !bWordStart)
            ch = (char)(ch - 'A' + 'a');

        pszOutput[uWrite++] = ch;
        bWordStart = false;
    }

    pszOutput[uWrite] = '\0';
}

static bool Floppy144CabinetMakeDoorDisplayName(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    const char *pszConnectionId
)
{
    const Floppy144DataRecord *pConnection;
    const char *pszFirst;
    const char *pszSecond;
    Floppy144RoomId eCurrentRoom;
    Floppy144RoomId eFirstRoom;
    Floppy144RoomId eSecondRoom;
    char szFirst[32];
    char szSecond[32];

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        pszConnectionId == NULL
    )
    {
        return false;
    }

    pConnection =
        Floppy144GameDataFind(
            FLOPPY144_DATA_CONNECTION,
            pszConnectionId
        );

    if(
        pConnection == NULL ||
        pConnection->pszA == NULL ||
        pConnection->pszB == NULL
    )
    {
        return false;
    }

    pszFirst = pConnection->pszA;
    pszSecond = pConnection->pszB;

    eCurrentRoom =
        Floppy144SiteRoomAtPosition(
            pRunState->player_site_x,
            pRunState->player_site_y
        );

    eFirstRoom =
        Floppy144GameDataRoomId(
            pszFirst
        );

    eSecondRoom =
        Floppy144GameDataRoomId(
            pszSecond
        );

    /*
     * Put the room the player is currently standing in first. The connection
     * ledger's authored order is an implementation detail and should not leak
     * into the Contents screen.
     */
    if(
        eCurrentRoom < FLOPPY144_ROOM_COUNT &&
        eSecondRoom == eCurrentRoom &&
        eFirstRoom != eCurrentRoom
    )
    {
        const char *pszSwap = pszFirst;
        pszFirst = pszSecond;
        pszSecond = pszSwap;
    }

    Floppy144CabinetRoomDisplayName(
        pszFirst,
        szFirst,
        (uint32_t)sizeof(szFirst)
    );

    Floppy144CabinetRoomDisplayName(
        pszSecond,
        szSecond,
        (uint32_t)sizeof(szSecond)
    );

    if(strcmp(pszFirst, pszSecond) == 0)
    {
        (void)snprintf(
            pCabinet->szDisplayName,
            sizeof(pCabinet->szDisplayName),
            "Door within %s",
            szFirst
        );
    }
    else if(strcmp(pszSecond, "OUTSIDE") == 0)
    {
        (void)snprintf(
            pCabinet->szDisplayName,
            sizeof(pCabinet->szDisplayName),
            "Exterior Door - %s",
            szFirst
        );
    }
    else if(strcmp(pszFirst, "OUTSIDE") == 0)
    {
        (void)snprintf(
            pCabinet->szDisplayName,
            sizeof(pCabinet->szDisplayName),
            "Exterior Door - %s",
            szSecond
        );
    }
    else
    {
        (void)snprintf(
            pCabinet->szDisplayName,
            sizeof(pCabinet->szDisplayName),
            "Door between %s and %s",
            szFirst,
            szSecond
        );
    }

    return true;
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
    const char *pszFocusedParentId;
    const Floppy144DataRecord *pBest;
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

    /*
     * Secure storage no longer owns an independent 2U proximity halo.
     * It can be accessed only when the same half-unit parent currently owns
     * the Site label/focus, so a nearby trolley, desk or other parent cannot
     * leak Cabinet Access into its action prompt.
     */
    pszFocusedParentId = Floppy144SiteFocusedParentId(pRunState);
    if(pszFocusedParentId == NULL)
        return false;

    pBest = Floppy144GameDataFind(
        FLOPPY144_DATA_FURNITURE,
        pszFocusedParentId
    );

    if(
        !Floppy144CabinetRecordIsSecure(pBest) ||
        Floppy144GameDataRoomId(pBest->pszA) != eRoom ||
        pBest->pszId == NULL
    )
    {
        return false;
    }

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
    Floppy144CabinetSetContainerType(pCabinet, pBest);
    pCabinet->bSecureContainer = true;

    pCabinet->uCabinetOrdinal = (uint8_t)nOrdinal;
    pCabinet->uRequiredDigits = (uint8_t)pBest->n5;
    if(pCabinet->uRequiredDigits!=6U&&pCabinet->uRequiredDigits!=8U)pCabinet->uRequiredDigits=6U;

    pCabinet->bInteriorOpen =
        Floppy144RunStateSecureCabinetUnlocked(
            pRunState,
            (uint32_t)pCabinet->uCabinetOrdinal
        );

    if(pCabinet->bInteriorOpen)
        pCabinet->pszStatus = "CABINET ACCESS GRANTED";
    else if(Floppy144CabinetCodeKnown(pCabinet, pRunState))
        pCabinet->pszStatus = "RECOVERED CODE AVAILABLE";
    else
        pCabinet->pszStatus = "ACCESS CODE NOT RECOVERED";

    return true;
}

bool Floppy144CabinetOpenParent(
    Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    const char *pszParentId
)
{
    const Floppy144DataRecord *pParent;
    uint32_t uRecordIndex;
    bool bHasVisibleContent = false;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        pszParentId == NULL
    )
    {
        return false;
    }

    pParent =
        Floppy144CabinetParentRecord(
            pszParentId
        );

    if(pParent == NULL || Floppy144CabinetRecordIsSecure(pParent))
    {
        return false;
    }

    for(
        uRecordIndex = 0U;
        uRecordIndex < Floppy144GameDataRecordCount();
        ++uRecordIndex
    )
    {
        const Floppy144DataRecord *pItem =
            Floppy144GameDataRecordAt(uRecordIndex);

        if(
            pItem != NULL &&
            pItem->eKind == FLOPPY144_DATA_PHYSICAL_ITEM &&
            Floppy144CabinetStringEqual(
                pItem->pszC,
                pszParentId
            ) &&
            Floppy144SitePhysicalItemVisible(
                pRunState,
                pItem
            )
        )
        {
            bHasVisibleContent = true;
            break;
        }
    }

    /*
     * Storage remains inspectable even when reconstruction has recovered no
     * visible children yet. An empty cupboard/bookcase is still a real place
     * the player can examine, and the Contents screen already has an explicit
     * empty-state presentation.
     */
    if(
        !bHasVisibleContent &&
        !(
            pParent->pszB != NULL &&
            (
                strcmp(pParent->pszB, "BOOKCASE") == 0 ||
                strcmp(pParent->pszB, "NONSECURE_CABINET") == 0 ||
                strcmp(pParent->pszB, "SHELVING_FULL") == 0
            )
        )
    )
    {
        return false;
    }

    Floppy144CabinetReset(pCabinet);

    (void)snprintf(
        pCabinet->szCabinetId,
        sizeof(pCabinet->szCabinetId),
        "%s",
        pszParentId
    );

    Floppy144CabinetMakeDisplayName(
        pCabinet,
        pszParentId
    );

    if(
        pParent->pszB != NULL &&
        strcmp(pParent->pszB, "DOOR") == 0
    )
    {
        (void)Floppy144CabinetMakeDoorDisplayName(
            pCabinet,
            pRunState,
            pszParentId
        );
    }

    Floppy144CabinetSetContainerType(
        pCabinet,
        pParent
    );

    pCabinet->bSecureContainer = false;
    pCabinet->bInteriorOpen = true;
    pCabinet->bDetailOpen = false;
    pCabinet->uSelectedContent = 0U;
    pCabinet->pszStatus =
        bHasVisibleContent
            ? "RECOVERED CONTENTS"
            : "EMPTY STORAGE";

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

    /*
     * A revealed physical record may identify the cabinet holding the other
     * half of a comparison before the later master-code register is recovered.
     * Recover only that specific cabinet's deterministic code, rather than
     * granting the site-wide code capability early.
     */
    {
        uint32_t uRecordIndex;

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

            return true;
        }
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

static bool Floppy144CabinetPhysicalItemRevealControlled(
    const char *pszPhysicalItemId
)
{
    uint32_t uRecordIndex;

    if(pszPhysicalItemId == NULL)
        return false;

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
            (
                pRecord->eKind != FLOPPY144_DATA_TRIGGER_EFFECT &&
                pRecord->eKind != FLOPPY144_DATA_INTERACTION_EFFECT
            )
        )
        {
            continue;
        }

        if(
            Floppy144CabinetStringEqual(
                pRecord->pszA,
                "REVEAL_PHYSICAL_ITEM"
            ) &&
            Floppy144CabinetStringEqual(
                pRecord->pszB,
                pszPhysicalItemId
            )
        )
        {
            return true;
        }
    }

    return false;
}

/*
 * Contents order is deterministic for one recovery seed but varies between
 * playthroughs. Ordinary contextual items and explicitly revealed evidence
 * remain in separate bands, so newly revealed evidence is appended after the
 * ordinary clutter instead of reordering everything the player already saw.
 */
static uint32_t Floppy144CabinetVisibleContentLimit(
    const Floppy144CabinetState *pCabinet
)
{
    /*
     * Chairs can plausibly retain a dropped or wedged object or two, but they
     * must never behave like miniature cupboards. The authored data is also
     * regression-checked against this contract.
     */
    if(
        pCabinet != NULL &&
        strcmp(
            pCabinet->szContainerType,
            "CHAIR"
        ) == 0
    )
    {
        return 2U;
    }

    return UINT32_MAX;
}

uint32_t Floppy144CabinetVisibleContentCount(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    uint32_t uRecordIndex;
    uint32_t uCount = 0U;
    uint32_t uLimit;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        !pCabinet->bInteriorOpen
    )
    {
        return 0U;
    }

    uLimit =
        Floppy144CabinetVisibleContentLimit(
            pCabinet
        );

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
            !Floppy144SitePhysicalItemVisible(
                pRunState,
                pRecord
            )
        )
        {
            continue;
        }

        ++uCount;

        if(uCount >= uLimit)
            break;
    }

    return uCount;
}

static uint32_t Floppy144CabinetContentShuffleKey(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    const Floppy144DataRecord *pRecord
)
{
    uint32_t h;

    if(pCabinet==NULL||pRunState==NULL||pRecord==NULL)return 0U;

    h=Floppy144CabinetCodeHash(
        pCabinet->szCabinetId,
        pRunState->recovery_seed^0x9e3779b9U
    );

    return
        Floppy144CabinetCodeHash(
            pRecord->pszId,
            h
        );
}

static bool Floppy144CabinetContentComesBefore(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    const Floppy144DataRecord *pA,
    const Floppy144DataRecord *pB
)
{
    uint32_t a,b;
    int n;

    if(pA==NULL)return false;
    if(pB==NULL)return true;

    a=Floppy144CabinetContentShuffleKey(pCabinet,pRunState,pA);
    b=Floppy144CabinetContentShuffleKey(pCabinet,pRunState,pB);

    if(a!=b)return a<b;

    n=strcmp(
        pA->pszId!=NULL?pA->pszId:"",
        pB->pszId!=NULL?pB->pszId:""
    );

    return n<0;
}

const Floppy144DataRecord *Floppy144CabinetVisibleContentAt(
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState,
    uint32_t uVisibleIndex
)
{
    uint32_t uPass;
    uint32_t uLimit;
    uint32_t uBandOffset=0U;

    if(
        pCabinet == NULL ||
        pRunState == NULL ||
        !pCabinet->bInteriorOpen
    )
    {
        return NULL;
    }

    uLimit=Floppy144CabinetVisibleContentLimit(pCabinet);
    if(uVisibleIndex>=uLimit)return NULL;

    for(uPass=0U;uPass<2U;++uPass)
    {
        bool bRecoveredPass=uPass!=0U;
        uint32_t uBandCount=0U;
        uint32_t uRankWanted;
        uint32_t uRecordIndex;
        const Floppy144DataRecord *pWinner=NULL;

        for(uRecordIndex=0U;uRecordIndex<Floppy144GameDataRecordCount();++uRecordIndex)
        {
            const Floppy144DataRecord *pRecord=
                Floppy144GameDataRecordAt(uRecordIndex);

            if(
                pRecord==NULL ||
                pRecord->eKind!=FLOPPY144_DATA_PHYSICAL_ITEM ||
                !Floppy144CabinetStringEqual(
                    pRecord->pszC,
                    pCabinet->szCabinetId
                ) ||
                !Floppy144SitePhysicalItemVisible(
                    pRunState,
                    pRecord
                ) ||
                Floppy144CabinetPhysicalItemRevealControlled(
                    pRecord->pszId
                )!=bRecoveredPass
            )
            {
                continue;
            }

            ++uBandCount;
        }

        if(uVisibleIndex>=uBandOffset+uBandCount)
        {
            uBandOffset+=uBandCount;
            continue;
        }

        uRankWanted=uVisibleIndex-uBandOffset;

        while(true)
        {
            const Floppy144DataRecord *pNext=NULL;

            for(uRecordIndex=0U;uRecordIndex<Floppy144GameDataRecordCount();++uRecordIndex)
            {
                const Floppy144DataRecord *pRecord=
                    Floppy144GameDataRecordAt(uRecordIndex);

                if(
                    pRecord==NULL ||
                    pRecord->eKind!=FLOPPY144_DATA_PHYSICAL_ITEM ||
                    !Floppy144CabinetStringEqual(
                        pRecord->pszC,
                        pCabinet->szCabinetId
                    ) ||
                    !Floppy144SitePhysicalItemVisible(
                        pRunState,
                        pRecord
                    ) ||
                    Floppy144CabinetPhysicalItemRevealControlled(
                        pRecord->pszId
                    )!=bRecoveredPass
                )
                {
                    continue;
                }

                if(
                    pWinner!=NULL &&
                    !Floppy144CabinetContentComesBefore(
                        pCabinet,
                        pRunState,
                        pWinner,
                        pRecord
                    )
                )
                {
                    continue;
                }

                if(
                    pNext==NULL ||
                    Floppy144CabinetContentComesBefore(
                        pCabinet,
                        pRunState,
                        pRecord,
                        pNext
                    )
                )
                {
                    pNext=pRecord;
                }
            }

            if(pNext==NULL)return NULL;
            pWinner=pNext;

            if(uRankWanted==0U)return pWinner;
            --uRankWanted;
        }
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

    if(eInteraction < FLOPPY144_INTERACTION_COUNT)
    {
        bool bNotebookUpdated =
            Floppy144RunStateInteractionCompleted(
                pRunState,
                eInteraction
            );

        if(
            !bNotebookUpdated &&
            Floppy144InteractionCanRun(
                pRunState,
                eInteraction
            )
        )
        {
            bNotebookUpdated =
                Floppy144InteractionTryRun(
                    pWorld,
                    pRunState,
                    eInteraction
                );
        }

        pCabinet->pszStatus =
            bNotebookUpdated
                ? "ITEM INSPECTED - NOTEBOOK UPDATED"
                : "ITEM INSPECTED";
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
        pCabinet->pszStatus =
            pCabinet->bSecureContainer
                ? "CABINET INTERIOR"
                : "RECOVERED CONTENTS";
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

static bool Floppy144CabinetTypeContains(
    const Floppy144CabinetState *pCabinet,
    const char *pszToken
)
{
    return
        pCabinet != NULL &&
        pszToken != NULL &&
        strstr(
            pCabinet->szContainerType,
            pszToken
        ) != NULL;
}

static void Floppy144CabinetDrawChairBody(
    Floppy144Surface *pSurface,
    uint32_t uBody,
    uint32_t uEdge
)
{
    uint32_t uIndex;

    if(pSurface == NULL)
        return;

    /*
     * Open-backed office chair. The negative space between the back slats,
     * broad seat and narrow legs makes it read as seating instead of storage.
     */
    Floppy144DrawFillRect(pSurface, 88U, 76U, 12U, 92U, uBody);
    Floppy144DrawFillRect(pSurface, 234U, 76U, 12U, 92U, uBody);
    Floppy144DrawFillRect(pSurface, 88U, 76U, 158U, 14U, uBody);
    Floppy144DrawFillRect(pSurface, 88U, 154U, 158U, 14U, uBody);
    Floppy144DrawRect(pSurface, 88U, 76U, 158U, 92U, uEdge);

    for(uIndex = 0U; uIndex < 4U; ++uIndex)
    {
        uint32_t uX = 112U + uIndex * 30U;

        Floppy144DrawFillRect(pSurface, uX, 96U, 10U, 52U, uBody);
        Floppy144DrawRect(pSurface, uX, 96U, 10U, 52U, uEdge);
    }

    Floppy144DrawFillRect(pSurface, 78U, 172U, 178U, 38U, uBody);
    Floppy144DrawRect(pSurface, 78U, 172U, 178U, 38U, uEdge);
    Floppy144DrawFillRect(pSurface, 88U, 210U, 158U, 10U, uBody);
    Floppy144DrawRect(pSurface, 88U, 210U, 158U, 10U, uEdge);

    Floppy144DrawFillRect(pSurface, 94U, 220U, 14U, 78U, uBody);
    Floppy144DrawFillRect(pSurface, 226U, 220U, 14U, 78U, uBody);
    Floppy144DrawRect(pSurface, 94U, 220U, 14U, 78U, uEdge);
    Floppy144DrawRect(pSurface, 226U, 220U, 14U, 78U, uEdge);
    Floppy144DrawFillRect(pSurface, 118U, 220U, 10U, 64U, uBody);
    Floppy144DrawFillRect(pSurface, 206U, 220U, 10U, 64U, uBody);
}

static bool Floppy144CabinetTypeIs(
    const Floppy144CabinetState *pCabinet,
    const char *pszType
)
{
    return
        pCabinet!=NULL &&
        pszType!=NULL &&
        strcmp(pCabinet->szContainerType,pszType)==0;
}

static void Floppy144CabinetDrawContainerBody(
    Floppy144Surface *pSurface,
    const Floppy144CabinetState *pCabinet,
    uint32_t uBody,
    uint32_t uEdge
)
{
    uint32_t uIndex;

    if(pSurface==NULL||pCabinet==NULL)return;

    if(Floppy144CabinetTypeIs(pCabinet,"CHAIR"))
    {
        Floppy144CabinetDrawChairBody(pSurface,uBody,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"STANDARD_DESK"))
    {
        /* Top-down desk with drawer blocks and a generous working surface. */
        Floppy144DrawFillRect(pSurface,42U,104U,250U,118U,uBody);
        Floppy144DrawRect(pSurface,42U,104U,250U,118U,uEdge);
        Floppy144DrawRect(pSurface,54U,116U,54U,94U,uEdge);
        Floppy144DrawRect(pSurface,226U,116U,54U,94U,uEdge);
        Floppy144DrawFillRect(pSurface,116U,210U,102U,10U,uEdge);
        return;
    }

    if(
        Floppy144CabinetTypeIs(pCabinet,"GDR_TERMINAL") ||
        Floppy144CabinetTypeIs(pCabinet,"IT_TERMINAL")
    )
    {
        /* Terminal desk: monitor at rear, keyboard shelf and side work area. */
        Floppy144DrawFillRect(pSurface,42U,102U,250U,124U,uBody);
        Floppy144DrawRect(pSurface,42U,102U,250U,124U,uEdge);
        Floppy144DrawRect(pSurface,76U,114U,118U,66U,uEdge);
        Floppy144DrawFillRect(pSurface,92U,124U,86U,42U,uEdge);
        Floppy144DrawRect(pSurface,92U,188U,86U,22U,uEdge);
        Floppy144DrawRect(pSurface,210U,118U,62U,92U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"WORKBENCH"))
    {
        Floppy144DrawFillRect(pSurface,36U,106U,264U,106U,uBody);
        Floppy144DrawRect(pSurface,36U,106U,264U,106U,uEdge);
        Floppy144DrawRect(pSurface,48U,118U,240U,38U,uEdge);
        for(uIndex=0U;uIndex<4U;++uIndex)
            Floppy144DrawRect(pSurface,52U+uIndex*60U,166U,48U,34U,uEdge);
        return;
    }

    if(
        Floppy144CabinetTypeIs(pCabinet,"BOARDROOM_TABLE") ||
        Floppy144CabinetTypeIs(pCabinet,"DINING_TABLE")
    )
    {
        Floppy144DrawFillRect(pSurface,52U,108U,230U,112U,uBody);
        Floppy144DrawRect(pSurface,52U,108U,230U,112U,uEdge);
        Floppy144DrawFillRect(pSurface,66U,120U,202U,88U,uBody);
        Floppy144DrawRect(pSurface,66U,120U,202U,88U,uEdge);
        return;
    }

    if(
        Floppy144CabinetTypeIs(pCabinet,"COFFEE_TABLE") ||
        Floppy144CabinetTypeIs(pCabinet,"RECEPTION_TABLE")
    )
    {
        Floppy144DrawFillRect(pSurface,74U,126U,186U,80U,uBody);
        Floppy144DrawRect(pSurface,74U,126U,186U,80U,uEdge);
        Floppy144DrawRect(pSurface,88U,140U,158U,52U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"WORKTOP"))
    {
        Floppy144DrawFillRect(pSurface,36U,118U,264U,70U,uBody);
        Floppy144DrawRect(pSurface,36U,118U,264U,70U,uEdge);
        for(uIndex=0U;uIndex<4U;++uIndex)
            Floppy144DrawRect(pSurface,42U+uIndex*64U,194U,54U,76U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"SINK"))
    {
        Floppy144DrawFillRect(pSurface,70U,110U,194U,116U,uBody);
        Floppy144DrawRect(pSurface,70U,110U,194U,116U,uEdge);
        Floppy144DrawRect(pSurface,94U,132U,146U,70U,uEdge);
        Floppy144DrawFillRect(pSurface,158U,100U,18U,28U,uEdge);
        Floppy144DrawFillRect(pSurface,170U,98U,34U,8U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"COFFEE_MAKER"))
    {
        Floppy144DrawFillRect(pSurface,102U,92U,130U,176U,uBody);
        Floppy144DrawRect(pSurface,102U,92U,130U,176U,uEdge);
        Floppy144DrawRect(pSurface,122U,112U,90U,48U,uEdge);
        Floppy144DrawRect(pSurface,132U,174U,70U,64U,uEdge);
        Floppy144DrawFillRect(pSurface,116U,246U,102U,10U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"SOFA"))
    {
        Floppy144DrawFillRect(pSurface,50U,118U,234U,118U,uBody);
        Floppy144DrawRect(pSurface,50U,118U,234U,118U,uEdge);
        Floppy144DrawRect(pSurface,64U,132U,206U,48U,uEdge);
        Floppy144DrawRect(pSurface,64U,184U,98U,38U,uEdge);
        Floppy144DrawRect(pSurface,172U,184U,98U,38U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"FRIDGE"))
    {
        Floppy144DrawFillRect(pSurface,76U,82U,172U,216U,uBody);
        Floppy144DrawRect(pSurface,76U,82U,172U,216U,uEdge);
        Floppy144DrawFillRect(pSurface,82U,152U,160U,2U,uEdge);
        Floppy144DrawRect(pSurface,94U,96U,134U,44U,uEdge);
        Floppy144DrawRect(pSurface,94U,166U,134U,116U,uEdge);
        Floppy144DrawFillRect(pSurface,222U,112U,8U,28U,uEdge);
        Floppy144DrawFillRect(pSurface,222U,174U,8U,54U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"TROLLEY"))
    {
        Floppy144DrawRect(pSurface,54U,98U,226U,164U,uEdge);
        for(uIndex=0U;uIndex<3U;++uIndex)
        {
            uint32_t y=108U+uIndex*52U;
            Floppy144DrawFillRect(pSurface,62U,y,210U,34U,uBody);
            Floppy144DrawRect(pSurface,62U,y,210U,34U,uEdge);
        }
        Floppy144DrawFillRect(pSurface,74U,270U,28U,10U,uEdge);
        Floppy144DrawFillRect(pSurface,232U,270U,28U,10U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"SERVER"))
    {
        Floppy144DrawFillRect(pSurface,78U,80U,168U,224U,uBody);
        Floppy144DrawRect(pSurface,78U,80U,168U,224U,uEdge);
        for(uIndex=0U;uIndex<8U;++uIndex)
        {
            uint32_t y=92U+uIndex*25U;
            Floppy144DrawRect(pSurface,92U,y,140U,15U,uEdge);
        }
        return;
    }

    if(
        Floppy144CabinetTypeIs(pCabinet,"SHELVING") ||
        Floppy144CabinetTypeIs(pCabinet,"SHELVING_FULL") ||
        Floppy144CabinetTypeIs(pCabinet,"BOOKCASE")
    )
    {
        Floppy144DrawRect(pSurface,42U,80U,250U,224U,uEdge);
        Floppy144DrawFillRect(pSurface,42U,80U,8U,224U,uBody);
        Floppy144DrawFillRect(pSurface,284U,80U,8U,224U,uBody);
        for(uIndex=0U;uIndex<4U;++uIndex)
        {
            uint32_t y=92U+uIndex*54U;
            Floppy144DrawFillRect(pSurface,50U,y+38U,234U,6U,uBody);
            Floppy144DrawRect(pSurface,50U,y+38U,234U,6U,uEdge);
        }
        return;
    }

    if(
        Floppy144CabinetTypeIs(pCabinet,"NONSECURE_CABINET") ||
        Floppy144CabinetTypeIs(pCabinet,"SECURE_CABINET")
    )
    {
        Floppy144DrawFillRect(pSurface,54U,80U,226U,224U,uBody);
        Floppy144DrawRect(pSurface,54U,80U,226U,224U,uEdge);
        Floppy144DrawFillRect(pSurface,166U,84U,2U,216U,uEdge);
        for(uIndex=1U;uIndex<4U;++uIndex)
        {
            uint32_t y=80U+(224U*uIndex)/4U;
            Floppy144DrawFillRect(pSurface,60U,y,214U,2U,uEdge);
        }
        Floppy144DrawFillRect(pSurface,150U,180U,10U,5U,uEdge);
        Floppy144DrawFillRect(pSurface,174U,180U,10U,5U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"PATCH_PANEL"))
    {
        Floppy144DrawFillRect(pSurface,52U,118U,230U,96U,uBody);
        Floppy144DrawRect(pSurface,52U,118U,230U,96U,uEdge);
        for(uIndex=0U;uIndex<8U;++uIndex)
            Floppy144DrawRect(pSurface,68U+uIndex*25U,142U,16U,22U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"MONITOR_BANK"))
    {
        Floppy144DrawFillRect(pSurface,42U,96U,250U,148U,uBody);
        Floppy144DrawRect(pSurface,42U,96U,250U,148U,uEdge);
        for(uIndex=0U;uIndex<3U;++uIndex)
            Floppy144DrawRect(pSurface,56U+uIndex*78U,116U,66U,92U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"CABLE_RISER"))
    {
        Floppy144DrawFillRect(pSurface,78U,74U,178U,232U,uBody);
        Floppy144DrawRect(pSurface,78U,74U,178U,232U,uEdge);
        for(uIndex=0U;uIndex<5U;++uIndex)
            Floppy144DrawFillRect(pSurface,98U+uIndex*28U,90U,8U,194U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"DOOR"))
    {
        Floppy144DrawFillRect(pSurface,82U,76U,164U,232U,uBody);
        Floppy144DrawRect(pSurface,82U,76U,164U,232U,uEdge);
        Floppy144DrawRect(pSurface,112U,108U,104U,42U,uEdge);
        Floppy144DrawFillRect(pSurface,214U,194U,10U,10U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"WINDOW"))
    {
        Floppy144DrawRect(pSurface,54U,92U,226U,164U,uEdge);
        Floppy144DrawFillRect(pSurface,164U,96U,6U,156U,uBody);
        Floppy144DrawFillRect(pSurface,58U,170U,218U,6U,uBody);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"PARTITION_WALL"))
    {
        Floppy144DrawFillRect(pSurface,54U,92U,226U,176U,uBody);
        Floppy144DrawRect(pSurface,54U,92U,226U,176U,uEdge);
        for(uIndex=1U;uIndex<3U;++uIndex)
            Floppy144DrawFillRect(pSurface,54U+uIndex*75U,96U,2U,168U,uEdge);
        return;
    }

    if(
        Floppy144CabinetTypeIs(pCabinet,"NOTICEBOARD") ||
        Floppy144CabinetTypeIs(pCabinet,"SITE_DIRECTORY")
    )
    {
        Floppy144DrawFillRect(pSurface,52U,94U,230U,164U,uBody);
        Floppy144DrawRect(pSurface,52U,94U,230U,164U,uEdge);
        Floppy144DrawRect(pSurface,70U,112U,194U,128U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"SUPPRESSION_PANEL"))
    {
        Floppy144DrawFillRect(pSurface,84U,94U,166U,176U,uBody);
        Floppy144DrawRect(pSurface,84U,94U,166U,176U,uEdge);
        Floppy144DrawRect(pSurface,104U,116U,126U,54U,uEdge);
        for(uIndex=0U;uIndex<4U;++uIndex)
            Floppy144DrawRect(pSurface,106U+uIndex*30U,192U,18U,18U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"FIRST_AID_KIT"))
    {
        Floppy144DrawFillRect(pSurface,92U,104U,150U,150U,uBody);
        Floppy144DrawRect(pSurface,92U,104U,150U,150U,uEdge);
        Floppy144DrawFillRect(pSurface,155U,130U,24U,96U,uEdge);
        Floppy144DrawFillRect(pSurface,119U,166U,96U,24U,uEdge);
        return;
    }

    if(Floppy144CabinetTypeIs(pCabinet,"KEY_CABINET"))
    {
        Floppy144DrawFillRect(pSurface,78U,90U,178U,184U,uBody);
        Floppy144DrawRect(pSurface,78U,90U,178U,184U,uEdge);
        for(uIndex=0U;uIndex<4U;++uIndex)
            Floppy144DrawFillRect(pSurface,104U+uIndex*38U,126U,4U,92U,uEdge);
        return;
    }

    /*
     * Fallback for a future generated parent type: still a distinct, bounded
     * inspection object rather than an unframed marker cloud.
     */
    Floppy144DrawFillRect(pSurface,54U,92U,226U,176U,uBody);
    Floppy144DrawRect(pSurface,54U,92U,226U,176U,uEdge);
}

static void Floppy144CabinetContentMarkerRegion(
    const Floppy144CabinetState *pCabinet,
    uint32_t *pX,
    uint32_t *pY,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pMaximumColumns
)
{
    if(pX==NULL||pY==NULL||pWidth==NULL||pHeight==NULL||pMaximumColumns==NULL)return;

    *pX=70U;*pY=110U;*pWidth=194U;*pHeight=132U;*pMaximumColumns=5U;

    if(Floppy144CabinetTypeIs(pCabinet,"CHAIR"))
    { *pX=96U;*pY=176U;*pWidth=142U;*pHeight=28U;*pMaximumColumns=2U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"STANDARD_DESK"))
    { *pX=116U;*pY=116U;*pWidth=102U;*pHeight=84U;*pMaximumColumns=5U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"GDR_TERMINAL")||Floppy144CabinetTypeIs(pCabinet,"IT_TERMINAL"))
    { *pX=204U;*pY=118U;*pWidth=70U;*pHeight=92U;*pMaximumColumns=3U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"WORKBENCH"))
    { *pX=48U;*pY=118U;*pWidth=240U;*pHeight=82U;*pMaximumColumns=7U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"BOARDROOM_TABLE")||Floppy144CabinetTypeIs(pCabinet,"DINING_TABLE"))
    { *pX=72U;*pY=126U;*pWidth=190U;*pHeight=76U;*pMaximumColumns=6U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"COFFEE_TABLE")||Floppy144CabinetTypeIs(pCabinet,"RECEPTION_TABLE"))
    { *pX=88U;*pY=140U;*pWidth=158U;*pHeight=52U;*pMaximumColumns=5U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"WORKTOP"))
    { *pX=46U;*pY=128U;*pWidth=244U;*pHeight=48U;*pMaximumColumns=7U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"SINK"))
    { *pX=102U;*pY=140U;*pWidth=130U;*pHeight=54U;*pMaximumColumns=4U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"COFFEE_MAKER"))
    { *pX=122U;*pY=176U;*pWidth=90U;*pHeight=54U;*pMaximumColumns=3U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"SOFA"))
    { *pX=70U;*pY=188U;*pWidth=194U;*pHeight=30U;*pMaximumColumns=6U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"FRIDGE"))
    { *pX=98U;*pY=170U;*pWidth=126U;*pHeight=104U;*pMaximumColumns=4U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"TROLLEY"))
    { *pX=68U;*pY=112U;*pWidth=198U;*pHeight=136U;*pMaximumColumns=6U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"SERVER"))
    { *pX=100U;*pY=96U;*pWidth=124U;*pHeight=190U;*pMaximumColumns=4U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"SHELVING")||Floppy144CabinetTypeIs(pCabinet,"SHELVING_FULL")||Floppy144CabinetTypeIs(pCabinet,"BOOKCASE"))
    { *pX=58U;*pY=102U;*pWidth=218U;*pHeight=180U;*pMaximumColumns=6U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"NONSECURE_CABINET")||Floppy144CabinetTypeIs(pCabinet,"SECURE_CABINET"))
    { *pX=68U;*pY=96U;*pWidth=198U;*pHeight=190U;*pMaximumColumns=5U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"PATCH_PANEL"))
    { *pX=64U;*pY=168U;*pWidth=206U;*pHeight=34U;*pMaximumColumns=8U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"MONITOR_BANK"))
    { *pX=58U;*pY=214U;*pWidth=218U;*pHeight=22U;*pMaximumColumns=8U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"CABLE_RISER"))
    { *pX=112U;*pY=94U;*pWidth=110U;*pHeight=184U;*pMaximumColumns=4U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"DOOR"))
    { *pX=112U;*pY=108U;*pWidth=104U;*pHeight=42U;*pMaximumColumns=3U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"WINDOW"))
    { *pX=68U;*pY=110U;*pWidth=198U;*pHeight=124U;*pMaximumColumns=6U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"PARTITION_WALL"))
    { *pX=70U;*pY=112U;*pWidth=194U;*pHeight=136U;*pMaximumColumns=6U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"NOTICEBOARD")||Floppy144CabinetTypeIs(pCabinet,"SITE_DIRECTORY"))
    { *pX=76U;*pY=118U;*pWidth=182U;*pHeight=116U;*pMaximumColumns=5U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"SUPPRESSION_PANEL"))
    { *pX=104U;*pY=116U;*pWidth=126U;*pHeight=132U;*pMaximumColumns=4U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"FIRST_AID_KIT"))
    { *pX=108U;*pY=118U;*pWidth=118U;*pHeight=120U;*pMaximumColumns=4U; }
    else if(Floppy144CabinetTypeIs(pCabinet,"KEY_CABINET"))
    { *pX=94U;*pY=108U;*pWidth=146U;*pHeight=144U;*pMaximumColumns=4U; }
}

static void Floppy144CabinetDrawContentMarkers(
    Floppy144Surface *pSurface,
    const Floppy144CabinetState *pCabinet,
    uint32_t uCount,
    uint32_t uSelected,
    uint32_t uEdge,
    uint32_t uBright
)
{
    uint32_t uRegionX=0U,uRegionY=0U,uRegionWidth=0U,uRegionHeight=0U;
    uint32_t uMaximumColumns=3U;
    uint32_t uColumns,uRows,uMarkerWidth,uMarkerHeight,uGapX,uGapY;
    uint32_t uGridWidth,uGridHeight,uBaseX,uBaseY,uIndex;

    if(pSurface==NULL||pCabinet==NULL||uCount==0U)return;

    Floppy144CabinetContentMarkerRegion(
        pCabinet,
        &uRegionX,
        &uRegionY,
        &uRegionWidth,
        &uRegionHeight,
        &uMaximumColumns
    );

    /*
     * Shelving and bookcases use physical shelf slots rather than the generic
     * centred grid. The bottom edge of every marker rests on one of the four
     * drawn shelf slabs, while each shelf's contents are centred independently.
     *
     * Round-robin shelf assignment spreads dense containers through the whole
     * unit instead of filling one floating row in the middle.
     */
    if(
        Floppy144CabinetTypeIs(pCabinet,"SHELVING") ||
        Floppy144CabinetTypeIs(pCabinet,"SHELVING_FULL") ||
        Floppy144CabinetTypeIs(pCabinet,"BOOKCASE")
    )
    {
        static const uint32_t auShelfY[4]={130U,184U,238U,292U};
        const uint32_t uShelfCount=4U;
        const uint32_t uShelfLeft=58U;
        const uint32_t uShelfWidth=218U;
        const uint32_t uShelfGap=8U;
        const uint32_t uShelfMarkerHeight=10U;

        for(uIndex=0U;uIndex<uCount;++uIndex)
        {
            uint32_t uShelf=uIndex%uShelfCount;
            uint32_t uSlot=uIndex/uShelfCount;
            uint32_t uItemsOnShelf=
                uCount>uShelf
                    ? 1U+(uCount-1U-uShelf)/uShelfCount
                    : 0U;
            uint32_t uShelfMarkerWidth=14U;
            uint32_t uGroupWidth;
            uint32_t uShelfBaseX;
            uint32_t uX;
            uint32_t uY;
            uint32_t uMarker=uIndex==uSelected?uBright:uEdge;

            if(
                uItemsOnShelf*uShelfMarkerWidth+
                (uItemsOnShelf-1U)*uShelfGap>
                uShelfWidth
            )
            {
                uShelfMarkerWidth=
                    (
                        uShelfWidth-
                        (uItemsOnShelf-1U)*uShelfGap
                    )/
                    uItemsOnShelf;
            }

            if(uShelfMarkerWidth<4U)uShelfMarkerWidth=4U;

            uGroupWidth=
                uItemsOnShelf*uShelfMarkerWidth+
                (uItemsOnShelf-1U)*uShelfGap;

            uShelfBaseX=
                uShelfLeft+
                (
                    uShelfWidth>uGroupWidth
                        ? (uShelfWidth-uGroupWidth)/2U
                        : 0U
                );

            uX=
                uShelfBaseX+
                uSlot*(uShelfMarkerWidth+uShelfGap);

            uY=
                auShelfY[uShelf]-
                uShelfMarkerHeight;

            Floppy144DrawFillRect(
                pSurface,
                uX,
                uY,
                uShelfMarkerWidth,
                uShelfMarkerHeight,
                uMarker
            );
        }

        return;
    }

    /*
     * Fit every visible PI into the authored silhouette. Marker dimensions
     * shrink only when necessary; this avoids the old nine-item ceiling and
     * keeps dense desks/cupboards readable without drawing outside furniture.
     */
    uColumns=uCount<uMaximumColumns?uCount:uMaximumColumns;
    if(uColumns==0U)uColumns=1U;
    uRows=(uCount+uColumns-1U)/uColumns;

    uGapX=uCount>15U?3U:6U;
    uGapY=uCount>15U?3U:6U;

    uMarkerWidth=14U;
    uMarkerHeight=10U;

    if(
        uColumns*uMarkerWidth+(uColumns-1U)*uGapX>uRegionWidth
    )
    {
        uMarkerWidth=
            (uRegionWidth-(uColumns-1U)*uGapX)/uColumns;
    }

    if(
        uRows*uMarkerHeight+(uRows-1U)*uGapY>uRegionHeight
    )
    {
        uMarkerHeight=
            (uRegionHeight-(uRows-1U)*uGapY)/uRows;
    }

    if(uMarkerWidth<4U)uMarkerWidth=4U;
    if(uMarkerHeight<4U)uMarkerHeight=4U;

    uGridWidth=uColumns*uMarkerWidth+(uColumns-1U)*uGapX;
    uGridHeight=uRows*uMarkerHeight+(uRows-1U)*uGapY;
    uBaseX=uRegionX+(uRegionWidth>uGridWidth?(uRegionWidth-uGridWidth)/2U:0U);
    uBaseY=uRegionY+(uRegionHeight>uGridHeight?(uRegionHeight-uGridHeight)/2U:0U);

    for(uIndex=0U;uIndex<uCount;++uIndex)
    {
        uint32_t uRow=uIndex/uColumns;
        uint32_t uColumn=uIndex%uColumns;
        uint32_t uX=uBaseX+uColumn*(uMarkerWidth+uGapX);
        uint32_t uY=uBaseY+uRow*(uMarkerHeight+uGapY);
        uint32_t uMarker=uIndex==uSelected?uBright:uEdge;

        Floppy144DrawFillRect(
            pSurface,
            uX,
            uY,
            uMarkerWidth,
            uMarkerHeight,
            uMarker
        );
    }
}

static void Floppy144CabinetDrawWrappedText(
    Floppy144Surface *pSurface,
    uint32_t uX,
    uint32_t uY,
    const char *pszText,
    uint32_t uMaximumCharacters,
    uint32_t uMaximumLines,
    uint32_t uLineHeight,
    uint32_t uColour
)
{
    const char *pszRead = pszText;
    uint32_t uLine;

    if(
        pSurface == NULL ||
        pszRead == NULL ||
        uMaximumCharacters == 0U ||
        uMaximumLines == 0U
    )
    {
        return;
    }

    for(uLine = 0U; uLine < uMaximumLines && *pszRead != '\0'; ++uLine)
    {
        char szLine[64];
        uint32_t uLength = 0U;
        uint32_t uBreak = 0U;

        while(*pszRead == ' ')
            ++pszRead;

        while(
            pszRead[uLength] != '\0' &&
            pszRead[uLength] != '\n' &&
            uLength < uMaximumCharacters &&
            uLength + 1U < (uint32_t)sizeof(szLine)
        )
        {
            if(pszRead[uLength] == ' ')
                uBreak = uLength;

            ++uLength;
        }

        if(
            pszRead[uLength] != '\0' &&
            pszRead[uLength] != '\n' &&
            uLength == uMaximumCharacters &&
            uBreak > 0U
        )
        {
            uLength = uBreak;
        }

        memcpy(szLine, pszRead, uLength);
        szLine[uLength] = '\0';

        Floppy144DrawText(
            pSurface,
            uX,
            uY + uLine * uLineHeight,
            szLine,
            1U,
            uColour
        );

        pszRead += uLength;

        if(*pszRead == '\n')
            ++pszRead;

        while(*pszRead == ' ')
            ++pszRead;
    }
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

    Floppy144DrawText(
        pSurface,
        28U,
        20U,
        pCabinet->bSecureContainer ? "CABINET INTERIOR" : "RECOVERED CONTENTS",
        2U,
        uBright
    );
    Floppy144DrawText(pSurface, 28U, 48U, pCabinet->szDisplayName, 1U, uText);

    Floppy144CabinetDrawContainerBody(
        pSurface,
        pCabinet,
        uMetal,
        uEdge
    );

    Floppy144DrawText(
        pSurface,
        318U,
        82U,
        "RECOVERED CONTENTS - INSPECTION ONLY",
        1U,
        uMuted
    );

    uCount = Floppy144CabinetVisibleContentCount(pCabinet, pRunState);

    Floppy144CabinetDrawContentMarkers(
        pSurface,
        pCabinet,
        uCount,
        pCabinet->uSelectedContent,
        uEdge,
        uBright
    );

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
        "UP/DOWN SELECT  ENTER VIEW  BACKSPACE SITE",
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

        if(pItem != NULL)
        {
            char szName[48];
            const char *pszDetail =
                pItem->pszF != NULL
                    ? pItem->pszF
                    : "A recovered physical item from the reconstructed site.";

            /*
             * The recovered item's authored name is the screen heading.
             * Generic ledger labels and P- IDs stay in the data layer rather
             * than leaking into the fiction.
             */
            Floppy144CabinetCopyForDisplay(
                szName,
                sizeof(szName),
                pItem->pszA != NULL ? pItem->pszA : "RECOVERED ITEM",
                38U
            );

            Floppy144DrawText(pSurface, 94U, 94U, szName, 2U, uBright);

            /*
             * Physical-item descriptions are deliberately shorter than
             * authored documents, but may contain enough transcription to
             * make labels, notices, tags and checklists feel like real props.
             */
            Floppy144CabinetDrawWrappedText(
                pSurface,
                94U,
                150U,
                pszDetail,
                54U,
                3U,
                20U,
                uText
            );
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
            pCabinet->bSecureContainer
                ? "BACKSPACE: CABINET"
                : "BACKSPACE: CONTENTS",
            1U,
            uMuted
        );
    }
}

void Floppy144CabinetDraw(
    Floppy144Surface *pSurface,
    const Floppy144CabinetState *pCabinet,
    const Floppy144RunState *pRunState
)
{
    Floppy144Surface sSurface;
    const uint32_t uBackground = FLOPPY144_RGB(8, 13, 11);

    if(
        pSurface == NULL ||
        pCabinet == NULL ||
        pRunState == NULL ||
        pSurface->pixels == NULL
    )
    {
        return;
    }

    sSurface = *pSurface;

    Floppy144DrawClear(&sSurface, uBackground);

    if(pCabinet->bInteriorOpen)
        Floppy144CabinetDrawInterior(&sSurface, pCabinet, pRunState);
    else
        Floppy144CabinetDrawKeypad(&sSurface, pCabinet, pRunState);
}
