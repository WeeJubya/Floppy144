/*
 * Floppy//144 - restored Site Directory screen
 *
 * This is a deliberately sparse schematic. The complete 100 x 100 Site keeps
 * its canonical orientation, but only floor rectangles belonging to restored
 * rooms are drawn. That means the map reveals progress without leaking rooms,
 * furniture, fixtures or doors that the archive has not reconstructed yet.
 */

#include "floppy144_site_directory.h"

#include "floppy144_draw.h"
#include "floppy144_room.h"
#include "floppy144_site.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FLOPPY144_DIRECTORY_MAP_X        176U
#define FLOPPY144_DIRECTORY_MAP_Y         20U
#define FLOPPY144_DIRECTORY_MAP_SIZE     288U
#define FLOPPY144_DIRECTORY_SITE_SIZE    100U

#define FLOPPY144_DIRECTORY_FOOTER_X      20U
#define FLOPPY144_DIRECTORY_FOOTER_Y     312U
#define FLOPPY144_DIRECTORY_FOOTER_WIDTH 600U
#define FLOPPY144_DIRECTORY_FOOTER_HEIGHT 28U

static bool Floppy144SiteDirectoryIsFloor(
    Floppy144SiteElement eElement
)
{
    return
        eElement >= FLOPPY144_SITE_FLOOR_A &&
        eElement <= FLOPPY144_SITE_FLOOR_D;
}

static const char *Floppy144SiteDirectoryRoomName(
    Floppy144RoomId eRoom
)
{
    switch(eRoom)
    {
        #define FLOPPY144_ROOM(symbol, name) \
            case FLOPPY144_ROOM_##symbol: return name;

        #include "floppy144_rooms.def"

        #undef FLOPPY144_ROOM

        default:
            return "SITE";
    }
}

static uint32_t Floppy144SiteDirectoryMapX(
    uint32_t uSiteX
)
{
    return
        FLOPPY144_DIRECTORY_MAP_X +
        (
            uSiteX *
            FLOPPY144_DIRECTORY_MAP_SIZE /
            FLOPPY144_DIRECTORY_SITE_SIZE
        );
}

static uint32_t Floppy144SiteDirectoryMapY(
    uint32_t uSiteY
)
{
    return
        FLOPPY144_DIRECTORY_MAP_Y +
        (
            uSiteY *
            FLOPPY144_DIRECTORY_MAP_SIZE /
            FLOPPY144_DIRECTORY_SITE_SIZE
        );
}

static void Floppy144SiteDirectoryDrawCentredText(
    Floppy144Surface *pSurface,
    uint32_t uCentreX,
    uint32_t uY,
    const char *pszText,
    uint32_t uColour
)
{
    uint32_t uWidth;

    if(
        pSurface == NULL ||
        pszText == NULL
    )
    {
        return;
    }

    uWidth =
        Floppy144DrawTextWidth(
            pszText,
            1U
        );

    Floppy144DrawText(
        pSurface,
        uCentreX - uWidth / 2U,
        uY,
        pszText,
        1U,
        uColour
    );
}

