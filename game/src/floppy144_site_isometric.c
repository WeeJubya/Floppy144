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
#include "floppy144_site.h"
#include "floppy144_site_object.h"
#include "floppy144_site_rooms.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

    if(pRunState == NULL)
    {
        return "ARROWS TO MOVE   N NOTEBOOK";
    }

    uActions =
        Floppy144SiteAvailableActions(
            pRunState
        );

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

    snprintf(
        szStatus,
        sizeof(szStatus),
        "STATUS %02u%% // FM-23 PROJECTION ACTIVE",
        (unsigned)Floppy144RunStateReconstructionPercent(pRunState)
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
