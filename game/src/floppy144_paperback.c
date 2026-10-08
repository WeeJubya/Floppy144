/*
 * Compact fictional paperback cover generator.
 * No real authors, copyrighted titles, time dependence or game-state effects.
 */
#include "floppy144_paperback.h"
#include "floppy144_variation.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static const char *const possessions[] =
{
    "STAPLER", "LEDGER", "AUDIT", "INBOX",
    "BINDER", "SANDWICH", "FILING CABINET", "TEA TROLLEY"
};
static const char *const locations[] =
{
    "LOST TUESDAYS", "ROOM ZERO", "THE NORTH ANNEX",
    "UNCLAIMED DESKS", "THE LIFT SHAFT", "MISSING MINUTES",
    "EXPIRED FORMS", "THE LOCKED CUPBOARD"
};
static const char *const qualities[] =
{
    "RELUCTANT", "CONFIDENTIAL", "PROVISIONAL",
    "UNCLAIMED", "HEARTBROKEN", "UNVERIFIED", "SUSPENDED", "SENIOR"
};
static const char *const professions[] =
{
    "ARCHIVE CLERK", "RECEPTIONIST", "COMPLIANCE OFFICER",
    "PHOTOCOPIER", "RECORDS INSPECTOR", "FILING ASSISTANT",
    "WITNESS", "LINE MANAGER"
};
static const char *const romantic_objects[] =
{
    "STAPLER", "BADGE", "MEMO", "BINDER",
    "RECEIPT", "FOLDER", "DOSSIER", "PAPERCLIP"
};
static const char *const romantic_qualities[] =
{
    "PASSIONATE", "CONFIDENTIAL", "SENTIMENTAL", "OFFICIAL",
    "DANGEROUS", "LATE", "COMPLICATED", "NEEDY"
};
static const char *const ordinals[] =
{
    "THIRD", "SEVENTEENTH", "FINAL", "UNNUMBERED",
    "SECOND", "FORGOTTEN", "PENULTIMATE"
};
static const char *const departments[] =
{
    "REGISTRY", "COMMITTEE", "DIRECTORATE", "DEPARTMENT",
    "RECORDS OFFICE", "COMPLAINTS UNIT", "AUTHORISATION DESK"
};
static const char *const verbs[] =
{
    "ARCHIVE", "STAMP", "FILE", "AUDIT",
    "COUNTERSIGN", "INVOICE", "ENDORSE", "SHRED"
};
static const char *const deadlines[] =
{
    "THE TEA BREAK", "LAST THURSDAY", "THE BOARD MEETS",
    "FINAL APPROVAL", "THE FIRE DRILL", "THE REVIEW PANEL",
    "LUNCH", "THE CLOSURE MEMO"
};
static const char *const given_names[] =
{
    "BERYL", "CRISPIN", "MILLICENT", "EDGAR",
    "NORA", "ALGERNON", "TRUDY", "PEGGY"
};
static const char *const family_names[] =
{
    "DOCKET", "GRINDLE", "WAINSCOT", "QUIBBLE",
    "MUDDLE", "FLEEB", "PINCE", "BUMBLEWICK"
};
static const char *const annotations[] =
{
    "SPINE HELD TOGETHER WITH A PAYSLIP",
    "CHAPTER 7 HAS BEEN STAMPED RECEIVED",
    "A TEA RING HAS EATEN THE DEDICATION",
    "THE ENDING IS PENDING LEGAL REVIEW",
    "PAGE 47 HAS BEEN FILED SEPARATELY",
    "THE HERO HAS BEEN PUT ON NOTICE",
    "THE LAST PAGE IS A LEAVE REQUEST",
    "SOMEONE UNDERLINED 'SEE ATTACHED'"
};

#define F144_PB_COUNT(a) ((uint32_t)(sizeof(a) / sizeof((a)[0])))

static uint32_t Floppy144PaperbackPoolSize(uint32_t form, bool left)
{
    switch(form)
    {
        case 0U: return left ? F144_PB_COUNT(possessions) : F144_PB_COUNT(locations);
        case 1U: return left ? F144_PB_COUNT(qualities) : F144_PB_COUNT(professions);
        case 2U: return left ? F144_PB_COUNT(romantic_objects) : F144_PB_COUNT(romantic_qualities);
        case 3U: return left ? F144_PB_COUNT(ordinals) : F144_PB_COUNT(departments);
        case 4U: return left ? F144_PB_COUNT(verbs) : F144_PB_COUNT(deadlines);
        default: return 0U;
    }
}