static void Floppy144SiteDirectoryDrawRoomLabel(
    Floppy144Surface *pSurface,
    const char *pszName,
    const Floppy144SiteRect *pAnchor,
    uint32_t uTextColour
)
{
    uint32_t uLeft;
    uint32_t uRight;
    uint32_t uTop;
    uint32_t uBottom;
    uint32_t uCentreX;
    uint32_t uCentreY;
    uint32_t uRoomWidth;
    uint32_t uTextWidth;
    const char *pszBreak;

    if(
        pSurface == NULL ||
        pszName == NULL ||
        pAnchor == NULL
    )
    {
        return;
    }

    uLeft =
        Floppy144SiteDirectoryMapX(
            pAnchor->x
        );
    uRight =
        Floppy144SiteDirectoryMapX(
            (uint32_t)pAnchor->x +
            (uint32_t)pAnchor->width
        );
    uTop =
        Floppy144SiteDirectoryMapY(
            pAnchor->y
        );
    uBottom =
        Floppy144SiteDirectoryMapY(
            (uint32_t)pAnchor->y +
            (uint32_t)pAnchor->height
        );

    uCentreX =
        (uLeft + uRight) / 2U;
    uCentreY =
        (uTop + uBottom) / 2U;
    uRoomWidth =
        uRight > uLeft
            ? uRight - uLeft
            : 1U;

    uTextWidth =
        Floppy144DrawTextWidth(
            pszName,
            1U
        );

    /*
     * Long office names can exceed their room footprint at map scale.
     * Split once at the final space so the full authored room name remains
     * visible rather than abbreviating or truncating it.
     */
    pszBreak =
        strrchr(
            pszName,
            ' '
        );

    if(
        pszBreak != NULL &&
        uTextWidth > uRoomWidth + 12U
    )
    {
        char szFirst[32];
        char szSecond[32];
        size_t uFirstLength =
            (size_t)(pszBreak - pszName);

        if(uFirstLength >= sizeof(szFirst))
        {
            uFirstLength =
                sizeof(szFirst) - 1U;
        }

        memcpy(
            szFirst,
            pszName,
            uFirstLength
        );

        szFirst[uFirstLength] =
            '\0';

        (void)snprintf(
            szSecond,
            sizeof(szSecond),
            "%s",
            pszBreak + 1
        );

        Floppy144SiteDirectoryDrawCentredText(
            pSurface,
            uCentreX,
            uCentreY - 8U,
            szFirst,
            uTextColour
        );

        Floppy144SiteDirectoryDrawCentredText(
            pSurface,
            uCentreX,
            uCentreY + 2U,
            szSecond,
            uTextColour
        );

        return;
    }

    Floppy144SiteDirectoryDrawCentredText(
        pSurface,
        uCentreX,
        uCentreY - 3U,
        pszName,
        uTextColour
    );
}

