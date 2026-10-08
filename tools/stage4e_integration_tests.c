/*
 * S4E-10 fixed-seed integration vectors.
 * The stand-alone portable executable combines every flavour generator
 * with independently keyed variation, rather than testing them separately.
 */
#include "floppy144_variation.h"
#include "floppy144_takeaway.h"
#include "floppy144_crossword.h"
#include "floppy144_paperback.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static unsigned failures = 0U;
static void Check(bool condition, const char *message)
{
    if(!condition)
    {
        fprintf(stderr, "S4E-10 FAIL: %s\n", message);
        ++failures;
    }
}
static bool StartsWith(const char *text, const char *prefix)
{
    return strncmp(text, prefix, strlen(prefix)) == 0;
}
int main(void)
{
    static const uint32_t seeds[2] = { 146U, 144U };
    static const uint8_t expectedCrossword[2] = { 3U, 4U };
    static const uint8_t expectedSwap[2] = { 0U, 1U };
    static const char *const expectedShop[2] =
    {
        "THE OFFICIAL FERRET CHIP SHOP\n",
        "THE PENDING HEDGEHOG PIE OFFICE\n"
    };
    static const char *const expectedBook[2] =
    {
        "TITLE: A PAPERCLIP TOO CONFIDENTIAL\nBY: CRISPIN BUMBLEWICK\n",
        "TITLE: SENIOR LINE MANAGER\nBY: CRISPIN QUIBBLE\n"
    };
    static const char *const expectedAcross[2] = { "QUEUE", "ORDER" };
    static const char *const expectedDown[2] = { "SHELF", "INDEX" };
    uint32_t i;

    for(i = 0U; i < 2U; ++i)
    {
        char shop[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
        char shopReloaded[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
        char book[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
        char bookReloaded[FLOPPY144_PAPERBACK_TEXT_CAPACITY];
        Floppy144CrosswordView crossword, crosswordReloaded;
        uint32_t seed = seeds[i];

        Check(
            Floppy144TakeawayMenuGenerate(seed,shop,(uint32_t)sizeof(shop)) &&
            StartsWith(shop,expectedShop[i]),
            "takeaway golden business identity"
        );
        Check(
            Floppy144CrosswordGenerate(seed,&crossword) &&
            crossword.variant == expectedCrossword[i] &&
            strcmp(crossword.across_answer, expectedAcross[i]) == 0 &&
            strcmp(crossword.down_answer, expectedDown[i]) == 0,
            "crossword golden variant and crossings"
        );
        Check(
            Floppy144PaperbackGenerate(seed,book,(uint32_t)sizeof(book)) &&
            StartsWith(book,expectedBook[i]),
            "paperback golden title and author"
        );
        Check(
            Floppy144VariationRange(seed,
                "dr04.workstream-swap.v1","DR-04",2U) == expectedSwap[i],
            "opposite deterministic DR-04 arrangement classes"
        );

        /* Scramble feature access order, without touching the run seed. */
        (void)Floppy144VariationValue(seed,"another.future.feature.v1","EXTRA");
        (void)Floppy144VariationRange(seed,"staff.crossword.scribble.v1",
            "P-074",6U);
        (void)Floppy144VariationRange(seed,"takeaway.name.subject.v1",
            "P-330",11U);
        (void)Floppy144VariationRange(seed,"dr04.workstream-swap.v1",
            "DR-04",2U);

        Check(
            Floppy144PaperbackGenerate(seed,bookReloaded,
                (uint32_t)sizeof(bookReloaded)) &&
            strcmp(book,bookReloaded) == 0,
            "recreated paperback is byte-identical after unrelated calls"
        );
        Check(
            Floppy144TakeawayMenuGenerate(seed,shopReloaded,
                (uint32_t)sizeof(shopReloaded)) &&
            strcmp(shop,shopReloaded) == 0,
            "recreated takeaway is byte-identical after unrelated calls"
        );
        Check(
            Floppy144CrosswordGenerate(seed,&crosswordReloaded) &&
            crosswordReloaded.variant == crossword.variant &&
            strcmp(crosswordReloaded.across_answer,
                crossword.across_answer) == 0 &&
            strcmp(crosswordReloaded.down_answer,
                crossword.down_answer) == 0 &&
            strcmp(crosswordReloaded.annotation,crossword.annotation) == 0 &&
            memcmp(crosswordReloaded.grid,crossword.grid,
                sizeof(crossword.grid)) == 0,
            "recreated crossword grid/clues are unchanged by call order"
        );
        printf(
            "S4E-10 SEED %u: takeaway=%s crossword=%s/%s paperback=%.*s swap=%u\n",
            (unsigned)seed, i == 0U ? "FERRET CHIP SHOP" : "HEDGEHOG PIE OFFICE",
            expectedAcross[i],expectedDown[i],
            (int)(strchr(book,'\n') - book), book,
            (unsigned)expectedSwap[i]
        );
    }

    if(failures != 0U)
    {
        fprintf(stderr,"STAGE 4E INTEGRATION VECTORS: FAIL (%u)\n",failures);
        return 1;
    }
    puts("STAGE 4E INTEGRATION VECTORS: PASS");
    return 0;
}
