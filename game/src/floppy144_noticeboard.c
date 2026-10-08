#include "floppy144_noticeboard.h"
#include "floppy144_variation.h"

#include <stddef.h>
#include <string.h>

typedef struct Floppy144NoticeboardAlternative
{
    const char *id;
    const char *annotation;
} Floppy144NoticeboardAlternative;

/*
 * Alternative marginalia add texture to the existing authored ambient flyers.
 * The headline, window and permanent P-138 remain entirely canonical.
 */
static const Floppy144NoticeboardAlternative alternatives[] =
{
    { "AMB-NB-01", "The previous year's corrections form is still awaiting approval." },
    { "AMB-NB-02", "One unsigned card has been returned to its sender for verification." },
    { "AMB-NB-03", "Facilities requests a complete inventory of missing chocolate." },
    { "AMB-NB-04", "The sunshine contingency form has not yet been countersigned." },
    { "AMB-NB-05", "A ghost has been asked to sign the visitor register." },
    { "AMB-NB-06", "The phrase 'controlled explosion' has been crossed out twice." },
    { "AMB-NB-07", "Dietary preferences will be retained until the next closure review." },
    { "AMB-NB-08", "An unsigned reply slip has been referred back to its originator." }
};

bool Floppy144NoticeboardSelect(
    F144CalendarDate date,
    uint32_t recovery_seed,
    const Floppy144DataRecord **ambient,
    const char **annotation
)
{
    const Floppy144DataRecord *entry;
    uint32_t index;

    if(ambient == NULL || annotation == NULL)
    {
        return false;
    }

    *ambient = NULL;
    *annotation = NULL;

    if(!f144CalendarDateValid(date.year, date.month, date.day))
    {
        return false;
    }

    entry = Floppy144GameDataAmbientForDate(
        "P-138", (uint32_t)date.month, (uint32_t)date.day
    );

    if(
        entry == NULL ||
        entry->pszId == NULL ||
        entry->pszA == NULL ||
        strcmp(entry->pszA, "NOTICEBOARD_SEASONAL") != 0 ||
        entry->pszE == NULL ||
        entry->pszF == NULL
    )
    {
        return false;
    }

    *ambient = entry;
    *annotation = entry->pszF;

    for(index = 0U; index < (uint32_t)(sizeof(alternatives)/sizeof(alternatives[0])); ++index)
    {
        if(strcmp(entry->pszId, alternatives[index].id) == 0)
        {
            if(
                Floppy144VariationRange(
                    recovery_seed, "staff.noticeboard.annotation.v1",
                    entry->pszId, 2U
                ) == 1U
            )
            {
                *annotation = alternatives[index].annotation;
            }
            break;
        }
    }

    return true;
}
