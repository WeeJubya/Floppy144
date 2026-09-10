#include "floppy144_collection_registry.h"

#include <stddef.h>

/*
 * Collection-specific procedural title vocabularies
 *
 * These tables are immutable catalogue design data. Their owning collection
 * declarations reference them from floppy144_collections.def.
 */

static const char *const floppy144_dr01_record_titles[] =
{
    "DISK RECOVERY INDEX",
    "DISK 144 MEDIA MANIFEST",
    "ARCHIVE SERVICE STARTUP NOTE",
    "COLLECTION CODE DIRECTORY",
    "RECORD IDENTIFIER GUIDE",
    "SITE RECONSTRUCTION STATUS SUMMARY",
    "RECOVERED DATA LIMITATIONS",
    "ARCHIVE ACCESS NOTICE",
    "DATA INTEGRITY CHECK",
    "RECOVERY SESSION REGISTER",
    "SOURCE MEDIA HANDLING NOTE",
    "END OF RECOVERY INDEX"
};

static const char *const floppy144_hr01_record_subjects[] =
{
    "APPOINTMENT",
    "TRANSFER",
    "ABSENCE",
    "TRAINING",
    "ACCESS",
    "PAYROLL",
    "LEAVE",
    "DESK ALLOCATION",
    "APPRAISAL",
    "EXIT"
};

/*
 * FM-04 procedural catalogue subjects.
 *
 * Final authored record identities will replace these during archive
 * population. The first two readable documents are already authored
 * separately in floppy144_document.c.
 */
static const char *const floppy144_fm04_record_subjects[] =
{
    "SITE ACCESS",
    "MAINTENANCE",
    "FACILITIES",
    "CIRCULATION",
    "DOOR CONTROL",
    "UTILITIES",
    "ROOM IDENTIFICATION",
    "INSPECTION",
    "WORK ORDER",
    "HANDOVER"
};

static const char *const floppy144_fm07_record_subjects[] =
{
    "ASSET DISPOSAL",
    "RETAINED SERVICE",
    "UTILITY SHUTDOWN",
    "FURNITURE TRANSFER",
    "CONTRACTOR COORDINATION",
    "FINAL ISOLATION",
    "DECOMMISSIONING",
    "ASSET TRANSFER",
    "SITE HANDOVER",
    "CLOSURE WORK"
};

static const char *const floppy144_fm13_record_subjects[] =
{
    "SUPPRESSION PANEL",
    "HALON CYLINDER",
    "ALARM CIRCUIT",
    "SERVER ROOM SAFETY",
    "EMERGENCY CONTROL",
    "VENTILATION SYSTEM",
    "FIRE DOOR",
    "DETECTOR LOOP",
    "MAINTENANCE ACCESS",
    "PRESSURE SENSOR"
};

/*
 * Collection availability progression
 *
 * The full recovery uses persistent trigger state to determine when a
 * collection becomes available for restoration.
 *
 * The current technical-slice collections retain the compatibility sentinel
 * until their authoritative trigger routes are installed during Stage 2G.
 */

typedef struct Floppy144CollectionAvailabilityRoute
{
    Floppy144CollectionId collection;
    Floppy144TriggerId trigger;
}
Floppy144CollectionAvailabilityRoute;

static const Floppy144CollectionAvailabilityRoute
floppy144_collection_availability_routes[] =
{
    {
        FLOPPY144_COLLECTION_DR02,
        FLOPPY144_TRIGGER_T001
    },

    {
        FLOPPY144_COLLECTION_DR03,
        FLOPPY144_TRIGGER_T001
    },

    {
        FLOPPY144_COLLECTION_HR01,
        FLOPPY144_TRIGGER_T001
    },

    {
        FLOPPY144_COLLECTION_FM04,
        FLOPPY144_TRIGGER_T003
    },

    {
        FLOPPY144_COLLECTION_FM07,
        FLOPPY144_TRIGGER_T005
    },

    {
        FLOPPY144_COLLECTION_FM13,
        FLOPPY144_TRIGGER_T005
    }
};

