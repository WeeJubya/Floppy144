#include "floppy144_takeaway.h"
#include "floppy144_variation.h"

#include <stdio.h>

/*
 * A staff-room relic from an unusually heavily regulated food economy.
 * Combinatorial nouns produce variety without storing 95,040 whole menus.
 * All phrases are bounded for the 54-character/3-line inspection detail.
 */
static const char *const modifiers[] =
{
    "THE AUTHORISED", "THE COMPULSORY", "THE PROVISIONAL",
    "THE UNFILED", "THE MISPLACED", "THE LATE",
    "THE UNVERIFIED", "THE COUNTERSIGNED", "THE PENDING",
    "THE OFFICIAL", "THE UNCLAIMED", "THE INTERIM"
};

static const char *const subjects[] =
{
    "BADGER", "KETTLE", "WOMBAT", "STAPLER",
    "PIGEON", "HEDGEHOG", "MOTH", "FERRET",
    "LADLE", "SQUIRREL", "GOOSE"
};

static const char *const businesses[] =
{
    "CURRY HOUSE", "NOODLE BAR", "CHIP SHOP",
    "PIE OFFICE", "DUMPLING DEPOT", "KEBAB COUNTER"
};

static const char *const specials[] =
{
    "QUEUE-JUMPING ONION BHAJIS",
    "REDACTED SWEET-AND-SOUR",
    "FORM 27 FRIED RICE",
    "THE ALLEGED LAMB CURRY",
    "UNCLAIMED CHIPS WITH GRAVY",
    "PROVISIONAL PORK DUMPLINGS",
    "SMALL-PRINT SPRING ROLLS",
    "STAPLED-TOGETHER NOODLES",
    "MINUTES-APPROVED MUSHROOM PIE",
    "AUTHORISED TOFU OF UNCERTAIN ORIGIN",
    "CLASSIFIED BEEF IN A BOX",
    "THE URGENT ONION PAKORA"
};

static const char *const taglines[] =
{
    "DELIVERIES REQUIRE TWO WITNESSES",
    "ALL COMPLAINTS IN TRIPLICATE",
    "PORTIONS SUBJECT TO REVIEW",
    "NO REFUNDS AFTER FINAL SIGN-OFF",
    "INSPECT BEFORE CONSUMPTION",
    "ALL PICKLES ARE OFFICIAL",
    "SAUCES PENDING COUNTERSIGNATURE",
    "QUEUE NUMBERS DO NOT GUARANTEE FOOD",
    "CLOSING TIME IS UNDER CONSULTATION",
    "RETAIN YOUR RECEIPT INDEFINITELY"
};

#define F144_TAKEAWAY_COUNT(array) ((uint32_t)(sizeof(array) / sizeof((array)[0])))

bool Floppy144TakeawayMenuGenerate(
    uint32_t recovery_seed,
    char *output,
    uint32_t capacity
)
{
    const char *modifier;
    const char *subject;
    const char *business;
    const char *special;
    const char *tagline;
    int written;

    if(output == NULL || capacity == 0U)
    {
        return false;
    }

    output[0] = '\0';

    modifier = modifiers[Floppy144VariationRange(
        recovery_seed, "takeaway.name.modifier.v1", "P-330",
        F144_TAKEAWAY_COUNT(modifiers)
    )];
    subject = subjects[Floppy144VariationRange(
        recovery_seed, "takeaway.name.subject.v1", "P-330",
        F144_TAKEAWAY_COUNT(subjects)
    )];
    business = businesses[Floppy144VariationRange(
        recovery_seed, "takeaway.name.business.v1", "P-330",
        F144_TAKEAWAY_COUNT(businesses)
    )];
    special = specials[Floppy144VariationRange(
        recovery_seed, "takeaway.special.v1", "P-330",
        F144_TAKEAWAY_COUNT(specials)
    )];
    tagline = taglines[Floppy144VariationRange(
        recovery_seed, "takeaway.tagline.v1", "P-330",
        F144_TAKEAWAY_COUNT(taglines)
    )];

    written = snprintf(
        output, (size_t)capacity,
        "%s %s %s\nSPECIAL: %s\n%s",
        modifier, subject, business, special, tagline
    );

    if(written < 0 || (uint32_t)written >= capacity)
    {
        /* Never hand a clipped, partial menu to the inspection renderer. */
        output[0] = '\0';
        return false;
    }

    return true;
}
