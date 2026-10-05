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

#define FLOPPY144_ISO_VIEWPORT_X             32
#define FLOPPY144_ISO_VIEWPORT_Y             32
#define FLOPPY144_ISO_VIEWPORT_WIDTH        576
#define FLOPPY144_ISO_VIEWPORT_HEIGHT       252
#define FLOPPY144_ISO_SORT_MAX              768U

/*
 * FM-23 is a presentation change, not a second world model. Projection is
 * configured around the player's current room each frame while all gameplay
 * continues to use canonical Site coordinates.
 */
static int32_t g_nIsoOriginX=320;
static int32_t g_nIsoOriginY=170;
static int32_t g_nIsoCentreX16=0;
static int32_t g_nIsoCentreY16=0;
static int32_t g_nIsoHalfTileX=10;
static int32_t g_nIsoHalfTileY=4;
static int32_t g_nIsoHeightScale=6;

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
        nX < FLOPPY144_ISO_VIEWPORT_X ||
        nY < FLOPPY144_ISO_VIEWPORT_Y ||
        nX >= FLOPPY144_ISO_VIEWPORT_X+FLOPPY144_ISO_VIEWPORT_WIDTH ||
        nY >= FLOPPY144_ISO_VIEWPORT_Y+FLOPPY144_ISO_VIEWPORT_HEIGHT ||
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

static int64_t Floppy144IsometricEdge(
    int32_t ax,
    int32_t ay,
    int32_t bx,
    int32_t by,
    int32_t px,
    int32_t py
)
{
    return
        (int64_t)(px-ax)*(int64_t)(by-ay)-
        (int64_t)(py-ay)*(int64_t)(bx-ax);
}

static void Floppy144IsometricFillTriangle(
    Floppy144Surface *pSurface,
    int32_t ax,
    int32_t ay,
    int32_t bx,
    int32_t by,
    int32_t cx,
    int32_t cy,
    uint32_t uColour
)
{
    int32_t x0=ax,x1=ax,y0=ay,y1=ay,x,y;

    if(bx<x0)x0=bx;if(cx<x0)x0=cx;
    if(bx>x1)x1=bx;if(cx>x1)x1=cx;
    if(by<y0)y0=by;if(cy<y0)y0=cy;
    if(by>y1)y1=by;if(cy>y1)y1=cy;

    if(x0<FLOPPY144_ISO_VIEWPORT_X)x0=FLOPPY144_ISO_VIEWPORT_X;
    if(y0<FLOPPY144_ISO_VIEWPORT_Y)y0=FLOPPY144_ISO_VIEWPORT_Y;
    if(x1>=FLOPPY144_ISO_VIEWPORT_X+FLOPPY144_ISO_VIEWPORT_WIDTH)
        x1=FLOPPY144_ISO_VIEWPORT_X+FLOPPY144_ISO_VIEWPORT_WIDTH-1;
    if(y1>=FLOPPY144_ISO_VIEWPORT_Y+FLOPPY144_ISO_VIEWPORT_HEIGHT)
        y1=FLOPPY144_ISO_VIEWPORT_Y+FLOPPY144_ISO_VIEWPORT_HEIGHT-1;

    for(y=y0;y<=y1;++y)
    {
        for(x=x0;x<=x1;++x)
        {
            int64_t e0=Floppy144IsometricEdge(ax,ay,bx,by,x,y);
            int64_t e1=Floppy144IsometricEdge(bx,by,cx,cy,x,y);
            int64_t e2=Floppy144IsometricEdge(cx,cy,ax,ay,x,y);

            if(
                (e0>=0&&e1>=0&&e2>=0) ||
                (e0<=0&&e1<=0&&e2<=0)
            )
            {
                Floppy144IsometricPixel(pSurface,x,y,uColour);
            }
        }
    }
}

static void Floppy144IsometricFillQuad(
    Floppy144Surface *pSurface,
    int32_t ax,
    int32_t ay,
    int32_t bx,
    int32_t by,
    int32_t cx,
    int32_t cy,
    int32_t dx,
    int32_t dy,
    uint32_t uColour
)
{
    Floppy144IsometricFillTriangle(
        pSurface,ax,ay,bx,by,cx,cy,uColour
    );
    Floppy144IsometricFillTriangle(
        pSurface,ax,ay,cx,cy,dx,dy,uColour
    );
}

/*
 * FM-23 lighting model.
 *
 * The light source is above and to the north-west of the room. Top faces are
 * therefore brightest, the +X face is mid-tone and the +Y face is darkest.
 * Keeping this directional rule fixed makes furniture orientation readable
 * without textures.
 */
static uint32_t Floppy144IsometricShadeColour(
    uint32_t uColour,
    uint32_t uPercent
)
{
    uint32_t r=(uColour>>16)&0xffU;
    uint32_t g=(uColour>>8)&0xffU;
    uint32_t b=uColour&0xffU;

    r=(r*uPercent)/100U;
    g=(g*uPercent)/100U;
    b=(b*uPercent)/100U;

    if(r>255U)r=255U;
    if(g>255U)g=255U;
    if(b>255U)b=255U;

    return FLOPPY144_RGB(r,g,b);
}

