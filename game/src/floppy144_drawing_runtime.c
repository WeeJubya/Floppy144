#include "floppy144_drawing_runtime.h"
#include "floppy144_game_data.h"

#include <stddef.h>
#include <string.h>

typedef struct Floppy144DrawingClip
{
    int32_t nX0;
    int32_t nY0;
    int32_t nX1;
    int32_t nY1;
}
Floppy144DrawingClip;

static int32_t Floppy144DrawingScale(
    int32_t nOrigin,
    int32_t nExtent,
    int32_t nValue
)
{
    return nOrigin + (nExtent * nValue) / 100;
}

static uint32_t Floppy144DrawingColour(
    const char *pszId
)
{
    uint32_t uIndex;

    for(
        uIndex = 0U;
        uIndex < Floppy144GameDataRecordCount();
        ++uIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uIndex);

        if(
            pRecord != NULL &&
            pRecord->eKind == FLOPPY144_DATA_COLOUR &&
            pRecord->pszId != NULL &&
            pszId != NULL &&
            strcmp(pRecord->pszId, pszId) == 0
        )
        {
            return FLOPPY144_RGB(
                (uint32_t)pRecord->n0,
                (uint32_t)pRecord->n1,
                (uint32_t)pRecord->n2
            );
        }
    }

    return FLOPPY144_RGB(202U, 211U, 205U);
}

static bool Floppy144DrawingBuildClip(
    const Floppy144Surface *pSurface,
    int32_t nClipX,
    int32_t nClipY,
    int32_t nClipWidth,
    int32_t nClipHeight,
    Floppy144DrawingClip *pClip
)
{
    int32_t nX0;
    int32_t nY0;
    int32_t nX1;
    int32_t nY1;

    if(
        pSurface == NULL ||
        pSurface->pixels == NULL ||
        pClip == NULL ||
        nClipWidth <= 0 ||
        nClipHeight <= 0
    )
    {
        return false;
    }

    nX0 = nClipX;
    nY0 = nClipY;
    nX1 = nClipX + nClipWidth;
    nY1 = nClipY + nClipHeight;

    if(nX0 < 0)
        nX0 = 0;
    if(nY0 < 0)
        nY0 = 0;

    if(nX1 > (int32_t)pSurface->width)
        nX1 = (int32_t)pSurface->width;
    if(nY1 > (int32_t)pSurface->height)
        nY1 = (int32_t)pSurface->height;

    if(nX1 <= nX0 || nY1 <= nY0)
        return false;

    pClip->nX0 = nX0;
    pClip->nY0 = nY0;
    pClip->nX1 = nX1;
    pClip->nY1 = nY1;

    return true;
}

static void Floppy144DrawingFill(
    Floppy144Surface *pSurface,
    const Floppy144DrawingClip *pClip,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight,
    uint32_t uColour
)
{
    int32_t nX0;
    int32_t nY0;
    int32_t nX1;
    int32_t nY1;

    if(
        pSurface == NULL ||
        pClip == NULL ||
        nWidth <= 0 ||
        nHeight <= 0
    )
    {
        return;
    }

    nX0 = nX;
    nY0 = nY;
    nX1 = nX + nWidth;
    nY1 = nY + nHeight;

    if(nX0 < pClip->nX0)
        nX0 = pClip->nX0;
    if(nY0 < pClip->nY0)
        nY0 = pClip->nY0;
    if(nX1 > pClip->nX1)
        nX1 = pClip->nX1;
    if(nY1 > pClip->nY1)
        nY1 = pClip->nY1;

    if(nX1 <= nX0 || nY1 <= nY0)
        return;

    Floppy144DrawFillRect(
        pSurface,
        (uint32_t)nX0,
        (uint32_t)nY0,
        (uint32_t)(nX1 - nX0),
        (uint32_t)(nY1 - nY0),
        uColour
    );
}

