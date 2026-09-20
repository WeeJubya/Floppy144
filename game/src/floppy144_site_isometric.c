/*
 * Floppy//144 - lightweight Stage 2 isometric Site projection
 *
 * This is intentionally a small 2.5D renderer, not a second world model.
 * Every projected line comes from the same generated Site rectangles used by
 * the 2D renderer and collision system. The only additional information is a
 * display height chosen from the element type.
 */
#include "floppy144_site_isometric.h"

#include "floppy144_draw.h"
#include "floppy144_game_data.h"
#include "floppy144_cabinet.h"
#include "floppy144_site.h"
#include "floppy144_site_object.h"
#include "floppy144_site_rooms.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FLOPPY144_ISO_ORIGIN_X              320
#define FLOPPY144_ISO_ORIGIN_Y               45
#define FLOPPY144_ISO_HALF_TILE_X             3
#define FLOPPY144_ISO_HALF_TILE_Y             1
#define FLOPPY144_ISO_HEIGHT_SCALE            2

/*
 * Write one clipped pixel. A private line primitive keeps this projection
 * independent of platform APIs and avoids growing the shared drawing module.
 */
static void Floppy144IsometricPixel(
    Floppy144Surface *pSurface,
    int32_t nX,
    int32_t nY,
    uint32_t uColour
)
{
    if(
        pSurface == NULL ||
        pSurface->pixels == NULL ||
        nX < 0 ||
        nY < 0 ||
        (uint32_t)nX >= pSurface->width ||
        (uint32_t)nY >= pSurface->height
    )
    {
        return;
    }

    pSurface->pixels[(uint32_t)nY * pSurface->width + (uint32_t)nX] =
        uColour;
}

/* Bresenham line used for diamond footprints and raised object edges. */
static void Floppy144IsometricLine(
    Floppy144Surface *pSurface,
    int32_t nX0,
    int32_t nY0,
    int32_t nX1,
    int32_t nY1,
    uint32_t uColour
)
{
    int32_t nDx = nX1 >= nX0 ? nX1 - nX0 : nX0 - nX1;
    int32_t nSx = nX0 < nX1 ? 1 : -1;
    int32_t nDy = -(nY1 >= nY0 ? nY1 - nY0 : nY0 - nY1);
    int32_t nSy = nY0 < nY1 ? 1 : -1;
    int32_t nError = nDx + nDy;

    for(;;)
    {
        int32_t nError2;

        Floppy144IsometricPixel(pSurface, nX0, nY0, uColour);

        if(nX0 == nX1 && nY0 == nY1)
        {
            break;
        }

        nError2 = 2 * nError;

        if(nError2 >= nDy)
        {
            nError += nDy;
            nX0 += nSx;
        }

        if(nError2 <= nDx)
        {
            nError += nDx;
            nY0 += nSy;
        }
    }
}

/* Convert whole-unit Site coordinates into the compact isometric viewport. */
static void Floppy144IsometricProject(
    int32_t nWorldX,
    int32_t nWorldY,
    int32_t nHeight,
    int32_t *pnScreenX,
    int32_t *pnScreenY
)
{
    if(pnScreenX != NULL)
    {
        *pnScreenX =
            FLOPPY144_ISO_ORIGIN_X +
            (nWorldX - nWorldY) * FLOPPY144_ISO_HALF_TILE_X;
    }

    if(pnScreenY != NULL)
    {
        *pnScreenY =
            FLOPPY144_ISO_ORIGIN_Y +
            (nWorldX + nWorldY) * FLOPPY144_ISO_HALF_TILE_Y -
            nHeight * FLOPPY144_ISO_HEIGHT_SCALE;
    }
}


/*
 * Stage 3C Task 12 presentation helpers.
 *
 * Half-unit fixture depth is represented in x16 fixed-point space. This keeps
 * the ISO renderer integer-only while allowing wall-mounted objects to project
 * as genuinely shallow 0.5U boxes rather than full furniture blocks.
 */
static void Floppy144IsometricProjectX16(
    int32_t nWorldX16,
    int32_t nWorldY16,
    int32_t nHeight16,
    int32_t *pnScreenX,
    int32_t *pnScreenY
)
{
    if(pnScreenX != NULL)
    {
        *pnScreenX =
            FLOPPY144_ISO_ORIGIN_X +
            (
                (nWorldX16 - nWorldY16) *
                FLOPPY144_ISO_HALF_TILE_X
            ) / FLOPPY144_SITE_FIXED_ONE;
    }

    if(pnScreenY != NULL)
    {
        *pnScreenY =
            FLOPPY144_ISO_ORIGIN_Y +
            (
                (nWorldX16 + nWorldY16) *
                FLOPPY144_ISO_HALF_TILE_Y
            ) / FLOPPY144_SITE_FIXED_ONE -
            (
                nHeight16 *
                FLOPPY144_ISO_HEIGHT_SCALE
            ) / FLOPPY144_SITE_FIXED_ONE;
    }
}