void Floppy144SiteDirectoryDraw(
    F144Runtime *pRuntime,
    const Floppy144RunState *pRunState
)
{
    const uint32_t uBackground =
        FLOPPY144_RGB(12, 17, 21);
    const uint32_t uRoomFill =
        FLOPPY144_RGB(50, 68, 66);
    const uint32_t uRoomEdge =
        FLOPPY144_RGB(128, 148, 140);
    const uint32_t uText =
        FLOPPY144_RGB(211, 220, 213);

    Floppy144Surface sSurface;
    uint32_t uRectCount;
    uint32_t uRectIndex;
    uint32_t uRoomIndex;
    uint32_t uTitleWidth;

    if(
        pRuntime == NULL ||
        pRunState == NULL ||
        pRuntime->backbuffer.data == NULL
    )
    {
        return;
    }

    sSurface.pixels =
        (uint32_t *)pRuntime->backbuffer.data;
    sSurface.width =
        pRuntime->backbuffer.width;
    sSurface.height =
        pRuntime->backbuffer.height;

    Floppy144DrawClear(
        &sSurface,
        uBackground
    );

    uTitleWidth =
        Floppy144DrawTextWidth(
            "GDR SITE DIRECTORY",
            1U
        );

    Floppy144DrawText(
        &sSurface,
        (sSurface.width - uTitleWidth) / 2U,
        5U,
        "GDR SITE DIRECTORY",
        1U,
        uText
    );

    uRectCount =
        Floppy144SiteRectCount();

    /*
     * First pass: room footprints only. Floor geometry is the authoritative
     * room shape, so no furniture, fixture, wall item, door or window can
     * accidentally leak into this screen.
     */
    for(
        uRectIndex = 0U;
        uRectIndex < uRectCount;
        ++uRectIndex
    )
    {
        const Floppy144SiteRect *pRect =
            Floppy144SiteRectAt(
                uRectIndex
            );

        uint32_t uLeft;
        uint32_t uRight;
        uint32_t uTop;
        uint32_t uBottom;

        if(
            pRect == NULL ||
            pRect->room >=
                (uint8_t)FLOPPY144_ROOM_COUNT ||
            !Floppy144SiteDirectoryIsFloor(
                (Floppy144SiteElement)pRect->type
            ) ||
            !Floppy144RunStateRoomReconstructed(
                pRunState,
                (Floppy144RoomId)pRect->room
            )
        )
        {
            continue;
        }

        uLeft =
            Floppy144SiteDirectoryMapX(
                pRect->x
            );
        uRight =
            Floppy144SiteDirectoryMapX(
                (uint32_t)pRect->x +
                (uint32_t)pRect->width
            );
        uTop =
            Floppy144SiteDirectoryMapY(
                pRect->y
            );
        uBottom =
            Floppy144SiteDirectoryMapY(
                (uint32_t)pRect->y +
                (uint32_t)pRect->height
            );

        Floppy144DrawFillRect(
            &sSurface,
            uLeft,
            uTop,
            uRight > uLeft
                ? uRight - uLeft
                : 1U,
            uBottom > uTop
                ? uBottom - uTop
                : 1U,
            uRoomFill
        );

        Floppy144DrawRect(
            &sSurface,
            uLeft,
            uTop,
            uRight > uLeft
                ? uRight - uLeft
                : 1U,
            uBottom > uTop
                ? uBottom - uTop
                : 1U,
            uRoomEdge
        );
    }

    /*
     * Second pass: one room name per reconstructed room. For L-shaped rooms,
     * the largest floor rectangle is used as the label anchor so the name is
     * guaranteed to sit on visible recovered geometry.
     */
    for(
        uRoomIndex = 0U;
        uRoomIndex <
            (uint32_t)FLOPPY144_ROOM_COUNT;
        ++uRoomIndex
    )
    {
        const Floppy144SiteRect *pLargestFloor =
            NULL;

        uint32_t uLargestArea =
            0U;

        if(
            !Floppy144RunStateRoomReconstructed(
                pRunState,
                (Floppy144RoomId)uRoomIndex
            )
        )
        {
            continue;
        }

        for(
            uRectIndex = 0U;
            uRectIndex < uRectCount;
            ++uRectIndex
        )
        {
            const Floppy144SiteRect *pRect =
                Floppy144SiteRectAt(
                    uRectIndex
                );

            uint32_t uArea;

            if(
                pRect == NULL ||
                pRect->room !=
                    (uint8_t)uRoomIndex ||
                !Floppy144SiteDirectoryIsFloor(
                    (Floppy144SiteElement)pRect->type
                )
            )
            {
                continue;
            }

            uArea =
                (uint32_t)pRect->width *
                (uint32_t)pRect->height;

            if(uArea > uLargestArea)
            {
                uLargestArea =
                    uArea;

                pLargestFloor =
                    pRect;
            }
        }

        if(pLargestFloor != NULL)
        {
            Floppy144SiteDirectoryDrawRoomLabel(
                &sSurface,
                Floppy144SiteDirectoryRoomName(
                    (Floppy144RoomId)uRoomIndex
                ),
                pLargestFloor,
                uText
            );
        }
    }

    /*
     * Keep the standard footer footprint so the map behaves like every other
     * game screen. It intentionally carries no close label: any subsequent
     * key press returns directly to Site exploration.
     */
    Floppy144DrawFillRect(
        &sSurface,
        FLOPPY144_DIRECTORY_FOOTER_X,
        FLOPPY144_DIRECTORY_FOOTER_Y,
        FLOPPY144_DIRECTORY_FOOTER_WIDTH,
        FLOPPY144_DIRECTORY_FOOTER_HEIGHT,
        uBackground
    );

    Floppy144DrawRect(
        &sSurface,
        FLOPPY144_DIRECTORY_FOOTER_X,
        FLOPPY144_DIRECTORY_FOOTER_Y,
        FLOPPY144_DIRECTORY_FOOTER_WIDTH,
        FLOPPY144_DIRECTORY_FOOTER_HEIGHT,
        uRoomEdge
    );
}