static void Floppy144DrawingStroke(
    Floppy144Surface *pSurface,
    const Floppy144DrawingClip *pClip,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight,
    uint32_t uColour
)
{
    if(nWidth <= 0 || nHeight <= 0)
        return;

    /*
     * Clip each authored edge independently. Clipping the rectangle first and
     * then stroking it would invent a new border on the camera edge, making a
     * partially visible object appear to resize.
     */
    Floppy144DrawingFill(
        pSurface,
        pClip,
        nX,
        nY,
        nWidth,
        1,
        uColour
    );

    if(nHeight > 1)
    {
        Floppy144DrawingFill(
            pSurface,
            pClip,
            nX,
            nY + nHeight - 1,
            nWidth,
            1,
            uColour
        );
    }

    if(nHeight > 2)
    {
        Floppy144DrawingFill(
            pSurface,
            pClip,
            nX,
            nY + 1,
            1,
            nHeight - 2,
            uColour
        );

        if(nWidth > 1)
        {
            Floppy144DrawingFill(
                pSurface,
                pClip,
                nX + nWidth - 1,
                nY + 1,
                1,
                nHeight - 2,
                uColour
            );
        }
    }
}

static void Floppy144DrawingPixel(
    Floppy144Surface *pSurface,
    const Floppy144DrawingClip *pClip,
    int32_t nX,
    int32_t nY,
    uint32_t uColour
)
{
    Floppy144DrawingFill(
        pSurface,
        pClip,
        nX,
        nY,
        1,
        1,
        uColour
    );
}

