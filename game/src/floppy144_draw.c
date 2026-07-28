#include "floppy144_draw.h"

#include <stddef.h>

#define FLOPPY144_GLYPH(row0, row1, row2, row3, row4, row5, row6) \
    ((uint64_t)(row0)       | \
    ((uint64_t)(row1) << 5)  | \
    ((uint64_t)(row2) << 10) | \
    ((uint64_t)(row3) << 15) | \
    ((uint64_t)(row4) << 20) | \
    ((uint64_t)(row5) << 25) | \
    ((uint64_t)(row6) << 30))

static uint64_t Floppy144Glyph(
    char character
)
{
    switch(character)
    {
        case 'A': return FLOPPY144_GLYPH(14, 17, 17, 31, 17, 17, 17);
        case 'B': return FLOPPY144_GLYPH(30, 17, 17, 30, 17, 17, 30);
        case 'C': return FLOPPY144_GLYPH(15, 16, 16, 16, 16, 16, 15);
        case 'D': return FLOPPY144_GLYPH(30, 17, 17, 17, 17, 17, 30);
        case 'E': return FLOPPY144_GLYPH(31, 16, 16, 30, 16, 16, 31);
        case 'F': return FLOPPY144_GLYPH(31, 16, 16, 30, 16, 16, 16);
        case 'G': return FLOPPY144_GLYPH(15, 16, 16, 23, 17, 17, 15);
        case 'H': return FLOPPY144_GLYPH(17, 17, 17, 31, 17, 17, 17);
        case 'I': return FLOPPY144_GLYPH(31, 4, 4, 4, 4, 4, 31);
        case 'J': return FLOPPY144_GLYPH(7, 2, 2, 2, 18, 18, 12);
        case 'K': return FLOPPY144_GLYPH(17, 18, 20, 24, 20, 18, 17);
        case 'L': return FLOPPY144_GLYPH(16, 16, 16, 16, 16, 16, 31);
        case 'M': return FLOPPY144_GLYPH(17, 27, 21, 21, 17, 17, 17);
        case 'N': return FLOPPY144_GLYPH(17, 25, 21, 19, 17, 17, 17);
        case 'O': return FLOPPY144_GLYPH(14, 17, 17, 17, 17, 17, 14);
        case 'P': return FLOPPY144_GLYPH(30, 17, 17, 30, 16, 16, 16);
        case 'Q': return FLOPPY144_GLYPH(14, 17, 17, 17, 21, 18, 13);
        case 'R': return FLOPPY144_GLYPH(30, 17, 17, 30, 20, 18, 17);
        case 'S': return FLOPPY144_GLYPH(15, 16, 16, 14, 1, 1, 30);
        case 'T': return FLOPPY144_GLYPH(31, 4, 4, 4, 4, 4, 4);
        case 'U': return FLOPPY144_GLYPH(17, 17, 17, 17, 17, 17, 14);
        case 'V': return FLOPPY144_GLYPH(17, 17, 17, 17, 17, 10, 4);
        case 'W': return FLOPPY144_GLYPH(17, 17, 17, 21, 21, 27, 17);
        case 'X': return FLOPPY144_GLYPH(17, 17, 10, 4, 10, 17, 17);
        case 'Y': return FLOPPY144_GLYPH(17, 17, 10, 4, 4, 4, 4);
        case 'Z': return FLOPPY144_GLYPH(31, 1, 2, 4, 8, 16, 31);

        case '0': return FLOPPY144_GLYPH(14, 17, 19, 21, 25, 17, 14);
        case '1': return FLOPPY144_GLYPH(4, 12, 4, 4, 4, 4, 14);
        case '2': return FLOPPY144_GLYPH(14, 17, 1, 2, 4, 8, 31);
        case '3': return FLOPPY144_GLYPH(30, 1, 1, 14, 1, 1, 30);
        case '4': return FLOPPY144_GLYPH(2, 6, 10, 18, 31, 2, 2);
        case '5': return FLOPPY144_GLYPH(31, 16, 16, 30, 1, 1, 30);
        case '6': return FLOPPY144_GLYPH(14, 16, 16, 30, 17, 17, 14);
        case '7': return FLOPPY144_GLYPH(31, 1, 2, 4, 8, 8, 8);
        case '8': return FLOPPY144_GLYPH(14, 17, 17, 14, 17, 17, 14);
        case '9': return FLOPPY144_GLYPH(14, 17, 17, 15, 1, 1, 14);

        case '-': return FLOPPY144_GLYPH(0, 0, 0, 31, 0, 0, 0);
        case ':': return FLOPPY144_GLYPH(0, 4, 4, 0, 4, 4, 0);
        case '.': return FLOPPY144_GLYPH(0, 0, 0, 0, 0, 4, 4);
        case '/': return FLOPPY144_GLYPH(1, 2, 2, 4, 8, 8, 16);
        case '%': return FLOPPY144_GLYPH(17, 2, 4, 8, 16, 17, 0);
        case '>': return FLOPPY144_GLYPH(16, 8, 4, 2, 4, 8, 16);
        case ' ': return 0;

        default: return FLOPPY144_GLYPH(14, 17, 2, 4, 0, 4, 0);
    }
}

