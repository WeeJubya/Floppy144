/*
 * S4E-05 miniature crossword fixture, pure portable C.
 * Crossings, clue/answer mapping, bounded grid and deterministic variation.
 */
#include "floppy144_crossword.h"
#include "floppy144_variation.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct ExpectedCrossing
{
    const char *across;
    const char *down;
    const char *across_clue;
    const char *down_clue;
} ExpectedCrossing;

static const ExpectedCrossing expected[] =
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

static uint32_t failures;

static void Expect(bool success, const char *description)
{
    if(!success)
    {
        ++failures;
        printf("FAIL: %s\n", description);
    }
}

static bool ValidView(const Floppy144CrosswordView *v)
{
    uint32_t r;
    uint32_t c;
    uint32_t across_letters = 0U;
    uint32_t down_letters = 0U;
    const ExpectedCrossing *e;

    if(
        v == NULL ||
        v->variant >= FLOPPY144_CROSSWORD_VARIANT_COUNT ||
        v->across_answer == NULL || v->down_answer == NULL ||
        v->across_clue == NULL || v->down_clue == NULL ||
        v->annotation == NULL
    )
        return false;

    e = &expected[v->variant];

    if(
        strcmp(v->across_answer, e->across) != 0 ||
        strcmp(v->down_answer, e->down) != 0 ||
        strcmp(v->across_clue, e->across_clue) != 0 ||
        strcmp(v->down_clue, e->down_clue) != 0 ||
        strlen(v->across_answer) != 5U ||
        strlen(v->down_answer) != 5U ||
        v->across_answer[2] != v->down_answer[2] ||
        strlen(v->across_clue) > 54U ||
        strlen(v->down_clue) > 54U ||
        strlen(v->annotation) > 54U ||
        strncmp(v->annotation, "PENCIL:", 7U) != 0
    )
        return false;

    for(r = 0U; r < 5U; ++r)
    {
        if(strlen(v->grid[r]) != 5U)
            return false;

        for(c = 0U; c < 5U; ++c)
        {
            const char symbol = v->grid[r][c];

            if(r == 2U)
            {
                if(symbol != '_' && symbol != e->across[c])
                    return false;
                if(symbol != '_')
                    ++across_letters;
            }
            else if(c == 2U)
            {
                if(symbol != '_' && symbol != e->down[r])
                    return false;
                if(symbol != '_')
                    ++down_letters;
            }
            else if(symbol != '.')
                return false;
        }
    }

    /* Add the shared middle letter to Down count. */
    ++down_letters;

    return
        v->grid[2][2] == e->across[2] &&
        across_letters >= 2U && across_letters <= 3U &&
        down_letters >= 2U && down_letters <= 3U;
}

static void TestKnownSeeds(void)
{
    Floppy144CrosswordView first;
    Floppy144CrosswordView second;
    Floppy144CrosswordView repeated;
    uint32_t row;

    Expect(
        Floppy144CrosswordGenerate(144U, &first) &&
        first.variant == 4U &&
        strcmp(first.across_answer, "ORDER") == 0 &&
        strcmp(first.down_answer, "INDEX") == 0 &&
        ValidView(&first),
        "seed 144 selects valid ORDER x INDEX record"
    );
    Expect(
        Floppy144CrosswordGenerate(145U, &second) &&
        second.variant == 3U &&
        strcmp(second.across_answer, "QUEUE") == 0 &&
        strcmp(second.down_answer, "SHELF") == 0 &&
        ValidView(&second),
        "seed 145 selects valid QUEUE x SHELF record"
    );

    (void)Floppy144VariationRange(
        144U, "staff.noticeboard.annotation.v1", "AMB-NB-07", 2U
    );
    (void)Floppy144VariationRange(
        144U, "takeaway.name.business.v1", "P-330", 6U
    );

    Expect(
        Floppy144CrosswordGenerate(144U, &repeated) &&
        memcmp(&first, &repeated, sizeof(first)) == 0,
        "unrelated variation calls never reshuffle the crossword"
    );
    Expect(
        first.variant != second.variant,
        "different known seeds use distinct crossings"
    );

    puts("SEED 144: ORDER / INDEX");
    for(row = 0U; row < 5U; ++row)
        printf("  %s\n", first.grid[row]);
    printf("%s; %s\n%s\n",
        first.across_clue, first.down_clue, first.annotation);
    puts("SEED 145: QUEUE / SHELF");
    for(row = 0U; row < 5U; ++row)
        printf("  %s\n", second.grid[row]);
    printf("%s; %s\n%s\n",
        second.across_clue, second.down_clue, second.annotation);
}

static void TestWideSeedSweep(void)
{
    bool seen[6] = { false, false, false, false, false, false };
    uint32_t seed;
    uint32_t distinct = 0U;

    for(seed = 1U; seed <= 10000U; ++seed)
    {
        Floppy144CrosswordView view;

        if(!Floppy144CrosswordGenerate(seed, &view) || !ValidView(&view))
        {
            Expect(false, "10,000 seeds produce valid aligned incomplete grids");
            return;
        }

        seen[view.variant] = true;
    }

    for(seed = 0U; seed < 6U; ++seed)
    {
        if(seen[seed])
            ++distinct;
    }

    Expect(distinct == 6U, "all six authored crossings occur in the seed sweep");
    Expect(
        !Floppy144CrosswordGenerate(144U, NULL),
        "null output fails safely"
    );
}

int main(void)
{
    TestKnownSeeds();
    TestWideSeedSweep();
    if(failures != 0U)
    {
        printf("STAGE 4E CROSSWORD TESTS: FAIL (%u)\n", failures);
        return 1;
    }
    puts("STAGE 4E CROSSWORD TESTS: PASS");
    return 0;
}
