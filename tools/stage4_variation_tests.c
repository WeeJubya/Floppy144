/*
 * Stage 4E deterministic variation regression. No time/OS/random source.
 */
#include "floppy144_variation.h"
#include "f144_startup_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static int failures;
static void Expect(bool condition, const char *label)
{
    if(!condition)
    {
        ++failures;
        printf("FAIL: %s\n", label);
    }
}

static void TestGoldenVectors(void)
{
    Expect(
        Floppy144VariationValue(144U, "takeaway.menu.v1", "staff-room") ==
        3081933934U, "seed 144 takeaway vector"
    );
    Expect(
        Floppy144VariationValue(144U, "dr04.records.v1", "record-001") ==
        3702914435U, "seed 144 records vector"
    );
    Expect(
        Floppy144VariationValue(0U, "takeaway.menu.v1", "staff-room") ==
        347724737U, "legacy zero seed is deterministic"
    );
    Expect(
        Floppy144VariationValue(0xffffffffU, "takeaway.menu.v1", "staff-room") ==
        2287403285U, "maximum 32-bit seed is portable"
    );
}

static void TestNamespacesAndRepeatability(void)
{
    uint32_t takeaway = Floppy144VariationValue(
        144U, "takeaway.menu.v1", "staff-room"
    );
    uint32_t records = Floppy144VariationValue(
        144U, "dr04.records.v1", "record-001"
    );
    uint32_t index;

    for(index = 0U; index < 1024U; ++index)
    {
        (void)Floppy144VariationValue(index, "unrelated.feature.v1", "extra");
        (void)Floppy144VariationRange(index, "another.feature.v1", NULL, 12U);
    }

    Expect(
        takeaway == Floppy144VariationValue(
            144U, "takeaway.menu.v1", "staff-room"
        ), "repeat calls never advance another stream"
    );
    Expect(
        records == Floppy144VariationValue(
            144U, "dr04.records.v1", "record-001"
        ), "DR-04 unaffected by unrelated calls"
    );
    Expect(takeaway != records, "independent feature namespaces");
    Expect(
        Floppy144VariationValue(144U, "takeaway.menu.v1", "item-a") !=
        Floppy144VariationValue(144U, "takeaway.menu.v1", "item-b"),
        "independent stable item IDs"
    );
    Expect(
        Floppy144VariationValue(144U, "a", "bc") !=
        Floppy144VariationValue(144U, "ab", "c"),
        "namespace boundary is not ambiguous"
    );
    Expect(
        Floppy144VariationValue(144U, "test.v1", NULL) ==
        Floppy144VariationValue(144U, "test.v1", ""),
        "missing optional item ID is the empty key"
    );
}

static void TestSeedsRangesAndChance(void)
{
    uint32_t seed;
    Expect(
        Floppy144VariationValue(144U, "takeaway.menu.v1", "staff-room") !=
        Floppy144VariationValue(145U, "takeaway.menu.v1", "staff-room"),
        "different run seeds can change output"
    );
    for(seed = 1U; seed <= 256U; ++seed)
    {
        uint32_t value = Floppy144VariationRange(
            seed, "takeaway.menu.v1", "staff-room", 7U
        );
        Expect(value < 7U, "range is bounded");
        Expect(
            value == Floppy144VariationRange(
                seed, "takeaway.menu.v1", "staff-room", 7U
            ), "range repeats"
        );
        Expect(
            Floppy144VariationChance(
                seed, "notice.v1", "reception", 1U, 3U
            ) ==
            Floppy144VariationChance(
                seed, "notice.v1", "reception", 1U, 3U
            ), "chance repeats"
        );
    }
    Expect(
        Floppy144VariationRange(144U, "test", "", 0U) == 0U,
        "zero range is safe"
    );
    Expect(
        Floppy144VariationRange(144U, NULL, "", 7U) == 0U,
        "null feature rejected"
    );
    Expect(
        Floppy144VariationValue(144U, "", "item") == 0U,
        "empty feature rejected"
    );
    Expect(
        !Floppy144VariationChance(144U, "test", "", 1U, 0U),
        "zero denominator rejected"
    );
    Expect(
        !Floppy144VariationChance(144U, "test", "", 0U, 10U),
        "zero numerator false"
    );
    Expect(
        Floppy144VariationChance(144U, "test", "", 10U, 10U),
        "full chance true"
    );
    Expect(
        !Floppy144VariationChance(144U, NULL, "", 1U, 1U),
        "invalid namespace never triggers"
    );
}

static void TestStage4BSeedOverride(void)
{
    F144StartupConfig config;
    uint32_t seed = 0U;
    f144StartupConfigReset(&config);
    Expect(
        !f144StartupConfigRecoverySeedOverride(&config, &seed),
        "ordinary start has no forced seed"
    );
    f144StartupConfigSetDebugEnabled(&config, true);
    Expect(
        f144StartupConfigSetRecoverySeedOverride(&config, 424242U),
        "portable test seed can be injected"
    );
    Expect(
        f144StartupConfigRecoverySeedOverride(&config, &seed) &&
        seed == 424242U, "test seed override round-trips"
    );
    Expect(
        Floppy144VariationValue(seed, "takeaway.menu.v1", "staff-room") ==
        3639091269U, "forced seed gives golden value"
    );
}

int main(void)
{
    TestGoldenVectors();
    TestNamespacesAndRepeatability();
    TestSeedsRangesAndChance();
    TestStage4BSeedOverride();

    if(failures != 0)
    {
        printf("STAGE 4E VARIATION TESTS: FAIL (%d)\n", failures);
        return 1;
    }
    puts("STAGE 4E VARIATION TESTS: PASS");
    return 0;
}