static const Floppy144DataRecord *Floppy144IsometricPlacementForRect(
    const Floppy144SiteRect *pRect
)
{
    uint32_t uIndex;

    if(
        pRect == NULL ||
        pRect->room >= (uint8_t)FLOPPY144_ROOM_COUNT
    )
    {
        return NULL;
    }

    for(uIndex = 0U; uIndex < Floppy144GameDataRecordCount(); ++uIndex)
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uIndex);

        if(
            pRecord == NULL ||
            (
                pRecord->eKind != FLOPPY144_DATA_FURNITURE &&
                pRecord->eKind != FLOPPY144_DATA_FIXTURE
            ) ||
            pRecord->pszA == NULL ||
            Floppy144GameDataRoomId(pRecord->pszA) !=
                (Floppy144RoomId)pRect->room ||
            pRecord->n0 != (int32_t)pRect->x ||
            pRecord->n1 != (int32_t)pRect->y ||
            pRecord->n2 != (int32_t)pRect->width ||
            pRecord->n3 != (int32_t)pRect->height
        )
        {
            continue;
        }

        return pRecord;
    }

    return NULL;
}

static bool Floppy144IsometricVariantIs(
    const Floppy144DataRecord *pPlacement,
    const char *pszVariant
)
{
    return
        pPlacement != NULL &&
        pPlacement->pszC != NULL &&
        pszVariant != NULL &&
        strcmp(pPlacement->pszC, pszVariant) == 0;
}

static uint32_t Floppy144IsometricClutterHash(
    const Floppy144SiteRect *pRect
)
{
    uint32_t uValue = 2166136261U;

    if(pRect == NULL)
    {
        return uValue;
    }

#define FLOPPY144_ISO_HASH_BYTE(v) \
    do { uValue ^= (uint32_t)(v); uValue *= 16777619U; } while(0)

    FLOPPY144_ISO_HASH_BYTE(pRect->room);
    FLOPPY144_ISO_HASH_BYTE(pRect->type);
    FLOPPY144_ISO_HASH_BYTE(pRect->x);
    FLOPPY144_ISO_HASH_BYTE(pRect->y);
    FLOPPY144_ISO_HASH_BYTE(pRect->width);
    FLOPPY144_ISO_HASH_BYTE(pRect->height);

#undef FLOPPY144_ISO_HASH_BYTE

    return uValue;
}

static void Floppy144IsometricDrawPrismX16(
    Floppy144Surface *pSurface,
    int32_t nX16,
    int32_t nY16,
    int32_t nWidth16,
    int32_t nDepth16,
    int32_t nBaseHeight16,
    int32_t nTopHeight16,
    uint32_t uColour
)
{
    int32_t ax, ay, bx, by, cx, cy, dx, dy;
    int32_t atx, aty, btx, bty, ctx, cty, dtx, dty;

    Floppy144IsometricProjectX16(
        nX16,
        nY16,
        nBaseHeight16,
        &ax,
        &ay
    );
    Floppy144IsometricProjectX16(
        nX16 + nWidth16,
        nY16,
        nBaseHeight16,
        &bx,
        &by
    );
    Floppy144IsometricProjectX16(
        nX16 + nWidth16,
        nY16 + nDepth16,
        nBaseHeight16,
        &cx,
        &cy
    );
    Floppy144IsometricProjectX16(
        nX16,
        nY16 + nDepth16,
        nBaseHeight16,
        &dx,
        &dy
    );

    Floppy144IsometricProjectX16(
        nX16,
        nY16,
        nTopHeight16,
        &atx,
        &aty
    );
    Floppy144IsometricProjectX16(
        nX16 + nWidth16,
        nY16,
        nTopHeight16,
        &btx,
        &bty
    );
    Floppy144IsometricProjectX16(
        nX16 + nWidth16,
        nY16 + nDepth16,
        nTopHeight16,
        &ctx,
        &cty
    );
    Floppy144IsometricProjectX16(
        nX16,
        nY16 + nDepth16,
        nTopHeight16,
        &dtx,
        &dty
    );

    Floppy144IsometricLine(pSurface, ax, ay, bx, by, uColour);
    Floppy144IsometricLine(pSurface, bx, by, cx, cy, uColour);
    Floppy144IsometricLine(pSurface, cx, cy, dx, dy, uColour);
    Floppy144IsometricLine(pSurface, dx, dy, ax, ay, uColour);

    Floppy144IsometricLine(pSurface, atx, aty, btx, bty, uColour);
    Floppy144IsometricLine(pSurface, btx, bty, ctx, cty, uColour);
    Floppy144IsometricLine(pSurface, ctx, cty, dtx, dty, uColour);
    Floppy144IsometricLine(pSurface, dtx, dty, atx, aty, uColour);

    Floppy144IsometricLine(pSurface, ax, ay, atx, aty, uColour);
    Floppy144IsometricLine(pSurface, bx, by, btx, bty, uColour);
    Floppy144IsometricLine(pSurface, cx, cy, ctx, cty, uColour);
    Floppy144IsometricLine(pSurface, dx, dy, dtx, dty, uColour);
}