/*
 * Generate the metadata table from the same master list used to generate the
 * collection enum.
 *
 * Because both are generated in definition-file order, an ID can be used
 * directly as an array index.
 */

const Floppy144CollectionDefinition
    floppy144_collection_definitions[FLOPPY144_COLLECTION_COUNT] =
{
#define FLOPPY144_COLLECTION(                                      \
    symbol,                                                        \
    code_text,                                                     \
    title_text,                                                    \
    domain_value,                                                  \
    reconstruction_percent_value,                                  \
    description_text,                                              \
    evidence_description_text,                                     \
    catalogue_record_count,                                        \
    catalogue_heading_text,                                        \
    catalogue_record_id_prefix,                                    \
    catalogue_subjects,                                            \
    catalogue_subject_count,                                       \
    catalogue_exact_titles,                                        \
    catalogue_record_number_base,                                  \
    catalogue_record_number_multiplier,                            \
    catalogue_record_number_offset                                 \
)                                                                  \
    {                                                              \
        FLOPPY144_COLLECTION_##symbol,                             \
        code_text,                                                 \
        title_text,                                                \
        domain_value,                                              \
        reconstruction_percent_value,                              \
        description_text,                                          \
        evidence_description_text,                                 \
        {                                                          \
            catalogue_record_count,                                \
            catalogue_heading_text,                                \
            catalogue_record_id_prefix,                            \
            catalogue_subjects,                                    \
            catalogue_subject_count,                               \
            catalogue_exact_titles,                                \
            catalogue_record_number_base,                          \
            catalogue_record_number_multiplier,                    \
            catalogue_record_number_offset                         \
        }                                                          \
    },

#include "floppy144_collections.def"

#undef FLOPPY144_COLLECTION
};

const Floppy144CollectionDefinition *Floppy144CollectionGet(
    Floppy144CollectionId collection
)
{
    uint32_t index =
        (uint32_t)collection;

    if(index >= (uint32_t)FLOPPY144_COLLECTION_COUNT)
    {
        index =
            (uint32_t)FLOPPY144_COLLECTION_DR01;
    }

    return
        &floppy144_collection_definitions[index];
}

Floppy144TriggerId Floppy144CollectionAvailabilityTrigger
(
    Floppy144CollectionId collection
)
{
    uint32_t route_index;

    if(
        (uint32_t)collection >=
        (uint32_t)FLOPPY144_COLLECTION_COUNT
    )
    {
        return FLOPPY144_TRIGGER_COUNT;
    }

    for(
        route_index = 0U;
    route_index <
    (uint32_t)(
        sizeof(floppy144_collection_availability_routes) /
        sizeof(floppy144_collection_availability_routes[0])
    );
    ++route_index
    )
    {
        const Floppy144CollectionAvailabilityRoute *route =
        &floppy144_collection_availability_routes[
            route_index
        ];

        if(route->collection == collection)
        {
            return route->trigger;
        }
    }

    return FLOPPY144_TRIGGER_COUNT;
}

const char *Floppy144CollectionDomainText(
    Floppy144CollectionDomain domain
)
{
    switch(domain)
    {
        case FLOPPY144_COLLECTION_DOMAIN_DR:
        {
            return "DR";
        }

        case FLOPPY144_COLLECTION_DOMAIN_HR:
        {
            return "HR";
        }

        case FLOPPY144_COLLECTION_DOMAIN_FM:
        {
            return "FM";
        }

        case FLOPPY144_COLLECTION_DOMAIN_OS:
        {
            return "OS";
        }

        case FLOPPY144_COLLECTION_DOMAIN_TS:
        {
            return "TS";
        }

        default:
        {
            break;
        }
    }

    return "UNKNOWN";
}
