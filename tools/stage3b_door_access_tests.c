/*
 * Floppy//144 Stage 3B.4 door/access regression.
 *
 * This slice formalises the movement contract already shared by the Site,
 * generated connection data and RunState:
 *
 * - reconstructed rooms are not automatically accessible;
 * - locked internal doors block crossing in both directions;
 * - generated trigger effects unlock the canonical connection;
 * - unlocked internal doors are crossed by movement, with no door action;
 * - unlocked exterior doors request a Site exit without moving the player;
 * - locked exterior doors remain impassable.
 */

#include "floppy144_game_data.h"
#include "floppy144_run_state.h"
#include "floppy144_site.h"
#include "floppy144_site_rooms.h"
#include "floppy144_trigger_engine.h"
#include "floppy144_world.h"

#include <stdbool.h>
#include <stdio.h>

static int g_nFailures = 0;

#define F144_CHECK(bCondition, pszMessage)                         \
    do                                                            \
    {                                                             \
        if(!(bCondition))                                         \
        {                                                         \
            fprintf(                                              \
                stderr,                                           \
                "FAIL: %s (line %d)\n",                          \
                (pszMessage),                                     \
                __LINE__                                          \
            );                                                    \
            ++g_nFailures;                                        \
        }                                                         \
    }                                                             \
    while(0)

static void Floppy144TestReset(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState
)
{
    Floppy144WorldReset(pWorld);
    Floppy144RunStateBegin(pState, 144U);
}

typedef struct Floppy144TestApproach
{
    int32_t nStartX16;
    int32_t nStartY16;
    int32_t nDeltaX16;
    int32_t nDeltaY16;
}
Floppy144TestApproach;

/*
 * Walk in half-unit steps until the requested room is reached or movement is
 * refused. This exercises the real Site collision and room-transition path.
 */
static bool Floppy144TestWalkToRoom(
    Floppy144RunState *pState,
    Floppy144RoomId eTargetRoom,
    int32_t nDeltaX16,
    int32_t nDeltaY16,
    uint32_t uMaximumSteps,
    bool *pbBlocked
)
{
    uint32_t uStep;

    if(pbBlocked != NULL)
    {
        *pbBlocked = false;
    }

    for(uStep = 0U; uStep < uMaximumSteps; ++uStep)
    {
        Floppy144RoomId eRoom =
            Floppy144SiteRoomAtPosition(
                pState->player_site_x,
                pState->player_site_y
            );

        if(eRoom == eTargetRoom)
        {
            return true;
        }

        if(
            !Floppy144RunStateMovePlayerSite(
                pState,
                nDeltaX16,
                nDeltaY16
            )
        )
        {
            if(pbBlocked != NULL)
            {
                *pbBlocked = true;
            }

            break;
        }
    }

    return
        Floppy144SiteRoomAtPosition(
            pState->player_site_x,
            pState->player_site_y
        ) == eTargetRoom;
}

static bool Floppy144TestDoorJoinsRooms(
    const Floppy144SiteRect *pDoor,
    Floppy144RoomId eRoomA,
    Floppy144RoomId eRoomB
)
{
    if(
        pDoor == NULL ||
        pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR
    )
    {
        return false;
    }

    return
        (
            pDoor->from_room == (uint8_t)eRoomA &&
            pDoor->to_room == (uint8_t)eRoomB
        ) ||
        (
            pDoor->from_room == (uint8_t)eRoomB &&
            pDoor->to_room == (uint8_t)eRoomA
        );
}

static const Floppy144SiteRect *Floppy144TestFindInternalDoor(
    Floppy144RoomId eRoomA,
    Floppy144RoomId eRoomB
)
{
    uint32_t uRectIndex;

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(uRectIndex);

        if(Floppy144TestDoorJoinsRooms(pRect, eRoomA, eRoomB))
        {
            return pRect;
        }
    }

    return NULL;
}

/*
 * Locate a real movement lane through an internal door by probing the compiled
 * Site geometry with an already-unlocked copy of RunState. This deliberately
 * avoids hard-coded Site coordinates: the generated geometry is authoritative.
 */