static void Floppy144IsometricDrawWallFixture(
    Floppy144Surface *pSurface,
    const Floppy144SiteRect *pRect,
    const Floppy144DataRecord *pPlacement
)
{
    const uint32_t uBody = FLOPPY144_RGB(152, 159, 155);
    const uint32_t uDetail = FLOPPY144_RGB(65, 70, 68);
    const uint32_t uScreen = FLOPPY144_RGB(58, 112, 117);
    const uint32_t uPaper = FLOPPY144_RGB(188, 183, 161);
    const uint32_t uAmber = FLOPPY144_RGB(202, 155, 69);

    int32_t nX16;
    int32_t nY16;
    int32_t nWidth16;
    int32_t nDepth16;
    int32_t nMountBase16;
    int32_t nMountTop16;
    bool bThinX;
    int32_t nLineIndex;
    int32_t x0, y0, x1, y1;

    if(pSurface == NULL || pRect == NULL)
    {
        return;
    }

    nX16 = (int32_t)pRect->x * FLOPPY144_SITE_FIXED_ONE;
    nY16 = (int32_t)pRect->y * FLOPPY144_SITE_FIXED_ONE;
    nWidth16 = (int32_t)pRect->width * FLOPPY144_SITE_FIXED_ONE;
    nDepth16 = (int32_t)pRect->height * FLOPPY144_SITE_FIXED_ONE;
    bThinX = pRect->width <= pRect->height;

    /*
     * Centre the shallow projection inside the authored collision footprint.
     * MONITOR_BANK remains 1U deep; IT shelving retains its 2U authored depth.
     */
    if(Floppy144IsometricVariantIs(pPlacement, "MONITOR_BANK"))
    {
        if(bThinX)
        {
            nWidth16 = FLOPPY144_SITE_FIXED_ONE;
            nX16 +=
                (
                    (int32_t)pRect->width *
                    FLOPPY144_SITE_FIXED_ONE -
                    nWidth16
                ) / 2;
        }
        else
        {
            nDepth16 = FLOPPY144_SITE_FIXED_ONE;
            nY16 +=
                (
                    (int32_t)pRect->height *
                    FLOPPY144_SITE_FIXED_ONE -
                    nDepth16
                ) / 2;
        }
    }
    else if(
        pRect->room == (uint8_t)FLOPPY144_ROOM_IT_SUPPORT &&
        Floppy144IsometricVariantIs(pPlacement, "SHELVING")
    )
    {
        /* authored 2U depth is the deliberate exception */
    }
    else
    {
        if(bThinX)
        {
            nWidth16 = FLOPPY144_SITE_FIXED_ONE / 2;

            if(
                pRect->room != (uint8_t)FLOPPY144_ROOM_RECEPTION ||
                !Floppy144IsometricVariantIs(
                    pPlacement,
                    "SITE_DIRECTORY"
                )
            )
            {
                nX16 +=
                    (
                        (int32_t)pRect->width *
                        FLOPPY144_SITE_FIXED_ONE -
                        nWidth16
                    ) / 2;
            }
            /*
             * The Reception directory sits immediately on the east face of
             * the x=76 partition wall. Keeping the authored x edge here makes
             * the 0.5U projection start flush at that face instead of floating
             * a quarter-unit into the room.
             */
        }
        else
        {
            nDepth16 = FLOPPY144_SITE_FIXED_ONE / 2;
            nY16 +=
                (
                    (int32_t)pRect->height *
                    FLOPPY144_SITE_FIXED_ONE -
                    nDepth16
                ) / 2;
        }
    }

    nMountBase16 = 6 * FLOPPY144_SITE_FIXED_ONE;
    nMountTop16 = 9 * FLOPPY144_SITE_FIXED_ONE;

    if(Floppy144IsometricVariantIs(pPlacement, "MONITOR_BANK"))
    {
        nMountBase16 = 4 * FLOPPY144_SITE_FIXED_ONE;
        nMountTop16 = 10 * FLOPPY144_SITE_FIXED_ONE;
    }
    else if(Floppy144IsometricVariantIs(pPlacement, "SHELVING"))
    {
        nMountBase16 = 1 * FLOPPY144_SITE_FIXED_ONE;
        nMountTop16 = 9 * FLOPPY144_SITE_FIXED_ONE;
    }
    else if(
        pRect->room == (uint8_t)FLOPPY144_ROOM_RECEPTION &&
        Floppy144IsometricVariantIs(pPlacement, "SITE_DIRECTORY")
    )
    {
        /* Eye-level directory, below the high window/viewpoint sightline. */
        nMountBase16 = 3 * FLOPPY144_SITE_FIXED_ONE;
        nMountTop16 = 7 * FLOPPY144_SITE_FIXED_ONE;
    }

    Floppy144IsometricDrawPrismX16(
        pSurface,
        nX16,
        nY16,
        nWidth16,
        nDepth16,
        nMountBase16,
        nMountTop16,
        uBody
    );

    /*
     * Variant marks are intentionally tiny in ISO. They only need to make
     * panels, boards, key storage, monitor glass and shelving read as different
     * objects at the compact 640x360 projection.
     */
    if(Floppy144IsometricVariantIs(pPlacement, "MONITOR_BANK"))
    {
        for(nLineIndex = 1; nLineIndex < 5; ++nLineIndex)
        {
            int32_t nZ16 =
                nMountBase16 +
                (nMountTop16 - nMountBase16) *
                nLineIndex / 5;

            Floppy144IsometricProjectX16(
                nX16,
                nY16,
                nZ16,
                &x0,
                &y0
            );
            Floppy144IsometricProjectX16(
                nX16 + nWidth16,
                nY16 + nDepth16,
                nZ16,
                &x1,
                &y1
            );
            Floppy144IsometricLine(
                pSurface,
                x0,
                y0,
                x1,
                y1,
                uScreen
            );
        }
    }
    else if(
        Floppy144IsometricVariantIs(pPlacement, "PATCH_PANEL") ||
        Floppy144IsometricVariantIs(pPlacement, "SUPPRESSION_PANEL")
    )
    {
        int32_t nZ16 =
            (nMountBase16 + nMountTop16) / 2;

        Floppy144IsometricProjectX16(
            nX16,
            nY16,
            nZ16,
            &x0,
            &y0
        );
        Floppy144IsometricProjectX16(
            nX16 + nWidth16,
            nY16 + nDepth16,
            nZ16,
            &x1,
            &y1
        );
        Floppy144IsometricLine(
            pSurface,
            x0,
            y0,
            x1,
            y1,
            Floppy144IsometricVariantIs(
                pPlacement,
                "SUPPRESSION_PANEL"
            ) ? uAmber : uScreen
        );
    }
    else if(
        Floppy144IsometricVariantIs(pPlacement, "SITE_DIRECTORY") ||
        Floppy144IsometricVariantIs(pPlacement, "NOTICEBOARD")
    )
    {
        int32_t nZ16 =
            nMountBase16 +
            (nMountTop16 - nMountBase16) / 2;

        Floppy144IsometricProjectX16(
            nX16 + nWidth16 / 5,
            nY16 + nDepth16 / 5,
            nZ16,
            &x0,
            &y0
        );
        Floppy144IsometricProjectX16(
            nX16 + nWidth16 * 4 / 5,
            nY16 + nDepth16 * 4 / 5,
            nZ16,
            &x1,
            &y1
        );
        Floppy144IsometricLine(
            pSurface,
            x0,
            y0,
            x1,
            y1,
            uPaper
        );
    }
    else if(
        Floppy144IsometricVariantIs(pPlacement, "KEY_CABINET") ||
        Floppy144IsometricVariantIs(pPlacement, "FIRST_AID_KIT")
    )
    {
        int32_t nZ16 =
            (nMountBase16 + nMountTop16) / 2;

        Floppy144IsometricProjectX16(
            nX16 + nWidth16 / 2,
            nY16 + nDepth16 / 2,
            nZ16 - FLOPPY144_SITE_FIXED_ONE,
            &x0,
            &y0
        );
        Floppy144IsometricProjectX16(
            nX16 + nWidth16 / 2,
            nY16 + nDepth16 / 2,
            nZ16 + FLOPPY144_SITE_FIXED_ONE,
            &x1,
            &y1
        );
        Floppy144IsometricLine(
            pSurface,
            x0,
            y0,
            x1,
            y1,
            Floppy144IsometricVariantIs(
                pPlacement,
                "KEY_CABINET"
            ) ? uAmber : uDetail
        );
    }
}