static void Floppy144IsometricBlendPixel(
    Floppy144Surface *pSurface,
    int32_t nX,
    int32_t nY,
    uint32_t uColour,
    uint32_t uAlpha
)
{
    uint32_t uDestination;
    uint32_t sr,sg,sb,dr,dg,db;

    if(
        pSurface==NULL ||
        pSurface->pixels==NULL ||
        nX<FLOPPY144_ISO_VIEWPORT_X ||
        nY<FLOPPY144_ISO_VIEWPORT_Y ||
        nX>=FLOPPY144_ISO_VIEWPORT_X+FLOPPY144_ISO_VIEWPORT_WIDTH ||
        nY>=FLOPPY144_ISO_VIEWPORT_Y+FLOPPY144_ISO_VIEWPORT_HEIGHT ||
        (uint32_t)nX>=pSurface->width ||
        (uint32_t)nY>=pSurface->height
    )
    {
        return;
    }

    if(uAlpha>255U)uAlpha=255U;

    uDestination=
        pSurface->pixels[
            (uint32_t)nY*pSurface->width+
            (uint32_t)nX
        ];

    sr=(uColour>>16)&0xffU;
    sg=(uColour>>8)&0xffU;
    sb=uColour&0xffU;
    dr=(uDestination>>16)&0xffU;
    dg=(uDestination>>8)&0xffU;
    db=uDestination&0xffU;

    pSurface->pixels[
        (uint32_t)nY*pSurface->width+
        (uint32_t)nX
    ]=
        FLOPPY144_RGB(
            (sr*uAlpha+dr*(255U-uAlpha))/255U,
            (sg*uAlpha+dg*(255U-uAlpha))/255U,
            (sb*uAlpha+db*(255U-uAlpha))/255U
        );
}

static void Floppy144IsometricFillTriangleAlpha(
    Floppy144Surface *pSurface,
    int32_t ax,
    int32_t ay,
    int32_t bx,
    int32_t by,
    int32_t cx,
    int32_t cy,
    uint32_t uColour,
    uint32_t uAlpha
)
{
    int32_t x0=ax,x1=ax,y0=ay,y1=ay,x,y;

    if(bx<x0)x0=bx;if(cx<x0)x0=cx;
    if(bx>x1)x1=bx;if(cx>x1)x1=cx;
    if(by<y0)y0=by;if(cy<y0)y0=cy;
    if(by>y1)y1=by;if(cy>y1)y1=cy;

    if(x0<FLOPPY144_ISO_VIEWPORT_X)x0=FLOPPY144_ISO_VIEWPORT_X;
    if(y0<FLOPPY144_ISO_VIEWPORT_Y)y0=FLOPPY144_ISO_VIEWPORT_Y;
    if(x1>=FLOPPY144_ISO_VIEWPORT_X+FLOPPY144_ISO_VIEWPORT_WIDTH)
        x1=FLOPPY144_ISO_VIEWPORT_X+FLOPPY144_ISO_VIEWPORT_WIDTH-1;
    if(y1>=FLOPPY144_ISO_VIEWPORT_Y+FLOPPY144_ISO_VIEWPORT_HEIGHT)
        y1=FLOPPY144_ISO_VIEWPORT_Y+FLOPPY144_ISO_VIEWPORT_HEIGHT-1;

    for(y=y0;y<=y1;++y)
    {
        for(x=x0;x<=x1;++x)
        {
            int64_t e0=Floppy144IsometricEdge(ax,ay,bx,by,x,y);
            int64_t e1=Floppy144IsometricEdge(bx,by,cx,cy,x,y);
            int64_t e2=Floppy144IsometricEdge(cx,cy,ax,ay,x,y);

            if(
                (e0>=0&&e1>=0&&e2>=0) ||
                (e0<=0&&e1<=0&&e2<=0)
            )
            {
                Floppy144IsometricBlendPixel(
                    pSurface,x,y,uColour,uAlpha
                );
            }
        }
    }
}