static bool Floppy144TestFindRoomApproach(
    const Floppy144RunState *pUnlockedState,
    Floppy144RoomId eFromRoom,
    Floppy144RoomId eToRoom,
    Floppy144TestApproach *pApproach
)
{
    static const int32_t anDirections[4][2] =
    {
        {  FLOPPY144_SITE_MOVE_STEP_X16, 0 },
        { -FLOPPY144_SITE_MOVE_STEP_X16, 0 },
        { 0,  FLOPPY144_SITE_MOVE_STEP_X16 },
        { 0, -FLOPPY144_SITE_MOVE_STEP_X16 }
    };

    const Floppy144SiteRect *pDoor;
    int32_t nMinX16;
    int32_t nMaxX16;
    int32_t nMinY16;
    int32_t nMaxY16;
    int32_t nX16;
    int32_t nY16;
    uint32_t uDirection;

    if(pUnlockedState == NULL || pApproach == NULL)
    {
        return false;
    }

    pDoor = Floppy144TestFindInternalDoor(eFromRoom, eToRoom);
    if(pDoor == NULL)
    {
        return false;
    }

    nMinX16 =
        ((int32_t)pDoor->x - 4) * FLOPPY144_SITE_FIXED_ONE;
    nMaxX16 =
        ((int32_t)pDoor->x + (int32_t)pDoor->width + 4) *
        FLOPPY144_SITE_FIXED_ONE;
    nMinY16 =
        ((int32_t)pDoor->y - 4) * FLOPPY144_SITE_FIXED_ONE;
    nMaxY16 =
        ((int32_t)pDoor->y + (int32_t)pDoor->height + 4) *
        FLOPPY144_SITE_FIXED_ONE;

    if(nMinX16 < 0) nMinX16 = 0;
    if(nMinY16 < 0) nMinY16 = 0;
    if(nMaxX16 >= FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)
    {
        nMaxX16 =
            FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE -
            FLOPPY144_SITE_MOVE_STEP_X16;
    }
    if(nMaxY16 >= FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)
    {
        nMaxY16 =
            FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE -
            FLOPPY144_SITE_MOVE_STEP_X16;
    }

    for(
        nY16 = nMinY16;
        nY16 <= nMaxY16;
        nY16 += FLOPPY144_SITE_MOVE_STEP_X16
    )
    {
        for(
            nX16 = nMinX16;
            nX16 <= nMaxX16;
            nX16 += FLOPPY144_SITE_MOVE_STEP_X16
        )
        {
            if(
                Floppy144SiteRoomAtPosition(nX16, nY16) !=
                eFromRoom
            )
            {
                continue;
            }

            for(uDirection = 0U; uDirection < 4U; ++uDirection)
            {
                Floppy144RunState sProbe = *pUnlockedState;
                uint32_t uStep;

                Floppy144RunStateSetPlayerSitePosition(
                    &sProbe,
                    nX16,
                    nY16
                );

                for(uStep = 0U; uStep < 16U; ++uStep)
                {
                    Floppy144RoomId eRoom;

                    if(
                        !Floppy144RunStateMovePlayerSite(
                            &sProbe,
                            anDirections[uDirection][0],
                            anDirections[uDirection][1]
                        )
                    )
                    {
                        break;
                    }

                    eRoom =
                        Floppy144SiteRoomAtPosition(
                            sProbe.player_site_x,
                            sProbe.player_site_y
                        );

                    if(eRoom == eToRoom)
                    {
                        pApproach->nStartX16 = nX16;
                        pApproach->nStartY16 = nY16;
                        pApproach->nDeltaX16 =
                            anDirections[uDirection][0];
                        pApproach->nDeltaY16 =
                            anDirections[uDirection][1];
                        return true;
                    }

                    if(eRoom != eFromRoom)
                    {
                        break;
                    }
                }
            }
        }
    }

    return false;
}

/*
 * Locate a one-step exterior threshold crossing from generated Site geometry.
 * bExpectExit selects an unlocked exit (true) or a geometrically valid but
 * progression-locked exit (false).
 */