static void Floppy144IsometricDrawSink(
    Floppy144Surface *pSurface,
    const Floppy144SiteRect *pRect
)
{
    const uint32_t uRim = FLOPPY144_RGB(151, 151, 137);
    const uint32_t uBowl = FLOPPY144_RGB(54, 68, 70);
    int32_t nX16;
    int32_t nY16;
    int32_t nW16;
    int32_t nD16;
    int32_t ax, ay, bx, by, cx, cy, dx, dy;
    int32_t tap0x, tap0y, tap1x, tap1y, spoutx, spouty;

    if(pSurface == NULL || pRect == NULL)
    {
        return;
    }

    nX16 =
        (int32_t)pRect->x * FLOPPY144_SITE_FIXED_ONE +
        FLOPPY144_SITE_FIXED_ONE / 2;

    nY16 =
        (int32_t)pRect->y * FLOPPY144_SITE_FIXED_ONE +
        FLOPPY144_SITE_FIXED_ONE / 2;

    nW16 =
        (int32_t)pRect->width * FLOPPY144_SITE_FIXED_ONE -
        FLOPPY144_SITE_FIXED_ONE;

    nD16 =
        (int32_t)pRect->height * FLOPPY144_SITE_FIXED_ONE -
        FLOPPY144_SITE_FIXED_ONE;

    /* Recessed bowl sits below the four-unit worktop surface. */
    Floppy144IsometricProjectX16(
        nX16,
        nY16,
        3 * FLOPPY144_SITE_FIXED_ONE,
        &ax,
        &ay
    );
    Floppy144IsometricProjectX16(
        nX16 + nW16,
        nY16,
        3 * FLOPPY144_SITE_FIXED_ONE,
        &bx,
        &by
    );
    Floppy144IsometricProjectX16(
        nX16 + nW16,
        nY16 + nD16,
        3 * FLOPPY144_SITE_FIXED_ONE,
        &cx,
        &cy
    );
    Floppy144IsometricProjectX16(
        nX16,
        nY16 + nD16,
        3 * FLOPPY144_SITE_FIXED_ONE,
        &dx,
        &dy
    );

    Floppy144IsometricLine(pSurface, ax, ay, bx, by, uBowl);
    Floppy144IsometricLine(pSurface, bx, by, cx, cy, uBowl);
    Floppy144IsometricLine(pSurface, cx, cy, dx, dy, uBowl);
    Floppy144IsometricLine(pSurface, dx, dy, ax, ay, uBowl);

    /* Mixer stem rises above the rear edge, then bends over the bowl. */
    Floppy144IsometricProjectX16(
        nX16 + nW16 / 2,
        nY16,
        4 * FLOPPY144_SITE_FIXED_ONE,
        &tap0x,
        &tap0y
    );
    Floppy144IsometricProjectX16(
        nX16 + nW16 / 2,
        nY16,
        5 * FLOPPY144_SITE_FIXED_ONE,
        &tap1x,
        &tap1y
    );
    Floppy144IsometricProjectX16(
        nX16 + nW16 / 2,
        nY16 + FLOPPY144_SITE_FIXED_ONE / 2,
        5 * FLOPPY144_SITE_FIXED_ONE,
        &spoutx,
        &spouty
    );

    Floppy144IsometricLine(
        pSurface,
        tap0x,
        tap0y,
        tap1x,
        tap1y,
        uRim
    );
    Floppy144IsometricLine(
        pSurface,
        tap1x,
        tap1y,
        spoutx,
        spouty,
        uRim
    );
}

