/*
 * FLOPPY//144 S4E-04 standalone generator regressions.
 * Pure C, with no game data, OS clock, filesystem, global random state.
 */
#include "floppy144_takeaway.h"
#include "floppy144_variation.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t failures;

static void Expect(bool condition, const char *message)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n", message);
    }
}

static bool FitsDetailLayout(const char *text)
{
    uint32_t lines = 1U;
    uint32_t line_length = 0U;
    uint32_t position;

    if(text == NULL || text[0] == '\0')
    {
        return false;
    }

    for(position = 0U; text[position] != '\0'; ++position)
    {
        unsigned char c = (unsigned char)text[position];

        if(c == (unsigned char)'\n')
        {
            if(line_length == 0U)
            {
                return false;
            }
            ++lines;
            line_length = 0U;
        }
        else
        {
            if(c < 32U || c > 126U)
            {
                return false;
            }
            ++line_length;
            if(line_length > FLOPPY144_TAKEAWAY_MAX_LINE_LENGTH)
            {
                return false;
            }
        }
    }

    return lines == 3U && line_length > 0U;
}

static bool SameBusiness(const char *a, const char *b)
{
    const char *a_end = strchr(a, '\n');
    const char *b_end = strchr(b, '\n');
    size_t a_length;
    size_t b_length;

    if(a_end == NULL || b_end == NULL)
    {
        return false;
    }
    a_length = (size_t)(a_end - a);
    b_length = (size_t)(b_end - b);
    return a_length == b_length && memcmp(a, b, a_length) == 0;
}

static void TestGoldenSeeds(void)
{
    char a[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
    char b[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
    char again[FLOPPY144_TAKEAWAY_MENU_CAPACITY];

    Expect(
        Floppy144TakeawayMenuGenerate(144U, a, (uint32_t)sizeof(a)),
        "seed 144 generates a complete menu"
    );
    Expect(
        strcmp(a,
            "THE PENDING HEDGEHOG PIE OFFICE\n"
            "SPECIAL: CLASSIFIED BEEF IN A BOX\n"
            "CLOSING TIME IS UNDER CONSULTATION") == 0,
        "seed 144 golden name, dish and tagline"
    );
    Expect(
        Floppy144TakeawayMenuGenerate(145U, b, (uint32_t)sizeof(b)),
        "seed 145 generates a complete menu"
    );
    Expect(
        strcmp(b,
            "THE OFFICIAL SQUIRREL DUMPLING DEPOT\n"
            "SPECIAL: STAPLED-TOGETHER NOODLES\n"
            "NO REFUNDS AFTER FINAL SIGN-OFF") == 0,
        "seed 145 golden name, dish and tagline"
    );
    Expect(!SameBusiness(a, b), "known seeds A and B have different businesses");

    (void)Floppy144VariationValue(
        144U, "staff.noticeboard.annotation.v1", "AMB-NB-07"
    );
    (void)Floppy144VariationRange(
        144U, "unrelated.future.feature.v1", "ANOTHER_ITEM", 1000U
    );

    Expect(
        Floppy144TakeawayMenuGenerate(144U, again, (uint32_t)sizeof(again)) &&
        strcmp(a, again) == 0,
        "unrelated random calls cannot reshuffle a fixed run seed"
    );

    printf("SEED 144 MENU:\n%s\n", a);
    printf("SEED 145 MENU:\n%s\n", b);
}

static void TestAllLayoutVariants(void)
{
    uint32_t seed;
    uint32_t changed_business_transitions = 0U;
    char previous[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
    char current[FLOPPY144_TAKEAWAY_MENU_CAPACITY];
    char again[FLOPPY144_TAKEAWAY_MENU_CAPACITY];

    previous[0] = '\0';

    /* Broad deterministic sweep catches any empty names or wrapping drift. */
    for(seed = 1U; seed <= 10000U; ++seed)
    {
        if(
            !Floppy144TakeawayMenuGenerate(
                seed, current, (uint32_t)sizeof(current)
            ) ||
            !FitsDetailLayout(current) ||
            strlen(current) >= sizeof(current)
        )
        {
            Expect(false, "10,000-seed sweep generates only complete 3-line menus");
            break;
        }

        if(seed == 1U || !SameBusiness(previous, current))
        {
            ++changed_business_transitions;
        }
        (void)snprintf(previous, sizeof(previous), "%s", current);

        if(seed % 97U == 0U)
        {
            Expect(
                Floppy144TakeawayMenuGenerate(
                    seed, again, (uint32_t)sizeof(again)
                ) &&
                strcmp(current, again) == 0,
                "repeated generation produces byte-identical output"
            );
        }
    }

    Expect(
        changed_business_transitions > 1000U,
        "run-seed sweep repeatedly selects different neighbouring business names"
    );
}

static void TestSafeCapacityHandling(void)
{
    char empty[1];
    unsigned char guarded[12];
    char enough[FLOPPY144_TAKEAWAY_MENU_CAPACITY];

    memset(guarded, 0x5AU, sizeof(guarded));
    Expect(
        !Floppy144TakeawayMenuGenerate(144U, NULL, 0U),
        "null output pointer fails safely"
    );
    Expect(
        !Floppy144TakeawayMenuGenerate(144U, empty, 0U),
        "zero-size output fails without writing"
    );
    Expect(
        !Floppy144TakeawayMenuGenerate(144U, empty, 1U) &&
        empty[0] == '\0',
        "one-byte output cannot expose truncated text"
    );
    Expect(
        !Floppy144TakeawayMenuGenerate(144U, (char *)&guarded[1], 9U) &&
        guarded[0] == 0x5AU && guarded[10] == 0x5AU &&
        guarded[11] == 0x5AU &&
        guarded[1] == (unsigned char)'\0',
        "short buffer fails and preserves boundary canaries"
    );
    Expect(
        Floppy144TakeawayMenuGenerate(
            144U, enough, (uint32_t)sizeof(enough)
        ) &&
        FitsDetailLayout(enough),
        "full-size output remains a valid three-line flyer"
    );
}

int main(void)
{
    TestGoldenSeeds();
    TestAllLayoutVariants();
    TestSafeCapacityHandling();

    if(failures != 0U)
    {
        printf("STAGE 4E TAKEAWAY TESTS: FAIL (%u)\n", failures);
        return 1;
    }
    puts("STAGE 4E TAKEAWAY TESTS: PASS");
    return 0;
}
