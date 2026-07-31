#include "floppy144_collection_registry.h"

#include <stddef.h>

/*
 * Collection-specific procedural title vocabularies
 *
 * These tables are immutable catalogue design data. Their owning collection
 * declarations reference them from floppy144_collections.def.
 */

static const char *const floppy144_hr02_record_subjects[] =
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

static const char *const floppy144_fa03_record_subjects[] =
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
    act_value,                                                     \
    class_value,                                                   \
    auto_restored_value,                                           \
    description_text,                                              \
    catalogue_record_count,                                        \
    catalogue_heading_text,                                        \
    catalogue_record_id_prefix,                                    \
    catalogue_subjects,                                            \
    catalogue_subject_count,                                       \
    catalogue_record_number_base,                                  \
    catalogue_record_number_multiplier,                            \
    catalogue_record_number_offset                                 \
)                                                                  \
    {                                                              \
        FLOPPY144_COLLECTION_##symbol,                             \
        code_text,                                                 \
        title_text,                                                \
        act_value,                                                 \
        class_value,                                               \
        auto_restored_value,                                       \
        description_text,                                          \
        {                                                          \
            catalogue_record_count,                                \
            catalogue_heading_text,                                \
            catalogue_record_id_prefix,                            \
            catalogue_subjects,                                    \
            catalogue_subject_count,                               \
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
            (uint32_t)FLOPPY144_COLLECTION_XX01;
    }

    return
        &floppy144_collection_definitions[index];
}

const char *Floppy144CollectionClassText(
    Floppy144CollectionClass collection_class
)
{
    switch(collection_class)
    {
        case FLOPPY144_COLLECTION_CLASS_MANDATORY:
        {
            return "MANDATORY";
        }

        case FLOPPY144_COLLECTION_CLASS_OPERATIONAL:
        {
            return "OPERATIONAL";
        }

        case FLOPPY144_COLLECTION_CLASS_ADMINISTRATIVE:
        {
            return "ADMINISTRATIVE";
        }

        case FLOPPY144_COLLECTION_CLASS_REPORTING:
        {
            return "REPORTING";
        }
    }

    return "UNKNOWN";
}

const char *Floppy144CollectionActText(
    Floppy144Act act
)
{
    switch(act)
    {
        case FLOPPY144_ACT_PROLOGUE:
        {
            return "PROLOGUE";
        }

        case FLOPPY144_ACT_ONE:
        {
            return "ACT I";
        }

        case FLOPPY144_ACT_TWO:
        {
            return "ACT II";
        }

        case FLOPPY144_ACT_THREE:
        {
            return "ACT III";
        }
    }

    return "UNKNOWN";
}