static void Floppy144IsometricDrawShelfClutter(
    Floppy144Surface *pSurface,
    const Floppy144SiteRect *pRect,
    int32_t nTopHeight
)
{
    const uint32_t uPaper = FLOPPY144_RGB(188, 183, 161);
    const uint32_t uBox = FLOPPY144_RGB(117, 100, 75);
    uint32_t uState;
    int32_t nCount;
    int32_t nIndex;

    if(pSurface == NULL || pRect == NULL)
    {
        return;
    }

    nCount =
        pRect->room == (uint8_t)FLOPPY144_ROOM_FACILITIES
            ? 15
            : 6;

    uState = Floppy144IsometricClutterHash(pRect);

    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
        int32_t nX16;
        int32_t nY16;
        int32_t nW16;
        int32_t nD16;
        int32_t nZ16;

        uState = uState * 1664525U + 1013904223U;
        nX16 =
            (int32_t)pRect->x * FLOPPY144_SITE_FIXED_ONE +
            (
                (int32_t)(uState % 12U) *
                (
                    (int32_t)pRect->width *
                    FLOPPY144_SITE_FIXED_ONE
                )
            ) / 16;

        uState = uState * 1664525U + 1013904223U;
        nY16 =
            (int32_t)pRect->y * FLOPPY144_SITE_FIXED_ONE +
            (
                (int32_t)(uState % 12U) *
                (
                    (int32_t)pRect->height *
                    FLOPPY144_SITE_FIXED_ONE
                )
            ) / 16;

        uState = uState * 1664525U + 1013904223U;
        nW16 =
            FLOPPY144_SITE_FIXED_ONE / 4 +
            (int32_t)(uState % 5U);

        uState = uState * 1664525U + 1013904223U;
        nD16 =
            FLOPPY144_SITE_FIXED_ONE / 4 +
            (int32_t)(uState % 5U);

        nZ16 =
            nTopHeight *
            FLOPPY144_SITE_FIXED_ONE;

        Floppy144IsometricDrawPrismX16(
            pSurface,
            nX16,
            nY16,
            nW16,
            nD16,
            nZ16,
            nZ16 + FLOPPY144_SITE_FIXED_ONE / 3,
            (uState & 1U) != 0U ? uPaper : uBox
        );
    }
}