static bool Floppy144PaperbackThreeLinesFit(const char *text)
{
    uint32_t length = 0U;
    uint32_t lines = 1U;
    const unsigned char *p = (const unsigned char *)text;

    if(text == NULL || text[0] == '\0')
        return false;

    while(*p != 0U)
    {
        if(*p == (unsigned char)'\n')
        {
            if(length == 0U)
                return false;
            length = 0U;
            ++lines;
        }
        else
        {
            /* Bitmap font intentionally supports only simple ASCII here. */
            if(*p < 32U || *p > 126U)
                return false;
            ++length;
            if(length > FLOPPY144_PAPERBACK_LINE_LIMIT)
                return false;
        }
        ++p;
    }

    return lines == 3U && length > 0U;
}

bool Floppy144PaperbackCompose(
    uint32_t form,
    uint32_t left,
    uint32_t right,
    uint32_t given_name,
    uint32_t family_name,
    uint32_t note,
    char *output,
    uint32_t capacity
)
{
    const char *first;
    const char *second;
    const char *format;
    char title[64];
    int written;

    if(output == NULL || capacity == 0U)
        return false;

    output[0] = '\0';

    if(
        form >= FLOPPY144_PAPERBACK_FORM_COUNT ||
        left >= Floppy144PaperbackPoolSize(form, true) ||
        right >= Floppy144PaperbackPoolSize(form, false) ||
        given_name >= F144_PB_COUNT(given_names) ||
        family_name >= F144_PB_COUNT(family_names) ||
        note >= F144_PB_COUNT(annotations)
    )
        return false;

    switch(form)
    {
        case 0U:
            first = possessions[left];
            second = locations[right];
            format = "THE %s OF %s";
            break;
        case 1U:
            first = qualities[left];
            second = professions[right];
            format = "%s %s";
            break;
        case 2U:
            first = romantic_objects[left];
            second = romantic_qualities[right];
            format = "A %s TOO %s";
            break;
        case 3U:
            first = ordinals[left];
            second = departments[right];
            format = "THE %s %s";
            break;
        case 4U:
            first = verbs[left];
            second = deadlines[right];
            format = "%s ME BEFORE %s";
            break;
        default:
            return false;
    }

    written = snprintf(title, sizeof(title), format, first, second);
    if(written <= 0 || (size_t)written >= sizeof(title))
        return false;

    written = snprintf(
        output, (size_t)capacity,
        "TITLE: %s\nBY: %s %s\n%s",
        title, given_names[given_name], family_names[family_name],
        annotations[note]
    );

    if(
        written <= 0 || (uint32_t)written >= capacity ||
        !Floppy144PaperbackThreeLinesFit(output)
    )
    {
        output[0] = '\0';
        return false;
    }
    return true;
}

bool Floppy144PaperbackGenerate(
    uint32_t recovery_seed,
    char *output,
    uint32_t capacity
)
{
    uint32_t form = Floppy144VariationRange(
        recovery_seed, "staff.paperback.form.v1", "P-073",
        FLOPPY144_PAPERBACK_FORM_COUNT
    );
    uint32_t left = Floppy144VariationRange(
        recovery_seed, "staff.paperback.fragment.first.v1", "P-073",
        Floppy144PaperbackPoolSize(form, true)
    );
    uint32_t right = Floppy144VariationRange(
        recovery_seed, "staff.paperback.fragment.second.v1", "P-073",
        Floppy144PaperbackPoolSize(form, false)
    );
    uint32_t given = Floppy144VariationRange(
        recovery_seed, "staff.paperback.author.given.v1", "P-073",
        F144_PB_COUNT(given_names)
    );
    uint32_t family = Floppy144VariationRange(
        recovery_seed, "staff.paperback.author.family.v1", "P-073",
        F144_PB_COUNT(family_names)
    );
    uint32_t note = Floppy144VariationRange(
        recovery_seed, "staff.paperback.marginalia.v1", "P-073",
        F144_PB_COUNT(annotations)
    );

    return Floppy144PaperbackCompose(
        form, left, right, given, family, note, output, capacity
    );
}
