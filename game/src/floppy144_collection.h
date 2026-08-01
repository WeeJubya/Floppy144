#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Narrative stage in which a collection becomes available.
 *
 * The technical slice currently uses the Prologue and Act One. The remaining
 * values are included now so future collections can be registered without
 * changing this type.
 */

typedef enum Floppy144Act
{
    FLOPPY144_ACT_PROLOGUE,
    FLOPPY144_ACT_ONE,
    FLOPPY144_ACT_TWO,
    FLOPPY144_ACT_THREE,

    FLOPPY144_ACT_COUNT
} Floppy144Act;

/*
 * Departmental classification shown in the archive terminal.
 */

typedef enum Floppy144CollectionClass
{
    FLOPPY144_COLLECTION_CLASS_MANDATORY,
    FLOPPY144_COLLECTION_CLASS_OPERATIONAL,
    FLOPPY144_COLLECTION_CLASS_ADMINISTRATIVE,
    FLOPPY144_COLLECTION_CLASS_REPORTING
} Floppy144CollectionClass;

/*
 * Generate one enum value for every entry in floppy144_collections.def.
 *
 * The definition order is also the terminal display order. XX-01, HR-02 and
 * FA-03 therefore retain their existing numeric IDs of zero, one and two.
 */

typedef enum Floppy144CollectionId
{
#define FLOPPY144_COLLECTION(                                      \
    symbol,                                                        \
    code,                                                          \
    title,                                                         \
    act,                                                           \
    collection_class,                                              \
    auto_restored,                                                 \
    description,                                                   \
    evidence_description,                                          \
    catalogue_record_count,                                        \
    catalogue_heading,                                             \
    catalogue_record_id_prefix,                                    \
    catalogue_subjects,                                            \
    catalogue_subject_count,                                       \
    catalogue_record_number_base,                                  \
    catalogue_record_number_multiplier,                            \
    catalogue_record_number_offset                                 \
)                                                                  \
    FLOPPY144_COLLECTION_##symbol,

#include "floppy144_collections.def"

#undef FLOPPY144_COLLECTION

    FLOPPY144_COLLECTION_COUNT
} Floppy144CollectionId;