/* Floors stay flat. Furniture is low; walls/partitions rise further. */
static int32_t Floppy144IsometricElementHeight(Floppy144SiteElement eElement)
{
    if(
        eElement >= FLOPPY144_SITE_FLOOR_A &&
        eElement <= FLOPPY144_SITE_FLOOR_D
    )
    {
        return 0;
    }

    if(
        eElement == FLOPPY144_SITE_DOOR ||
        eElement == FLOPPY144_SITE_WINDOW ||
        eElement == FLOPPY144_SITE_PARTITION_WALL
    )
    {
        return FLOPPY144_SITE_WALL_HEIGHT_UNITS;
    }

    switch(eElement)
    {
        case FLOPPY144_SITE_SECURE_CABINET_HALF:
        case FLOPPY144_SITE_SECURE_CABINET_FULL:
        case FLOPPY144_SITE_BOOKCASE:
        case FLOPPY144_SITE_FRIDGE:
        case FLOPPY144_SITE_SERVER:
        case FLOPPY144_SITE_SHELVING_FULL:
            return 7;

        case FLOPPY144_SITE_WALL_MOUNTED_ITEM:
            return 9;

        default:
            return 4;
    }
}

static uint32_t Floppy144IsometricElementColour(Floppy144SiteElement eElement)
{
    if(
        eElement >= FLOPPY144_SITE_FLOOR_A &&
        eElement <= FLOPPY144_SITE_FLOOR_D
    )
    {
        return FLOPPY144_RGB(67, 71, 67);
    }

    if(eElement == FLOPPY144_SITE_DOOR)
    {
        return FLOPPY144_RGB(194, 153, 76);
    }

    if(eElement == FLOPPY144_SITE_WINDOW)
    {
        return FLOPPY144_RGB(100, 151, 166);
    }

    return FLOPPY144_RGB(118, 133, 132);
}

/*
 * Draw a rectangle as an isometric footprint and, when non-zero height is
 * requested, lift its top face and connect the visible corners vertically.
 */