static bool Floppy144TestFindExteriorApproach(
    const Floppy144RunState *pState,
    Floppy144RoomId eInteriorRoom,
    bool bExpectExit,
    Floppy144TestApproach *pApproach
)
{
    static const int32_t anDirections[4][2] =
    {
        {  FLOPPY144_SITE_MOVE_STEP_X16, 0 },
        { -FLOPPY144_SITE_MOVE_STEP_X16, 0 },
        { 0,  FLOPPY144_SITE_MOVE_STEP_X16 },
        { 0, -FLOPPY144_SITE_MOVE_STEP_X16 }
    };

    uint32_t uRectIndex;

    if(pState == NULL || pApproach == NULL)
    {
        return false;
    }

    for(
        uRectIndex = 0U;
        uRectIndex < Floppy144SiteRectCount();
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pDoor =
            Floppy144SiteRectAt(uRectIndex);
        uint8_t uInteriorEndpoint;
        int32_t nMinX16;
        int32_t nMaxX16;
        int32_t nMinY16;
        int32_t nMaxY16;
        int32_t nX16;
        int32_t nY16;
        uint32_t uDirection;

        if(
            pDoor == NULL ||
            pDoor->type != (uint8_t)FLOPPY144_SITE_DOOR ||
            (
                pDoor->from_room != FLOPPY144_SITE_ROOM_OUTSIDE &&
                pDoor->to_room != FLOPPY144_SITE_ROOM_OUTSIDE
            )
        )
        {
            continue;
        }

        uInteriorEndpoint =
            pDoor->from_room == FLOPPY144_SITE_ROOM_OUTSIDE
                ? pDoor->to_room
                : pDoor->from_room;

        if(uInteriorEndpoint != (uint8_t)eInteriorRoom)
        {
            continue;
        }

        nMinX16 =
            ((int32_t)pDoor->x - 4) * FLOPPY144_SITE_FIXED_ONE;
        nMaxX16 =
            ((int32_t)pDoor->x + (int32_t)pDoor->width + 4) *
            FLOPPY144_SITE_FIXED_ONE;
        nMinY16 =
            ((int32_t)pDoor->y - 4) * FLOPPY144_SITE_FIXED_ONE;
        nMaxY16 =
            ((int32_t)pDoor->y + (int32_t)pDoor->height + 4) *
            FLOPPY144_SITE_FIXED_ONE;

        if(nMinX16 < 0) nMinX16 = 0;
        if(nMinY16 < 0) nMinY16 = 0;
        if(nMaxX16 >= FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)
        {
            nMaxX16 =
                FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE -
                FLOPPY144_SITE_MOVE_STEP_X16;
        }
        if(nMaxY16 >= FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE)
        {
            nMaxY16 =
                FLOPPY144_SITE_SIZE_UNITS * FLOPPY144_SITE_FIXED_ONE -
                FLOPPY144_SITE_MOVE_STEP_X16;
        }

        for(
            nY16 = nMinY16;
            nY16 <= nMaxY16;
            nY16 += FLOPPY144_SITE_MOVE_STEP_X16
        )
        {
            for(
                nX16 = nMinX16;
                nX16 <= nMaxX16;
                nX16 += FLOPPY144_SITE_MOVE_STEP_X16
            )
            {
                if(
                    Floppy144SiteRoomAtPosition(nX16, nY16) !=
                    eInteriorRoom
                )
                {
                    continue;
                }

                for(uDirection = 0U; uDirection < 4U; ++uDirection)
                {
                    const Floppy144SiteRect *pCrossedDoor =
                        Floppy144SiteExteriorDoorForMove(
                            nX16,
                            nY16,
                            anDirections[uDirection][0],
                            anDirections[uDirection][1]
                        );

                    if(pCrossedDoor != pDoor)
                    {
                        continue;
                    }

                    {
                        Floppy144RunState sProbe = *pState;

                        Floppy144RunStateSetPlayerSitePosition(
                            &sProbe,
                            nX16,
                            nY16
                        );

                        if(
                            Floppy144RunStateWouldExitSite(
                                &sProbe,
                                anDirections[uDirection][0],
                                anDirections[uDirection][1]
                            ) != bExpectExit
                        )
                        {
                            continue;
                        }
                    }

                    {
                        pApproach->nStartX16 = nX16;
                        pApproach->nStartY16 = nY16;
                        pApproach->nDeltaX16 =
                            anDirections[uDirection][0];
                        pApproach->nDeltaY16 =
                            anDirections[uDirection][1];
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

static bool Floppy144TestUnlockCorRec(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState
)
{
    Floppy144TriggerId eT004;
    Floppy144TriggerId eT005;

    if(pWorld == NULL || pState == NULL)
    {
        return false;
    }

    (void)Floppy144RunStateReconstructRoom(
        pState,
        FLOPPY144_ROOM_MAIN_OFFICE
    );

    eT004 = Floppy144GameDataTriggerId("T-004");
    eT005 = Floppy144GameDataTriggerId("T-005");

    if(
        eT004 >= FLOPPY144_TRIGGER_COUNT ||
        eT005 >= FLOPPY144_TRIGGER_COUNT
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateTriggerFired(pState, eT004) &&
        !Floppy144TriggerTryFire(pWorld, pState, eT004)
    )
    {
        return false;
    }

    if(
        !Floppy144RunStateTriggerFired(pState, eT005) &&
        !Floppy144TriggerTryFire(pWorld, pState, eT005)
    )
    {
        return false;
    }

    return Floppy144GameDataConnectionUnlocked(pState, "COR_REC");
}

/*
 * COR_REC is the compact internal-door fixture for 3B.4. Both rooms may be
 * reconstructed while T-005 has not yet unlocked their connection.
 */
static void Floppy144TestLockedInternalDoorBothDirections(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144WorldState sProbeWorld;
    Floppy144RunState sProbeState;
    Floppy144TestApproach sForward;
    Floppy144TestApproach sReverse;
    bool bForwardFound;
    bool bReverseFound;
    bool bBlocked = false;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(&sState, "COR_REC"),
        "COR_REC starts locked before T-005"
    );

    F144_CHECK(
        !Floppy144GameDataRoomTransitionAllowed(
            &sState,
            FLOPPY144_ROOM_RECEPTION,
            FLOPPY144_ROOM_CORRIDOR
        ),
        "reconstructed Reception cannot enter locked Corridor"
    );

    F144_CHECK(
        !Floppy144GameDataRoomTransitionAllowed(
            &sState,
            FLOPPY144_ROOM_CORRIDOR,
            FLOPPY144_ROOM_RECEPTION
        ),
        "locked internal connection blocks the reverse direction"
    );

    /*
     * Discover the actual generated door lane using an unlocked copy, then
     * replay that exact approach against the still-locked state.
     */
    sProbeWorld = sWorld;
    sProbeState = sState;

    F144_CHECK(
        Floppy144TestUnlockCorRec(&sProbeWorld, &sProbeState),
        "geometry probe can unlock COR_REC"
    );

    bForwardFound =
        Floppy144TestFindRoomApproach(
            &sProbeState,
            FLOPPY144_ROOM_RECEPTION,
            FLOPPY144_ROOM_CORRIDOR,
            &sForward
        );

    F144_CHECK(
        bForwardFound,
        "generated geometry provides a Reception-to-Corridor door lane"
    );

    if(bForwardFound)
    {
        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            sForward.nStartX16,
            sForward.nStartY16
        );
        bBlocked = false;

        F144_CHECK(
            Floppy144SiteRoomAtPosition(
                sState.player_site_x,
                sState.player_site_y
            ) == FLOPPY144_ROOM_RECEPTION,
            "derived forward fixture starts in Reception"
        );

        F144_CHECK(
            !Floppy144TestWalkToRoom(
                &sState,
                FLOPPY144_ROOM_CORRIDOR,
                sForward.nDeltaX16,
                sForward.nDeltaY16,
                16U,
                &bBlocked
            ) &&
            bBlocked,
            "locked COR_REC blocks physical movement into Corridor"
        );

        F144_CHECK(
            Floppy144SiteRoomAtPosition(
                sState.player_site_x,
                sState.player_site_y
            ) == FLOPPY144_ROOM_RECEPTION,
            "locked crossing leaves player on Reception side"
        );
    }

    bReverseFound =
        Floppy144TestFindRoomApproach(
            &sProbeState,
            FLOPPY144_ROOM_CORRIDOR,
            FLOPPY144_ROOM_RECEPTION,
            &sReverse
        );

    F144_CHECK(
        bReverseFound,
        "generated geometry provides a Corridor-to-Reception door lane"
    );

    if(bReverseFound)
    {
        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            sReverse.nStartX16,
            sReverse.nStartY16
        );
        bBlocked = false;

        F144_CHECK(
            Floppy144SiteRoomAtPosition(
                sState.player_site_x,
                sState.player_site_y
            ) == FLOPPY144_ROOM_CORRIDOR,
            "derived reverse fixture starts in Corridor"
        );

        F144_CHECK(
            !Floppy144TestWalkToRoom(
                &sState,
                FLOPPY144_ROOM_RECEPTION,
                sReverse.nDeltaX16,
                sReverse.nDeltaY16,
                16U,
                &bBlocked
            ) &&
            bBlocked,
            "locked COR_REC blocks physical movement back into Reception"
        );

        F144_CHECK(
            Floppy144SiteRoomAtPosition(
                sState.player_site_x,
                sState.player_site_y
            ) == FLOPPY144_ROOM_CORRIDOR,
            "reverse locked crossing leaves player on Corridor side"
        );
    }
}

/*
 * T-005 owns the circulation unlock effects. Once it has fired, movement alone
 * crosses COR_REC in either direction. No separate door interaction is needed.
 */
static void Floppy144TestUnlockedInternalDoorMovement(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144TestApproach sForward;
    Floppy144TestApproach sReverse;
    bool bForwardFound;
    bool bReverseFound;
    bool bBlocked = false;

    Floppy144TestReset(&sWorld, &sState);

    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    F144_CHECK(
        Floppy144TestUnlockCorRec(&sWorld, &sState),
        "T-004/T-005 unlock the COR_REC circulation connection"
    );

    F144_CHECK(
        Floppy144GameDataConnectionUnlocked(&sState, "COR_REC"),
        "COR_REC is unlocked after T-005"
    );

    F144_CHECK(
        Floppy144GameDataRoomTransitionAllowed(
            &sState,
            FLOPPY144_ROOM_RECEPTION,
            FLOPPY144_ROOM_CORRIDOR
        ) &&
        Floppy144GameDataRoomTransitionAllowed(
            &sState,
            FLOPPY144_ROOM_CORRIDOR,
            FLOPPY144_ROOM_RECEPTION
        ),
        "unlocked internal connection permits both directions"
    );

    bForwardFound =
        Floppy144TestFindRoomApproach(
            &sState,
            FLOPPY144_ROOM_RECEPTION,
            FLOPPY144_ROOM_CORRIDOR,
            &sForward
        );

    F144_CHECK(
        bForwardFound,
        "generated geometry resolves unlocked forward COR_REC approach"
    );

    if(bForwardFound)
    {
        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            sForward.nStartX16,
            sForward.nStartY16
        );
        bBlocked = false;

        F144_CHECK(
            Floppy144TestWalkToRoom(
                &sState,
                FLOPPY144_ROOM_CORRIDOR,
                sForward.nDeltaX16,
                sForward.nDeltaY16,
                16U,
                &bBlocked
            ),
            "movement alone crosses unlocked COR_REC into Corridor"
        );

        F144_CHECK(
            !bBlocked,
            "unlocked forward crossing is not reported as blocked"
        );
    }

    bReverseFound =
        Floppy144TestFindRoomApproach(
            &sState,
            FLOPPY144_ROOM_CORRIDOR,
            FLOPPY144_ROOM_RECEPTION,
            &sReverse
        );

    F144_CHECK(
        bReverseFound,
        "generated geometry resolves unlocked reverse COR_REC approach"
    );

    if(bReverseFound)
    {
        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            sReverse.nStartX16,
            sReverse.nStartY16
        );
        bBlocked = false;

        F144_CHECK(
            Floppy144TestWalkToRoom(
                &sState,
                FLOPPY144_ROOM_RECEPTION,
                sReverse.nDeltaX16,
                sReverse.nDeltaY16,
                16U,
                &bBlocked
            ),
            "movement alone crosses unlocked COR_REC back into Reception"
        );

        F144_CHECK(
            !bBlocked,
            "unlocked reverse crossing is not reported as blocked"
        );
    }
}

/*
 * Exterior doors use generated geometry and generated connection state. Leaving
 * the Site is a coordinator transition rather than an out-of-bounds movement.
 */
static void Floppy144TestExteriorDoorAccess(void)
{
    Floppy144WorldState sWorld;
    Floppy144RunState sState;
    Floppy144TestApproach sApproach;
    bool bApproachFound;
    int32_t nOriginalX;
    int32_t nOriginalY;

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_RECEPTION
    );

    F144_CHECK(
        Floppy144GameDataConnectionUnlocked(&sState, "ENTRY_01") &&
        Floppy144GameDataConnectionUnlocked(&sState, "ENTRY_02"),
        "Reception exterior connections start unlocked"
    );

    bApproachFound =
        Floppy144TestFindExteriorApproach(
            &sState,
            FLOPPY144_ROOM_RECEPTION,
            true,
            &sApproach
        );

    F144_CHECK(
        bApproachFound,
        "generated Reception exterior door provides an unlocked exit approach"
    );

    if(bApproachFound)
    {
        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            sApproach.nStartX16,
            sApproach.nStartY16
        );

        nOriginalX = sState.player_site_x;
        nOriginalY = sState.player_site_y;

        F144_CHECK(
            Floppy144RunStateWouldExitSite(
                &sState,
                sApproach.nDeltaX16,
                sApproach.nDeltaY16
            ),
            "unlocked Reception exterior door requests Site exit"
        );

        F144_CHECK(
            sState.player_site_x == nOriginalX &&
            sState.player_site_y == nOriginalY,
            "exterior exit request preserves player Site position"
        );
    }

    Floppy144TestReset(&sWorld, &sState);
    (void)Floppy144RunStateReconstructRoom(
        &sState,
        FLOPPY144_ROOM_CORRIDOR
    );

    F144_CHECK(
        !Floppy144GameDataConnectionUnlocked(&sState, "EMER_01") &&
        !Floppy144GameDataConnectionUnlocked(&sState, "EMER_02"),
        "Corridor emergency exterior connections start locked"
    );

    bApproachFound =
        Floppy144TestFindExteriorApproach(
            &sState,
            FLOPPY144_ROOM_CORRIDOR,
            false,
            &sApproach
        );

    F144_CHECK(
        bApproachFound,
        "generated Corridor emergency door provides a locked threshold approach"
    );

    if(bApproachFound)
    {
        Floppy144RunStateSetPlayerSitePosition(
            &sState,
            sApproach.nStartX16,
            sApproach.nStartY16
        );

        nOriginalX = sState.player_site_x;
        nOriginalY = sState.player_site_y;

        F144_CHECK(
            !Floppy144RunStateWouldExitSite(
                &sState,
                sApproach.nDeltaX16,
                sApproach.nDeltaY16
            ),
            "locked Corridor emergency door refuses Site exit"
        );

        F144_CHECK(
            !Floppy144RunStateMovePlayerSite(
                &sState,
                sApproach.nDeltaX16,
                sApproach.nDeltaY16
            ),
            "locked exterior door also blocks ordinary movement"
        );

        F144_CHECK(
            sState.player_site_x == nOriginalX &&
            sState.player_site_y == nOriginalY,
            "locked exterior door leaves player inside Site"
        );
    }
}

int main(void)
{
    Floppy144TestLockedInternalDoorBothDirections();
    Floppy144TestUnlockedInternalDoorMovement();
    Floppy144TestExteriorDoorAccess();

    if(g_nFailures != 0)
    {
        fprintf(
            stderr,
            "\nSTAGE 3B.4 DOOR / ACCESS TESTS: FAIL (%d failure%s)\n",
            g_nFailures,
            g_nFailures == 1 ? "" : "s"
        );

        return 1;
    }

    puts("\nSTAGE 3B.4 DOOR / ACCESS TESTS: PASS");
    return 0;
}