void Floppy144DrawClear(
    Floppy144Surface *surface,
    uint32_t colour
)
{
    uint64_t pixel_count;
    uint64_t pixel_index;

    pixel_count =
        (uint64_t)surface->width *
        (uint64_t)surface->height;

    for(pixel_index = 0; pixel_index < pixel_count; ++pixel_index)
    {
        surface->pixels[pixel_index] = colour;
    }
}

void Floppy144DrawFillRect(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t colour
)
{
    uint32_t end_x;
    uint32_t end_y;
    uint32_t draw_x;
    uint32_t draw_y;

    if(
        width == 0 ||
        height == 0 ||
        x >= surface->width ||
        y >= surface->height
    )
    {
        return;
    }

    end_x = x + width;
    end_y = y + height;

    if(end_x > surface->width)
    {
        end_x = surface->width;
    }

    if(end_y > surface->height)
    {
        end_y = surface->height;
    }

    for(draw_y = y; draw_y < end_y; ++draw_y)
    {
        for(draw_x = x; draw_x < end_x; ++draw_x)
        {
            surface->pixels[
                (uint64_t)draw_y * surface->width + draw_x
            ] = colour;
        }
    }
}

void Floppy144DrawRect(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t colour
)
{
    if(width == 0 || height == 0)
    {
        return;
    }

    Floppy144DrawFillRect(
        surface,
        x,
        y,
        width,
        1,
        colour
    );

    Floppy144DrawFillRect(
        surface,
        x,
        y + height - 1,
        width,
        1,
        colour
    );

    Floppy144DrawFillRect(
        surface,
        x,
        y,
        1,
        height,
        colour
    );

    Floppy144DrawFillRect(
        surface,
        x + width - 1,
        y,
        1,
        height,
        colour
    );
}

uint32_t Floppy144DrawTextWidth(
    const char *text,
    uint32_t scale
)
{
    uint32_t character_count = 0;

    while(text[character_count] != '\0')
    {
        ++character_count;
    }

    if(character_count == 0)
    {
        return 0;
    }

    return character_count * 6 * scale - scale;
}

void Floppy144DrawText(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    const char *text,
    uint32_t scale,
    uint32_t colour
)
{
    uint32_t character_index = 0;

    while(text[character_index] != '\0')
    {
        char character = text[character_index];
        uint64_t glyph;
        uint32_t glyph_row;
        uint32_t glyph_column;

        if(character >= 'a' && character <= 'z')
        {
            character =
                (char)(character - ('a' - 'A'));
        }

        glyph = Floppy144Glyph(character);

        for(glyph_row = 0; glyph_row < 7; ++glyph_row)
        {
            uint32_t row_bits =
                (uint32_t)((glyph >> (glyph_row * 5)) & 31);

            for(glyph_column = 0;
                glyph_column < 5;
                ++glyph_column)
            {
                uint32_t mask =
                    1U << (4U - glyph_column);

                if((row_bits & mask) != 0)
                {
                    Floppy144DrawFillRect(
                        surface,
                        x + glyph_column * scale,
                        y + glyph_row * scale,
                        scale,
                        scale,
                        colour
                    );
                }
            }
        }

        x += 6 * scale;
        ++character_index;
    }
}