static void Floppy144IsometricDrawRect(
    Floppy144Surface *pSurface,
    const Floppy144SiteRect *pRect
)
{
    const Floppy144DataRecord *pPlacement;

    int32_t nX0;
    int32_t nY0;
    int32_t nX1;
    int32_t nY1;
    int32_t nX2;
    int32_t nY2;
    int32_t nX3;
    int32_t nY3;
    int32_t nTopX0;
    int32_t nTopY0;
    int32_t nTopX1;
    int32_t nTopY1;
    int32_t nTopX2;
    int32_t nTopY2;
    int32_t nTopX3;
    int32_t nTopY3;
    int32_t nHeight;
    uint32_t uColour;
    Floppy144SiteElement eElement;

    if(pSurface == NULL || pRect == NULL)
    {
        return;
    }

    eElement = (Floppy144SiteElement)pRect->type;
    nHeight = Floppy144IsometricElementHeight(eElement);
    uColour = Floppy144IsometricElementColour(eElement);

    pPlacement =
        Floppy144IsometricPlacementForRect(pRect);

    if(eElement == FLOPPY144_SITE_WALL_MOUNTED_ITEM)
    {
        Floppy144IsometricDrawWallFixture(
            pSurface,
            pRect,
            pPlacement
        );

        return;
    }

    if(
        eElement == FLOPPY144_SITE_SINK &&
        pRect->room == (uint8_t)FLOPPY144_ROOM_STAFF_ROOM
    )
    {
        Floppy144IsometricDrawSink(
            pSurface,
            pRect
        );

        return;
    }

    Floppy144IsometricProject(pRect->x, pRect->y, 0, &nX0, &nY0);
    Floppy144IsometricProject(pRect->x + pRect->width, pRect->y, 0, &nX1, &nY1);
    Floppy144IsometricProject(pRect->x + pRect->width, pRect->y + pRect->height, 0, &nX2, &nY2);
    Floppy144IsometricProject(pRect->x, pRect->y + pRect->height, 0, &nX3, &nY3);

    Floppy144IsometricLine(pSurface, nX0, nY0, nX1, nY1, uColour);
    Floppy144IsometricLine(pSurface, nX1, nY1, nX2, nY2, uColour);
    Floppy144IsometricLine(pSurface, nX2, nY2, nX3, nY3, uColour);
    Floppy144IsometricLine(pSurface, nX3, nY3, nX0, nY0, uColour);

    if(nHeight <= 0)
    {
        return;
    }

    Floppy144IsometricProject(pRect->x, pRect->y, nHeight, &nTopX0, &nTopY0);
    Floppy144IsometricProject(pRect->x + pRect->width, pRect->y, nHeight, &nTopX1, &nTopY1);
    Floppy144IsometricProject(pRect->x + pRect->width, pRect->y + pRect->height, nHeight, &nTopX2, &nTopY2);
    Floppy144IsometricProject(pRect->x, pRect->y + pRect->height, nHeight, &nTopX3, &nTopY3);

    Floppy144IsometricLine(pSurface, nTopX0, nTopY0, nTopX1, nTopY1, uColour);
    Floppy144IsometricLine(pSurface, nTopX1, nTopY1, nTopX2, nTopY2, uColour);
    Floppy144IsometricLine(pSurface, nTopX2, nTopY2, nTopX3, nTopY3, uColour);
    Floppy144IsometricLine(pSurface, nTopX3, nTopY3, nTopX0, nTopY0, uColour);

    Floppy144IsometricLine(pSurface, nX0, nY0, nTopX0, nTopY0, uColour);
    Floppy144IsometricLine(pSurface, nX1, nY1, nTopX1, nTopY1, uColour);
    Floppy144IsometricLine(pSurface, nX2, nY2, nTopX2, nTopY2, uColour);
    Floppy144IsometricLine(pSurface, nX3, nY3, nTopX3, nTopY3, uColour);

    if(
        eElement == FLOPPY144_SITE_BOOKCASE ||
        eElement == FLOPPY144_SITE_SHELVING_FULL
    )
    {
        Floppy144IsometricDrawShelfClutter(
            pSurface,
            pRect,
            nHeight
        );
    }
}

static const char *Floppy144IsometricRoomLabel(Floppy144RoomId eRoom)
{
    static const char *apszRoomNames[FLOPPY144_ROOM_COUNT] =
    {
#define FLOPPY144_ROOM(symbol, name) name,
#include "floppy144_rooms.def"
#undef FLOPPY144_ROOM
    };

    if((uint32_t)eRoom >= (uint32_t)FLOPPY144_ROOM_COUNT)
    {
        return "SITE";
    }

    return apszRoomNames[eRoom];
}

static const char *Floppy144IsometricInteractionPrompt(
    const Floppy144RunState *pRunState
)
{
    uint32_t uActions;
    Floppy144CabinetState sCabinetProbe;

    if(pRunState == NULL)
    {
        return "ARROWS TO MOVE   N NOTEBOOK";
    }

    uActions =
        Floppy144SiteAvailableActions(
            pRunState
        );

    /* STAGE 3B.5 SECURE CABINET ACCESS PROMPT */
    Floppy144CabinetReset(&sCabinetProbe);
    if(Floppy144CabinetOpenNearby(&sCabinetProbe, pRunState))
    {
        uActions |= FLOPPY144_SITE_ACTION_ACCESS;
    }

    if(
        (uActions & FLOPPY144_SITE_ACTION_ACCESS) != 0U &&
        (uActions & FLOPPY144_SITE_ACTION_INSPECT) != 0U
    )
    {
        return "A ACCESS   I INSPECT   N NOTEBOOK";
    }

    if((uActions & FLOPPY144_SITE_ACTION_ACCESS) != 0U)
    {
        return "A ACCESS   N NOTEBOOK";
    }

    if((uActions & FLOPPY144_SITE_ACTION_INSPECT) != 0U)
    {
        return "I INSPECT   N NOTEBOOK";
    }

    return "ARROWS TO MOVE   N NOTEBOOK";
}

/*
 * Isometric view may show several reconstructed rooms at once, but boundary
 * geometry is still gated by its explicit endpoints. Internal doors/windows
 * appear only after both rooms exist in the reconstruction. Exterior
 * boundaries require only their interior room.
 */
static bool Floppy144IsometricRectVisible(
    const Floppy144RunState *pRunState,
    const Floppy144SiteRect *pRect
)
{
    return
        Floppy144SiteRectRuntimeVisible(
            pRunState,
            pRect
        );
}

