/*
 * S4E-05: six authored, intersecting miniature crosswords.
 * This is atmosphere, never a game challenge, trigger or evidence source.
 */
#include "floppy144_crossword.h"
#include "floppy144_variation.h"

#include <stddef.h>
#include <string.h>

typedef struct Floppy144CrosswordPattern
{
    const char *across;
    const char *down;
    const char *across_clue;
    const char *down_clue;
} Floppy144CrosswordPattern;

/* All words intersect at the central (2,2) square. No story secrets. */
static const Floppy144CrosswordPattern patterns[] =
{
    { "FORMS", "CARDS", "A: MANDATORY PAPERWORK (5)",
      "D: BOXED INDEX SLIPS (5)" },
    { "STAMP", "DRAFT", "A: APPROVAL BY INK (5)",
      "D: NOT QUITE FINAL (5)" },
    { "DESKS", "ASSET", "A: OFFICIAL ELBOW RESTS (5)",
      "D: ITEM WITH AN INVENTORY TAG (5)" },
    { "QUEUE", "SHELF", "A: LINE REQUIRING A FORM (5)",
      "D: STORAGE WITHOUT DRAWERS (5)" },
    { "ORDER", "INDEX", "A: A COUNTERSIGNED INSTRUCTION (5)",
      "D: AN ALPHABETISED LIST (5)" },
    { "PAPER", "TYPOS", "A: MEDIUM OF ADMINISTRATIVE DELAY (5)",
      "D: ERRORS ON AN APPROVED FORM (5)" }
};

static const char *const scribbles[] =
{
    "PENCIL: 'NEEDS MANAGER SIGN-OFF.'",
    "PENCIL: 'THE GRID IS UNDER REVIEW.'",
    "PENCIL: '14 ACROSS WAS CLEARER YESTERDAY.'",
    "PENCIL: 'ASK RECORDS FOR A PENCIL.'",
    "PENCIL: 'SOLVING THIS IS NOT IN MY REMIT.'",
    "PENCIL: 'DO NOT FILE UNTIL FINISHED.'"
};

#define F144_CROSSWORD_COUNT(a) ((uint32_t)(sizeof(a) / sizeof((a)[0])))

bool Floppy144CrosswordGenerate(
    uint32_t recovery_seed,
    Floppy144CrosswordView *view
)
{
    uint32_t row;
    uint32_t col;
    uint32_t across_mask;
    uint32_t down_mask;
    const Floppy144CrosswordPattern *pattern;

    if(view == NULL)
    {
        return false;
    }

    memset(view, 0, sizeof(*view));

    view->variant = (uint8_t)Floppy144VariationRange(
        recovery_seed, "staff.crossword.pattern.v1", "P-074",
        F144_CROSSWORD_COUNT(patterns)
    );
    pattern = &patterns[view->variant];

    /*
     * Select two independently keyed, nonempty pencil masks.
     * Centre is always written; at least two other squares in each word
     * are blank, keeping the crossword visibly half-finished.
     */
    {
        static const uint8_t masks[] = { 0x05U, 0x14U, 0x0EU, 0x1CU };
        across_mask = masks[Floppy144VariationRange(
            recovery_seed, "staff.crossword.across-fill.v1", "P-074",
            F144_CROSSWORD_COUNT(masks)
        )];
        down_mask = masks[Floppy144VariationRange(
            recovery_seed, "staff.crossword.down-fill.v1", "P-074",
            F144_CROSSWORD_COUNT(masks)
        )];
    }

    for(row = 0U; row < FLOPPY144_CROSSWORD_GRID_SIZE; ++row)
    {
        for(col = 0U; col < FLOPPY144_CROSSWORD_GRID_SIZE; ++col)
        {
            char glyph = '.';

            if(row == 2U)
            {
                glyph = (across_mask & (1U << col))
                    ? pattern->across[col] : '_';
            }
            if(col == 2U)
            {
                glyph = (down_mask & (1U << row))
                    ? pattern->down[row] : '_';
            }

            view->grid[row][col] = glyph;
        }
        view->grid[row][FLOPPY144_CROSSWORD_GRID_SIZE] = '\0';
    }

    /* Contract of the authored crossing: both words share the same R/A/etc. */
    if(
        strlen(pattern->across) != FLOPPY144_CROSSWORD_GRID_SIZE ||
        strlen(pattern->down) != FLOPPY144_CROSSWORD_GRID_SIZE ||
        pattern->across[2] != pattern->down[2]
    )
    {
        memset(view, 0, sizeof(*view));
        return false;
    }

    view->grid[2][2] = pattern->across[2];
    view->across_answer = pattern->across;
    view->down_answer = pattern->down;
    view->across_clue = pattern->across_clue;
    view->down_clue = pattern->down_clue;
    view->annotation = scribbles[Floppy144VariationRange(
        recovery_seed, "staff.crossword.scribble.v1", "P-074",
        F144_CROSSWORD_COUNT(scribbles)
    )];
    return true;
}