static void Floppy144IsometricFillQuadAlpha(
    Floppy144Surface *pSurface,
    int32_t ax,
    int32_t ay,
    int32_t bx,
    int32_t by,
    int32_t cx,
    int32_t cy,
    int32_t dx,
    int32_t dy,
    uint32_t uColour,
    uint32_t uAlpha
)
{
    Floppy144IsometricFillTriangleAlpha(
        pSurface,ax,ay,bx,by,cx,cy,uColour,uAlpha
    );
    Floppy144IsometricFillTriangleAlpha(
        pSurface,ax,ay,cx,cy,dx,dy,uColour,uAlpha
    );
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
        *pnScreenX=
            g_nIsoOriginX+
            (
                (
                    (nWorldX16-g_nIsoCentreX16)-
                    (nWorldY16-g_nIsoCentreY16)
                )*
                g_nIsoHalfTileX
            )/
            FLOPPY144_SITE_FIXED_ONE;
    }

    if(pnScreenY != NULL)
    {
        *pnScreenY=
            g_nIsoOriginY+
            (
                (
                    (nWorldX16-g_nIsoCentreX16)+
                    (nWorldY16-g_nIsoCentreY16)
                )*
                g_nIsoHalfTileY
            )/
            FLOPPY144_SITE_FIXED_ONE-
            (
                nHeight16*
                g_nIsoHeightScale
            )/
            FLOPPY144_SITE_FIXED_ONE;
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
    const uint32_t uTop=
        Floppy144IsometricShadeColour(uColour,118U);
    const uint32_t uFaceX=
        Floppy144IsometricShadeColour(uColour,92U);
    const uint32_t uFaceY=
        Floppy144IsometricShadeColour(uColour,72U);
    const uint32_t uEdge=
        Floppy144IsometricShadeColour(uColour,42U);
    int32_t ax, ay, bx, by, cx, cy, dx, dy;
    int32_t atx, aty, btx, bty, ctx, cty, dtx, dty;

    Floppy144IsometricProjectX16(nX16,nY16,nBaseHeight16,&ax,&ay);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16,nBaseHeight16,&bx,&by);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16+nDepth16,nBaseHeight16,&cx,&cy);
    Floppy144IsometricProjectX16(nX16,nY16+nDepth16,nBaseHeight16,&dx,&dy);

    Floppy144IsometricProjectX16(nX16,nY16,nTopHeight16,&atx,&aty);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16,nTopHeight16,&btx,&bty);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16+nDepth16,nTopHeight16,&ctx,&cty);
    Floppy144IsometricProjectX16(nX16,nY16+nDepth16,nTopHeight16,&dtx,&dty);

    if(nTopHeight16<=nBaseHeight16)
    {
        Floppy144IsometricFillQuad(
            pSurface,ax,ay,bx,by,cx,cy,dx,dy,uTop
        );
        Floppy144IsometricLine(pSurface,ax,ay,bx,by,uEdge);
        Floppy144IsometricLine(pSurface,bx,by,cx,cy,uEdge);
        Floppy144IsometricLine(pSurface,cx,cy,dx,dy,uEdge);
        Floppy144IsometricLine(pSurface,dx,dy,ax,ay,uEdge);
        return;
    }

    /*
     * Painter-facing solid: top plus the +X and +Y faces. Back/base edges are
     * deliberately not redrawn after filling, so hidden geometry cannot shine
     * through the object as wireframe.
     */
    Floppy144IsometricFillQuad(
        pSurface,atx,aty,btx,bty,ctx,cty,dtx,dty,uTop
    );
    Floppy144IsometricFillQuad(
        pSurface,bx,by,cx,cy,ctx,cty,btx,bty,uFaceX
    );
    Floppy144IsometricFillQuad(
        pSurface,cx,cy,dx,dy,dtx,dty,ctx,cty,uFaceY
    );

    /* Top silhouette. */
    Floppy144IsometricLine(pSurface,atx,aty,btx,bty,uEdge);
    Floppy144IsometricLine(pSurface,btx,bty,ctx,cty,uEdge);
    Floppy144IsometricLine(pSurface,ctx,cty,dtx,dty,uEdge);
    Floppy144IsometricLine(pSurface,dtx,dty,atx,aty,uEdge);

    /* Only the camera-facing lower/vertical edges remain visible. */
    Floppy144IsometricLine(pSurface,bx,by,cx,cy,uEdge);
    Floppy144IsometricLine(pSurface,cx,cy,dx,dy,uEdge);
    Floppy144IsometricLine(pSurface,bx,by,btx,bty,uEdge);
    Floppy144IsometricLine(pSurface,cx,cy,ctx,cty,uEdge);
    Floppy144IsometricLine(pSurface,dx,dy,dtx,dty,uEdge);
}

static void Floppy144IsometricDrawPrismAlphaX16(
    Floppy144Surface *pSurface,
    int32_t nX16,
    int32_t nY16,
    int32_t nWidth16,
    int32_t nDepth16,
    int32_t nBaseHeight16,
    int32_t nTopHeight16,
    uint32_t uColour,
    uint32_t uAlpha
)
{
    const uint32_t uTop=
        Floppy144IsometricShadeColour(uColour,118U);
    const uint32_t uFaceX=
        Floppy144IsometricShadeColour(uColour,92U);
    const uint32_t uFaceY=
        Floppy144IsometricShadeColour(uColour,72U);
    int32_t ax,ay,bx,by,cx,cy,dx,dy;
    int32_t atx,aty,btx,bty,ctx,cty,dtx,dty;

    Floppy144IsometricProjectX16(nX16,nY16,nBaseHeight16,&ax,&ay);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16,nBaseHeight16,&bx,&by);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16+nDepth16,nBaseHeight16,&cx,&cy);
    Floppy144IsometricProjectX16(nX16,nY16+nDepth16,nBaseHeight16,&dx,&dy);

    Floppy144IsometricProjectX16(nX16,nY16,nTopHeight16,&atx,&aty);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16,nTopHeight16,&btx,&bty);
    Floppy144IsometricProjectX16(nX16+nWidth16,nY16+nDepth16,nTopHeight16,&ctx,&cty);
    Floppy144IsometricProjectX16(nX16,nY16+nDepth16,nTopHeight16,&dtx,&dty);

    Floppy144IsometricFillQuadAlpha(
        pSurface,atx,aty,btx,bty,ctx,cty,dtx,dty,uTop,uAlpha
    );
    Floppy144IsometricFillQuadAlpha(
        pSurface,bx,by,cx,cy,ctx,cty,btx,bty,uFaceX,uAlpha
    );
    Floppy144IsometricFillQuadAlpha(
        pSurface,cx,cy,dx,dy,dtx,dty,ctx,cty,uFaceY,uAlpha
    );
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