void Floppy144SiteIsometricDraw(
    F144Runtime *pRuntime,
    const Floppy144RunState *pRunState,
    const char *pszNotice
)
{
    Floppy144Surface sSurface;
    Floppy144RoomId eActiveRoom;
    uint32_t uRectIndex;
    uint32_t uRectCount;
    int32_t nPlayerX;
    int32_t nPlayerY;
    const char *pszRoomLabel;
    const char *pszContextLabel = NULL;
    char szStatus[64];

    if(
        pRuntime == NULL ||
        pRunState == NULL ||
        pRuntime->backbuffer.data == NULL
    )
    {
        return;
    }

    sSurface.pixels = (uint32_t *)pRuntime->backbuffer.data;
    sSurface.width = pRuntime->backbuffer.width;
    sSurface.height = pRuntime->backbuffer.height;

    eActiveRoom = Floppy144SiteRoomAtPosition(
        pRunState->player_site_x,
        pRunState->player_site_y
    );

    pszRoomLabel =
        Floppy144IsometricRoomLabel(eActiveRoom);

    if(pszNotice != NULL)
    {
        pszRoomLabel = pszNotice;
    }
    else
    {
        pszContextLabel =
            Floppy144SiteContextLabel(pRunState);

        if(pszContextLabel != NULL)
        {
            pszRoomLabel = pszContextLabel;
        }
    }

    Floppy144DrawClear(&sSurface, FLOPPY144_RGB(12, 17, 21));
    Floppy144DrawText(
        &sSurface,
        10,
        6,
        "GDR SITE RECONSTRUCTION // ISOMETRIC 2.5D",
        1,
        FLOPPY144_RGB(118, 133, 132)
    );

    uRectCount = Floppy144SiteRectCount();

    /* Pass 1: flat reconstructed room floors. */
    for(uRectIndex = 0U; uRectIndex < uRectCount; ++uRectIndex)
    {
        const Floppy144SiteRect *pRect = Floppy144SiteRectAt(uRectIndex);
        Floppy144SiteElement eElement;

        if(pRect == NULL)
        {
            continue;
        }

        eElement = (Floppy144SiteElement)pRect->type;

        if(
            eElement < FLOPPY144_SITE_FLOOR_A ||
            eElement > FLOPPY144_SITE_FLOOR_D ||
            pRect->room >= (uint8_t)FLOPPY144_ROOM_COUNT ||
            !Floppy144SiteRectRuntimeVisible(
                pRunState,
                pRect
            )
        )
        {
            continue;
        }

        Floppy144IsometricDrawRect(&sSurface, pRect);
    }

    /* Pass 2: visible structure and furniture above those reconstructed floors. */
    for(uRectIndex = 0U; uRectIndex < uRectCount; ++uRectIndex)
    {
        const Floppy144SiteRect *pRect = Floppy144SiteRectAt(uRectIndex);
        Floppy144SiteElement eElement;
        bool bRoomVisible;

        if(pRect == NULL)
        {
            continue;
        }

        eElement = (Floppy144SiteElement)pRect->type;

        if(eElement >= FLOPPY144_SITE_FLOOR_A && eElement <= FLOPPY144_SITE_FLOOR_D)
        {
            continue;
        }

        bRoomVisible =
            Floppy144IsometricRectVisible(
                pRunState,
                pRect
            );

        if(
            !bRoomVisible
        )
        {
            continue;
        }

        Floppy144IsometricDrawRect(&sSurface, pRect);
    }

    /* Player marker uses the persistent canonical foot point. */
    Floppy144IsometricProject(
        pRunState->player_site_x / FLOPPY144_SITE_FIXED_ONE,
        pRunState->player_site_y / FLOPPY144_SITE_FIXED_ONE,
        2,
        &nPlayerX,
        &nPlayerY
    );

    Floppy144IsometricLine(
        &sSurface,
        nPlayerX,
        nPlayerY - 8,
        nPlayerX,
        nPlayerY,
        FLOPPY144_RGB(100, 156, 111)
    );
    Floppy144IsometricLine(
        &sSurface,
        nPlayerX - 2,
        nPlayerY - 8,
        nPlayerX + 2,
        nPlayerY - 8,
        FLOPPY144_RGB(100, 156, 111)
    );

    Floppy144RunStateFormatCapacity(
        pRunState,
        szStatus,
        (uint32_t)sizeof(szStatus)
    );

    Floppy144DrawText(
        &sSurface,
        20,
        312,
        pszRoomLabel,
        1,
        FLOPPY144_RGB(202, 211, 205)
    );
    Floppy144DrawText(
        &sSurface,
        20,
        328,
        Floppy144IsometricInteractionPrompt(pRunState),
        1,
        FLOPPY144_RGB(194, 153, 76)
    );
    Floppy144DrawText(
        &sSurface,
        20,
        344,
        szStatus,
        1,
        FLOPPY144_RGB(118, 133, 132)
    );
}