static void Floppy144DrawingLine(
    Floppy144Surface *pSurface,
    const Floppy144DrawingClip *pClip,
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

        Floppy144DrawingPixel(
            pSurface,
            pClip,
            nX0,
            nY0,
            uColour
        );

        if(nX0 == nX1 && nY0 == nY1)
            break;

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

static void Floppy144DrawingEllipse(
    Floppy144Surface *pSurface,
    const Floppy144DrawingClip *pClip,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight,
    uint32_t uColour
)
{
    int32_t nCentreX = nX + nWidth / 2;
    int32_t nCentreY = nY + nHeight / 2;
    int32_t nRadiusX = nWidth / 2;
    int32_t nRadiusY = nHeight / 2;
    int32_t nStep;

    for(nStep = 0; nStep < 64; ++nStep)
    {
        int32_t nQuadrant = nStep / 16;
        int32_t nT = nStep % 16;
        int32_t nOffsetX;
        int32_t nOffsetY;

        switch(nQuadrant)
        {
            case 0:
                nOffsetX =
                    nRadiusX -
                    (nRadiusX * nT * nT) / 256;
                nOffsetY =
                    (nRadiusY * nT) / 16;
                break;

            case 1:
                nOffsetX =
                    -(nRadiusX * nT) / 16;
                nOffsetY =
                    nRadiusY -
                    (nRadiusY * nT * nT) / 256;
                break;

            case 2:
                nOffsetX =
                    -nRadiusX +
                    (nRadiusX * nT * nT) / 256;
                nOffsetY =
                    -(nRadiusY * nT) / 16;
                break;

            default:
                nOffsetX =
                    (nRadiusX * nT) / 16;
                nOffsetY =
                    -nRadiusY +
                    (nRadiusY * nT * nT) / 256;
                break;
        }

        Floppy144DrawingPixel(
            pSurface,
            pClip,
            nCentreX + nOffsetX,
            nCentreY + nOffsetY,
            uColour
        );
    }
}

static bool Floppy144DrawingRuntimeDrawInternal(
    Floppy144Surface *pSurface,
    const char *pszDrawingId,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight,
    const Floppy144DrawingClip *pClip
)
{
    uint32_t uIndex;
    bool bFound = false;

    if(
        pSurface == NULL ||
        pszDrawingId == NULL ||
        pClip == NULL ||
        nWidth <= 0 ||
        nHeight <= 0
    )
    {
        return false;
    }

    for(
        uIndex = 0U;
        uIndex < Floppy144GameDataRecordCount();
        ++uIndex
    )
    {
        const Floppy144DataRecord *pRecord =
            Floppy144GameDataRecordAt(uIndex);

        uint32_t uColour;
        int32_t nPrimitiveX;
        int32_t nPrimitiveY;
        int32_t nPrimitiveWidth;
        int32_t nPrimitiveHeight;

        if(
            pRecord == NULL ||
            pRecord->eKind != FLOPPY144_DATA_DRAWING_PRIMITIVE ||
            pRecord->pszId == NULL ||
            strcmp(pRecord->pszId, pszDrawingId) != 0
        )
        {
            continue;
        }

        bFound = true;
        uColour = Floppy144DrawingColour(pRecord->pszB);

        nPrimitiveX =
            Floppy144DrawingScale(
                nX,
                nWidth,
                pRecord->n0
            );

        nPrimitiveY =
            Floppy144DrawingScale(
                nY,
                nHeight,
                pRecord->n1
            );

        nPrimitiveWidth =
            (nWidth * pRecord->n2) / 100;

        nPrimitiveHeight =
            (nHeight * pRecord->n3) / 100;

        if(strcmp(pRecord->pszA, "fill_rect") == 0)
        {
            Floppy144DrawingFill(
                pSurface,
                pClip,
                nPrimitiveX,
                nPrimitiveY,
                nPrimitiveWidth,
                nPrimitiveHeight,
                uColour
            );
        }
        else if(strcmp(pRecord->pszA, "stroke_rect") == 0)
        {
            Floppy144DrawingStroke(
                pSurface,
                pClip,
                nPrimitiveX,
                nPrimitiveY,
                nPrimitiveWidth,
                nPrimitiveHeight,
                uColour
            );
        }
        else if(strcmp(pRecord->pszA, "line") == 0)
        {
            Floppy144DrawingLine(
                pSurface,
                pClip,
                Floppy144DrawingScale(
                    nX,
                    nWidth,
                    pRecord->n0
                ),
                Floppy144DrawingScale(
                    nY,
                    nHeight,
                    pRecord->n1
                ),
                Floppy144DrawingScale(
                    nX,
                    nWidth,
                    pRecord->n2
                ),
                Floppy144DrawingScale(
                    nY,
                    nHeight,
                    pRecord->n3
                ),
                uColour
            );
        }
        else if(strcmp(pRecord->pszA, "circle") == 0)
        {
            int32_t nRadius =
                (
                    nWidth < nHeight
                        ? nWidth
                        : nHeight
                ) *
                pRecord->n2 /
                100;

            Floppy144DrawingEllipse(
                pSurface,
                pClip,
                nPrimitiveX - nRadius,
                nPrimitiveY - nRadius,
                nRadius * 2,
                nRadius * 2,
                uColour
            );
        }
        else if(strcmp(pRecord->pszA, "ellipse") == 0)
        {
            Floppy144DrawingEllipse(
                pSurface,
                pClip,
                nPrimitiveX,
                nPrimitiveY,
                nPrimitiveWidth,
                nPrimitiveHeight,
                uColour
            );
        }
    }

    return bFound;
}

bool Floppy144DrawingRuntimeDraw(
    Floppy144Surface *pSurface,
    const char *pszDrawingId,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight
)
{
    if(pSurface == NULL)
        return false;

    return
        Floppy144DrawingRuntimeDrawClipped(
            pSurface,
            pszDrawingId,
            nX,
            nY,
            nWidth,
            nHeight,
            0,
            0,
            (int32_t)pSurface->width,
            (int32_t)pSurface->height
        );
}

bool Floppy144DrawingRuntimeDrawClipped(
    Floppy144Surface *pSurface,
    const char *pszDrawingId,
    int32_t nX,
    int32_t nY,
    int32_t nWidth,
    int32_t nHeight,
    int32_t nClipX,
    int32_t nClipY,
    int32_t nClipWidth,
    int32_t nClipHeight
)
{
    Floppy144DrawingClip sClip;

    if(
        !Floppy144DrawingBuildClip(
            pSurface,
            nClipX,
            nClipY,
            nClipWidth,
            nClipHeight,
            &sClip
        )
    )
    {
        return false;
    }

    return
        Floppy144DrawingRuntimeDrawInternal(
            pSurface,
            pszDrawingId,
            nX,
            nY,
            nWidth,
            nHeight,
            &sClip
        );
}