static void Floppy144IsometricDrawChair(
    Floppy144Surface *pSurface,
    const Floppy144SiteRect *pRect,
    uint32_t uColour
)
{
    int32_t x,y,w,d;
    int32_t sx,sy,sw,sd;
    int32_t leg;
    int32_t backThickness;
    int32_t backX,backY,backW,backD;
    uint32_t uRotation;

    if(pSurface==NULL||pRect==NULL)return;

    x=(int32_t)pRect->x*FLOPPY144_SITE_FIXED_ONE;
    y=(int32_t)pRect->y*FLOPPY144_SITE_FIXED_ONE;
    w=(int32_t)pRect->width*FLOPPY144_SITE_FIXED_ONE;
    d=(int32_t)pRect->height*FLOPPY144_SITE_FIXED_ONE;

    sx=x+w/8;
    sy=y+d/8;
    sw=w*3/4;
    sd=d*3/4;

    leg=FLOPPY144_SITE_FIXED_ONE/4;
    backThickness=FLOPPY144_SITE_FIXED_ONE/4;

    /* Four slim legs. */
    Floppy144IsometricDrawPrismX16(
        pSurface,sx,sy,leg,leg,0,
        2*FLOPPY144_SITE_FIXED_ONE,uColour
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,sx+sw-leg,sy,leg,leg,0,
        2*FLOPPY144_SITE_FIXED_ONE,uColour
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,sx,sy+sd-leg,leg,leg,0,
        2*FLOPPY144_SITE_FIXED_ONE,uColour
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,sx+sw-leg,sy+sd-leg,leg,leg,0,
        2*FLOPPY144_SITE_FIXED_ONE,uColour
    );

    /* Seat slab. */
    Floppy144IsometricDrawPrismX16(
        pSurface,
        sx,
        sy,
        sw,
        sd,
        2*FLOPPY144_SITE_FIXED_ONE,
        3*FLOPPY144_SITE_FIXED_ONE,
        uColour
    );

    /*
     * Backrest follows the authored chair rotation. Diagonal authored chairs
     * snap to their nearest cardinal back edge for this compact 2.5D recipe.
     */
    uRotation=((uint32_t)pRect->rotation+45U)/90U;
    uRotation=(uRotation%4U)*90U;

    backX=sx;
    backY=sy;
    backW=sw;
    backD=backThickness;

    if(uRotation==90U)
    {
        backX=sx+sw-backThickness;
        backY=sy;
        backW=backThickness;
        backD=sd;
    }
    else if(uRotation==180U)
    {
        backX=sx;
        backY=sy+sd-backThickness;
        backW=sw;
        backD=backThickness;
    }
    else if(uRotation==270U)
    {
        backX=sx;
        backY=sy;
        backW=backThickness;
        backD=sd;
    }

    Floppy144IsometricDrawPrismX16(
        pSurface,
        backX,
        backY,
        backW,
        backD,
        3*FLOPPY144_SITE_FIXED_ONE,
        6*FLOPPY144_SITE_FIXED_ONE,
        uColour
    );
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
    Floppy144SiteElement eElement;
    int32_t nHeight;
    uint32_t uColour;

    if(pSurface==NULL||pRect==NULL)return;

    eElement=(Floppy144SiteElement)pRect->type;
    nHeight=Floppy144IsometricElementHeight(eElement);
    uColour=Floppy144IsometricElementColour(eElement);
    pPlacement=Floppy144IsometricPlacementForRect(pRect);

    if(eElement==FLOPPY144_SITE_WALL_MOUNTED_ITEM)
    {
        Floppy144IsometricDrawWallFixture(
            pSurface,
            pRect,
            pPlacement
        );
        return;
    }

    if(eElement==FLOPPY144_SITE_CHAIR)
    {
        Floppy144IsometricDrawChair(
            pSurface,
            pRect,
            uColour
        );
        return;
    }

    if(
        eElement==FLOPPY144_SITE_SINK &&
        pRect->room==(uint8_t)FLOPPY144_ROOM_STAFF_ROOM
    )
    {
        /*
         * The sink owns its worktop-sized base, then adds the recessed bowl
         * and tap detail over the filled body.
         */
        Floppy144IsometricDrawPrismX16(
            pSurface,
            (int32_t)pRect->x*FLOPPY144_SITE_FIXED_ONE,
            (int32_t)pRect->y*FLOPPY144_SITE_FIXED_ONE,
            (int32_t)pRect->width*FLOPPY144_SITE_FIXED_ONE,
            (int32_t)pRect->height*FLOPPY144_SITE_FIXED_ONE,
            0,
            4*FLOPPY144_SITE_FIXED_ONE,
            uColour
        );
        Floppy144IsometricDrawSink(pSurface,pRect);
        return;
    }

    Floppy144IsometricDrawPrismX16(
        pSurface,
        (int32_t)pRect->x*FLOPPY144_SITE_FIXED_ONE,
        (int32_t)pRect->y*FLOPPY144_SITE_FIXED_ONE,
        (int32_t)pRect->width*FLOPPY144_SITE_FIXED_ONE,
        (int32_t)pRect->height*FLOPPY144_SITE_FIXED_ONE,
        0,
        nHeight*FLOPPY144_SITE_FIXED_ONE,
        uColour
    );

    if(
        eElement==FLOPPY144_SITE_BOOKCASE ||
        eElement==FLOPPY144_SITE_SHELVING_FULL
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

static bool Floppy144IsometricIsFloor(
    Floppy144SiteElement eElement
)
{
    return
        eElement>=FLOPPY144_SITE_FLOOR_A &&
        eElement<=FLOPPY144_SITE_FLOOR_D;
}

static bool Floppy144IsometricRectVisibleInRoom(
    const Floppy144RunState *pRunState,
    Floppy144RoomId eRoom,
    const Floppy144SiteRect *pRect
)
{
    if(
        pRect==NULL ||
        !Floppy144SiteRectRuntimeVisible(pRunState,pRect)
    )
    {
        return false;
    }

    if(pRect->from_room==pRect->to_room)
    {
        return pRect->room==(uint8_t)eRoom;
    }

    return
        pRect->from_room==(uint8_t)eRoom ||
        pRect->to_room==(uint8_t)eRoom;
}

static void Floppy144IsometricConfigureRoomProjection(
    Floppy144RoomId eRoom,
    const Floppy144RunState *pRunState
)
{
    Floppy144SiteRegion sBounds;

    /*
     * FM-23 now uses the same camera philosophy as normal Site exploration:
     * zoom into the active room and follow the player's canonical foot point.
     * Pixel writes remain clipped to the fixed 576x252 camera window.
     */
    g_nIsoOriginX=
        FLOPPY144_ISO_VIEWPORT_X+
        FLOPPY144_ISO_VIEWPORT_WIDTH/2;
    g_nIsoOriginY=
        FLOPPY144_ISO_VIEWPORT_Y+
        FLOPPY144_ISO_VIEWPORT_HEIGHT/2+
        12;

    g_nIsoHalfTileX=10;
    g_nIsoHalfTileY=4;
    g_nIsoHeightScale=6;

    if(
        pRunState!=NULL &&
        Floppy144SiteRoomBounds(eRoom,&sBounds)
    )
    {
        g_nIsoCentreX16=pRunState->player_site_x;
        g_nIsoCentreY16=pRunState->player_site_y;
        return;
    }

    if(!Floppy144SiteRoomBounds(eRoom,&sBounds))
    {
        return;
    }

    g_nIsoCentreX16=
        (int32_t)sBounds.x*FLOPPY144_SITE_FIXED_ONE+
        (
            (int32_t)sBounds.width*
            FLOPPY144_SITE_FIXED_ONE
        )/2;

    g_nIsoCentreY16=
        (int32_t)sBounds.y*FLOPPY144_SITE_FIXED_ONE+
        (
            (int32_t)sBounds.height*
            FLOPPY144_SITE_FIXED_ONE
        )/2;
}

static bool Floppy144IsometricBoundaryIsNearCutaway(
    Floppy144RoomId eRoom,
    const Floppy144SiteRect *pRect
)
{
    Floppy144SiteRegion sBounds;
    int32_t nRoomRight;
    int32_t nRoomBottom;
    int32_t nRectRight;
    int32_t nRectBottom;

    if(pRect==NULL||!Floppy144SiteRoomBounds(eRoom,&sBounds))
    {
        return false;
    }

    nRoomRight=(int32_t)sBounds.x+(int32_t)sBounds.width;
    nRoomBottom=(int32_t)sBounds.y+(int32_t)sBounds.height;
    nRectRight=(int32_t)pRect->x+(int32_t)pRect->width;
    nRectBottom=(int32_t)pRect->y+(int32_t)pRect->height;

    /*
     * With this projection, increasing X/Y moves toward the camera. Right and
     * bottom perimeter planes are therefore the two cutaway walls.
     */
    return
        nRectRight>=nRoomRight ||
        nRectBottom>=nRoomBottom;
}

static bool Floppy144IsometricRoomOwnsCell(
    Floppy144RoomId eRoom,
    int32_t nX,
    int32_t nY
)
{
    return
        nX>=0 &&
        nY>=0 &&
        nX<FLOPPY144_SITE_SIZE_UNITS &&
        nY<FLOPPY144_SITE_SIZE_UNITS &&
        Floppy144SiteRoomContainsCell(
            eRoom,
            (uint8_t)nX,
            (uint8_t)nY
        );
}

static void Floppy144IsometricDrawWallCell(
    Floppy144Surface *pSurface,
    int32_t nX,
    int32_t nY
)
{
    if(
        pSurface==NULL ||
        nX<0 ||
        nY<0 ||
        nX>=FLOPPY144_SITE_SIZE_UNITS ||
        nY>=FLOPPY144_SITE_SIZE_UNITS
    )
    {
        return;
    }

    Floppy144IsometricDrawPrismX16(
        pSurface,
        nX*FLOPPY144_SITE_FIXED_ONE,
        nY*FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_SITE_FIXED_ONE,
        0,
        FLOPPY144_SITE_WALL_HEIGHT_UNITS*
            FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_RGB(74,82,81)
    );
}

static void Floppy144IsometricDrawWallCellAlpha(
    Floppy144Surface *pSurface,
    int32_t nX,
    int32_t nY,
    uint32_t uAlpha
)
{
    if(
        pSurface==NULL ||
        nX<0 ||
        nY<0 ||
        nX>=FLOPPY144_SITE_SIZE_UNITS ||
        nY>=FLOPPY144_SITE_SIZE_UNITS
    )
    {
        return;
    }

    Floppy144IsometricDrawPrismAlphaX16(
        pSurface,
        nX*FLOPPY144_SITE_FIXED_ONE,
        nY*FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_SITE_FIXED_ONE,
        0,
        FLOPPY144_SITE_WALL_HEIGHT_UNITS*
            FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_RGB(74,82,81),
        uAlpha
    );
}

/*
 * Camera-side perimeter planes remain present as ghost walls. 38/255 is
 * approximately 15% opacity: enough to communicate enclosure while preserving
 * the player, furniture and interactions behind them.
 */
static void Floppy144IsometricDrawNearRoomWalls(
    Floppy144Surface *pSurface,
    Floppy144RoomId eRoom
)
{
    const uint32_t uAlpha=38U;
    uint32_t uIndex;
    uint32_t uCount=Floppy144SiteRectCount();

    for(uIndex=0U;uIndex<uCount;++uIndex)
    {
        const Floppy144SiteRect *pRect=Floppy144SiteRectAt(uIndex);
        int32_t x,y,x0,y0,x1,y1;

        if(
            pRect==NULL ||
            pRect->room!=(uint8_t)eRoom ||
            !Floppy144IsometricIsFloor(
                (Floppy144SiteElement)pRect->type
            )
        )
        {
            continue;
        }

        x0=(int32_t)pRect->x;
        y0=(int32_t)pRect->y;
        x1=x0+(int32_t)pRect->width;
        y1=y0+(int32_t)pRect->height;

        for(x=x0;x<x1;++x)
        {
            if(!Floppy144IsometricRoomOwnsCell(eRoom,x,y1))
            {
                Floppy144IsometricDrawWallCellAlpha(
                    pSurface,
                    x,
                    y1,
                    uAlpha
                );
            }
        }

        for(y=y0;y<y1;++y)
        {
            if(!Floppy144IsometricRoomOwnsCell(eRoom,x1,y))
            {
                Floppy144IsometricDrawWallCellAlpha(
                    pSurface,
                    x1,
                    y,
                    uAlpha
                );
            }
        }

        if(
            !Floppy144IsometricRoomOwnsCell(eRoom,x1,y1-1) &&
            !Floppy144IsometricRoomOwnsCell(eRoom,x1-1,y1) &&
            !Floppy144IsometricRoomOwnsCell(eRoom,x1,y1)
        )
        {
            Floppy144IsometricDrawWallCellAlpha(
                pSurface,
                x1,
                y1,
                uAlpha
            );
        }
    }
}

static void Floppy144IsometricDrawBoundaryAlpha(
    Floppy144Surface *pSurface,
    const Floppy144SiteRect *pRect,
    uint32_t uAlpha
)
{
    Floppy144SiteElement eElement;
    int32_t nHeight;

    if(pSurface==NULL||pRect==NULL)return;

    eElement=(Floppy144SiteElement)pRect->type;
    nHeight=Floppy144IsometricElementHeight(eElement);

    Floppy144IsometricDrawPrismAlphaX16(
        pSurface,
        (int32_t)pRect->x*FLOPPY144_SITE_FIXED_ONE,
        (int32_t)pRect->y*FLOPPY144_SITE_FIXED_ONE,
        (int32_t)pRect->width*FLOPPY144_SITE_FIXED_ONE,
        (int32_t)pRect->height*FLOPPY144_SITE_FIXED_ONE,
        0,
        nHeight*FLOPPY144_SITE_FIXED_ONE,
        Floppy144IsometricElementColour(eElement),
        uAlpha
    );
}

/*
 * Draw only the two perimeter planes furthest from the camera. The opposite
 * pair is deliberately absent so the active room reads as a playable cutaway
 * rather than a closed box.
 */
static void Floppy144IsometricDrawFarRoomWalls(
    Floppy144Surface *pSurface,
    Floppy144RoomId eRoom
)
{
    uint32_t uIndex;
    uint32_t uCount=Floppy144SiteRectCount();

    for(uIndex=0U;uIndex<uCount;++uIndex)
    {
        const Floppy144SiteRect *pRect=Floppy144SiteRectAt(uIndex);
        int32_t x,y,x0,y0,x1,y1;

        if(
            pRect==NULL ||
            pRect->room!=(uint8_t)eRoom ||
            !Floppy144IsometricIsFloor(
                (Floppy144SiteElement)pRect->type
            )
        )
        {
            continue;
        }

        x0=(int32_t)pRect->x;
        y0=(int32_t)pRect->y;
        x1=x0+(int32_t)pRect->width;
        y1=y0+(int32_t)pRect->height;

        for(x=x0;x<x1;++x)
        {
            if(!Floppy144IsometricRoomOwnsCell(eRoom,x,y0-1))
            {
                Floppy144IsometricDrawWallCell(
                    pSurface,
                    x,
                    y0-1
                );
            }
        }

        for(y=y0;y<y1;++y)
        {
            if(!Floppy144IsometricRoomOwnsCell(eRoom,x0-1,y))
            {
                Floppy144IsometricDrawWallCell(
                    pSurface,
                    x0-1,
                    y
                );
            }
        }

        if(
            !Floppy144IsometricRoomOwnsCell(eRoom,x0-1,y0) &&
            !Floppy144IsometricRoomOwnsCell(eRoom,x0,y0-1) &&
            !Floppy144IsometricRoomOwnsCell(eRoom,x0-1,y0-1)
        )
        {
            Floppy144IsometricDrawWallCell(
                pSurface,
                x0-1,
                y0-1
            );
        }
    }
}

static int32_t Floppy144IsometricRectDepth(
    const Floppy144SiteRect *pRect
)
{
    if(pRect==NULL)return 0;

    return
        (
            (
                (int32_t)pRect->x*2+
                (int32_t)pRect->width+
                (int32_t)pRect->y*2+
                (int32_t)pRect->height
            )*
            FLOPPY144_SITE_FIXED_ONE
        )/2;
}

static void Floppy144IsometricDrawPlayer(
    Floppy144Surface *pSurface,
    const Floppy144RunState *pRunState
)
{
    const uint32_t uBody=FLOPPY144_RGB(100,156,111);
    const uint32_t uHead=FLOPPY144_RGB(176,170,148);
    const int32_t q=FLOPPY144_SITE_FIXED_ONE/4;
    const int32_t h=FLOPPY144_SITE_FIXED_ONE/2;
    int32_t x,y;

    if(pSurface==NULL||pRunState==NULL)return;

    x=pRunState->player_site_x;
    y=pRunState->player_site_y;

    /* Feet and separate legs establish a readable stance. */
    Floppy144IsometricDrawPrismX16(
        pSurface,x-h,y-h,q,h,0,q,uBody
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,x+q,y-h,q,h,0,q,uBody
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,x-h,y-q,q,q,
        q,
        2*FLOPPY144_SITE_FIXED_ONE,
        uBody
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,x+q,y-q,q,q,
        q,
        2*FLOPPY144_SITE_FIXED_ONE,
        uBody
    );

    /* Torso. */
    Floppy144IsometricDrawPrismX16(
        pSurface,
        x-h,
        y-h,
        FLOPPY144_SITE_FIXED_ONE,
        FLOPPY144_SITE_FIXED_ONE,
        2*FLOPPY144_SITE_FIXED_ONE,
        4*FLOPPY144_SITE_FIXED_ONE,
        uBody
    );

    /* Arms sit slightly proud of the torso. */
    Floppy144IsometricDrawPrismX16(
        pSurface,
        x-h-q,
        y-q,
        q,
        h,
        2*FLOPPY144_SITE_FIXED_ONE,
        4*FLOPPY144_SITE_FIXED_ONE,
        uBody
    );
    Floppy144IsometricDrawPrismX16(
        pSurface,
        x+h,
        y-q,
        q,
        h,
        2*FLOPPY144_SITE_FIXED_ONE,
        4*FLOPPY144_SITE_FIXED_ONE,
        uBody
    );

    /* Head. */
    Floppy144IsometricDrawPrismX16(
        pSurface,
        x-q,
        y-q,
        h,
        h,
        4*FLOPPY144_SITE_FIXED_ONE,
        5*FLOPPY144_SITE_FIXED_ONE,
        uHead
    );
}


void Floppy144SiteIsometricDraw(
    F144Runtime *pRuntime,
    const Floppy144RunState *pRunState,
    const char *pszNotice
)
{
    const uint32_t uBackground=FLOPPY144_RGB(12,17,21);
    const uint32_t uViewport=FLOPPY144_RGB(18,24,27);
    const uint32_t uFrame=FLOPPY144_RGB(74,82,81);
    const uint32_t uFrameEdge=FLOPPY144_RGB(113,124,120);
    const uint32_t uText=FLOPPY144_RGB(201,210,203);
    const uint32_t uMuted=FLOPPY144_RGB(116,132,130);
    const uint32_t uGreen=FLOPPY144_RGB(100,156,111);
    const uint32_t uAmber=FLOPPY144_RGB(194,153,76);

    Floppy144Surface sSurface;
    Floppy144RoomId eActiveRoom;
    const char *pszRoomLabel;
    const char *pszContextLabel=NULL;
    const char *pszPrompt;
    char szStatus[64];
    uint16_t auDepthRects[FLOPPY144_ISO_SORT_MAX];
    uint32_t uDepthCount=0U;
    uint32_t uRectCount;
    uint32_t uIndex;
    bool bRoomReconstructed;
    bool bPlayerDrawn=false;
    int32_t nPlayerDepth;

    if(
        pRuntime==NULL ||
        pRunState==NULL ||
        pRuntime->backbuffer.data==NULL
    )
    {
        return;
    }

    sSurface.pixels=(uint32_t *)pRuntime->backbuffer.data;
    sSurface.width=pRuntime->backbuffer.width;
    sSurface.height=pRuntime->backbuffer.height;

    eActiveRoom=Floppy144SiteRoomAtPosition(
        pRunState->player_site_x,
        pRunState->player_site_y
    );

    if(eActiveRoom==FLOPPY144_ROOM_COUNT)
    {
        eActiveRoom=FLOPPY144_ROOM_RECEPTION;
    }

    bRoomReconstructed=
        Floppy144RunStateRoomReconstructed(
            pRunState,
            eActiveRoom
        );

    pszRoomLabel=
        bRoomReconstructed
            ? Floppy144IsometricRoomLabel(eActiveRoom)
            : "ROOM DATA NOT RECONSTRUCTED";

    if(pszNotice!=NULL)
    {
        pszRoomLabel=pszNotice;
    }
    else if(bRoomReconstructed)
    {
        pszContextLabel=Floppy144SiteContextLabel(pRunState);

        if(pszContextLabel!=NULL)
        {
            pszRoomLabel=pszContextLabel;
        }
    }

    pszPrompt=Floppy144IsometricInteractionPrompt(pRunState);

    Floppy144RunStateFormatCapacity(
        pRunState,
        szStatus,
        (uint32_t)sizeof(szStatus)
    );

    Floppy144DrawClear(&sSurface,uBackground);

    Floppy144DrawText(
        &sSurface,
        10U,
        5U,
        "GDR SITE RECONSTRUCTION // ISOMETRIC 2.5D",
        1U,
        uMuted
    );

    Floppy144DrawText(
        &sSurface,
        630U-Floppy144DrawTextWidth(szStatus,1U),
        5U,
        szStatus,
        1U,
        uGreen
    );

    Floppy144DrawFillRect(
        &sSurface,
        20U,
        20U,
        600U,
        276U,
        uFrame
    );

    Floppy144DrawRect(
        &sSurface,
        20U,
        20U,
        600U,
        276U,
        uFrameEdge
    );

    Floppy144DrawFillRect(
        &sSurface,
        FLOPPY144_ISO_VIEWPORT_X,
        FLOPPY144_ISO_VIEWPORT_Y,
        FLOPPY144_ISO_VIEWPORT_WIDTH,
        FLOPPY144_ISO_VIEWPORT_HEIGHT,
        uViewport
    );

    if(bRoomReconstructed)
    {
        Floppy144IsometricConfigureRoomProjection(
            eActiveRoom,
            pRunState
        );
        uRectCount=Floppy144SiteRectCount();

        /*
         * Z0: active-room floor only. FM-23 no longer reveals the reconstructed
         * Site as a whole; it mirrors the 2D room-selection contract.
         */
        for(uIndex=0U;uIndex<uRectCount;++uIndex)
        {
            const Floppy144SiteRect *pRect=
                Floppy144SiteRectAt(uIndex);

            if(
                pRect!=NULL &&
                Floppy144IsometricRectVisibleInRoom(
                    pRunState,
                    eActiveRoom,
                    pRect
                ) &&
                Floppy144IsometricIsFloor(
                    (Floppy144SiteElement)pRect->type
                )
            )
            {
                Floppy144IsometricDrawRect(
                    &sSurface,
                    pRect
                );
            }
        }

        /*
         * Z1: opaque far room shell. Right/bottom walls are the camera-side
         * cutaway and are composited later at 15% opacity.
         */
        Floppy144IsometricDrawFarRoomWalls(
            &sSurface,
            eActiveRoom
        );

        /*
         * Z2: boundary furniture and wall-mounted fittings. Door/window
         * rectangles replace the far wall beneath them. A fitting remains on
         * its wall plane and is therefore never a walk-behind object.
         */
        for(uIndex=0U;uIndex<uRectCount;++uIndex)
        {
            const Floppy144SiteRect *pRect=
                Floppy144SiteRectAt(uIndex);
            Floppy144SiteElement eElement;

            if(
                pRect==NULL ||
                !Floppy144IsometricRectVisibleInRoom(
                    pRunState,
                    eActiveRoom,
                    pRect
                )
            )
            {
                continue;
            }

            eElement=(Floppy144SiteElement)pRect->type;

            if(eElement==FLOPPY144_SITE_WALL_MOUNTED_ITEM)
            {
                Floppy144IsometricDrawRect(
                    &sSurface,
                    pRect
                );
                continue;
            }

            if(
                eElement==FLOPPY144_SITE_DOOR ||
                eElement==FLOPPY144_SITE_WINDOW
            )
            {
                if(
                    !Floppy144IsometricBoundaryIsNearCutaway(
                        eActiveRoom,
                        pRect
                    )
                )
                {
                    Floppy144IsometricDrawRect(
                        &sSurface,
                        pRect
                    );
                }
                continue;
            }

            if(
                Floppy144IsometricIsFloor(eElement) ||
                uDepthCount>=FLOPPY144_ISO_SORT_MAX
            )
            {
                continue;
            }

            auDepthRects[uDepthCount++]=(uint16_t)uIndex;
        }

        /*
         * Sort freestanding furniture/partitions by floor depth. This is the
         * visual Z-space which lets the same canonical player position pass
         * behind or in front of an object without changing collision geometry.
         */
        for(uIndex=1U;uIndex<uDepthCount;++uIndex)
        {
            uint16_t uValue=auDepthRects[uIndex];
            int32_t nValueDepth=
                Floppy144IsometricRectDepth(
                    Floppy144SiteRectAt((uint32_t)uValue)
                );
            uint32_t uInsert=uIndex;

            while(uInsert>0U)
            {
                uint16_t uPrevious=auDepthRects[uInsert-1U];
                int32_t nPreviousDepth=
                    Floppy144IsometricRectDepth(
                        Floppy144SiteRectAt(
                            (uint32_t)uPrevious
                        )
                    );

                if(nPreviousDepth<=nValueDepth)
                {
                    break;
                }

                auDepthRects[uInsert]=uPrevious;
                --uInsert;
            }

            auDepthRects[uInsert]=uValue;
        }

        nPlayerDepth=
            pRunState->player_site_x+
            pRunState->player_site_y;

        for(uIndex=0U;uIndex<uDepthCount;++uIndex)
        {
            const Floppy144SiteRect *pRect=
                Floppy144SiteRectAt(
                    (uint32_t)auDepthRects[uIndex]
                );
            int32_t nRectDepth=
                Floppy144IsometricRectDepth(pRect);

            if(!bPlayerDrawn&&nPlayerDepth<nRectDepth)
            {
                Floppy144IsometricDrawPlayer(
                    &sSurface,
                    pRunState
                );
                bPlayerDrawn=true;
            }

            Floppy144IsometricDrawRect(
                &sSurface,
                pRect
            );
        }

        if(!bPlayerDrawn)
        {
            Floppy144IsometricDrawPlayer(
                &sSurface,
                pRunState
            );
        }

        /*
         * Z3: translucent near enclosure. The wall itself is 15% opaque and
         * near-side doors/windows use the same alpha so they remain legible
         * without hiding the playable room behind them.
         */
        Floppy144IsometricDrawNearRoomWalls(
            &sSurface,
            eActiveRoom
        );

        for(uIndex=0U;uIndex<uRectCount;++uIndex)
        {
            const Floppy144SiteRect *pRect=
                Floppy144SiteRectAt(uIndex);
            Floppy144SiteElement eElement;

            if(
                pRect==NULL ||
                !Floppy144IsometricRectVisibleInRoom(
                    pRunState,
                    eActiveRoom,
                    pRect
                )
            )
            {
                continue;
            }

            eElement=(Floppy144SiteElement)pRect->type;

            if(
                (
                    eElement==FLOPPY144_SITE_DOOR ||
                    eElement==FLOPPY144_SITE_WINDOW
                ) &&
                Floppy144IsometricBoundaryIsNearCutaway(
                    eActiveRoom,
                    pRect
                )
            )
            {
                Floppy144IsometricDrawBoundaryAlpha(
                    &sSurface,
                    pRect,
                    38U
                );
            }
        }
    }

    /* Mask and reassert the same camera/frame shell used by normal Site play. */
    Floppy144DrawRect(
        &sSurface,
        FLOPPY144_ISO_VIEWPORT_X-1U,
        FLOPPY144_ISO_VIEWPORT_Y-1U,
        FLOPPY144_ISO_VIEWPORT_WIDTH+2U,
        FLOPPY144_ISO_VIEWPORT_HEIGHT+2U,
        uFrameEdge
    );

    Floppy144DrawText(
        &sSurface,
        36U,
        300U,
        pszRoomLabel,
        1U,
        pszNotice!=NULL ||
        pszContextLabel!=NULL ||
        !bRoomReconstructed
            ? uAmber
            : uMuted
    );

    Floppy144DrawFillRect(
        &sSurface,
        20U,
        312U,
        600U,
        28U,
        uBackground
    );

    Floppy144DrawRect(
        &sSurface,
        20U,
        312U,
        600U,
        28U,
        uFrameEdge
    );

    Floppy144DrawText(
        &sSurface,
        32U,
        322U,
        pszPrompt,
        1U,
        uText
    );

    Floppy144DrawText(
        &sSurface,
        526U,
        322U,
        "ESC RECOVERY",
        1U,
        uMuted
    );
}
