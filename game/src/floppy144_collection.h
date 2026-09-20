#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Archive collection domain.
 *
 * Domains describe which part of the recovered organisation owns a
 * collection. They are catalogue/navigation metadata only and do not control
 * recovery progression.
 */

typedef enum Floppy144CollectionDomain
{
    FLOPPY144_COLLECTION_DOMAIN_DR = 0,
    FLOPPY144_COLLECTION_DOMAIN_HR,
    FLOPPY144_COLLECTION_DOMAIN_FM,
    FLOPPY144_COLLECTION_DOMAIN_OS,
    FLOPPY144_COLLECTION_DOMAIN_TS,

    FLOPPY144_COLLECTION_DOMAIN_COUNT
}
Floppy144CollectionDomain;

typedef enum Floppy144CollectionId
{
#define FLOPPY144_COLLECTION(                                      \
    symbol,                                                        \
    code,                                                          \
    title,                                                         \
    domain,                                                        \
    size_kb,                                                       \
    required_for_completion,                                       \
    description,                                                   \
    evidence_description,                                          \
    catalogue_record_count,                                        \
    catalogue_heading,                                             \
    catalogue_record_id_prefix,                                    \
    catalogue_subjects,                                            \
    catalogue_subject_count,                                       \
    catalogue_exact_titles,                                        \
    catalogue_record_number_base,                                  \
    catalogue_record_number_multiplier,                            \
    catalogue_record_number_offset                                 \
)                                                                  \
    FLOPPY144_COLLECTION_##symbol,

#include "floppy144_collections.def"

#undef FLOPPY144_COLLECTION

    FLOPPY144_COLLECTION_COUNT
} Floppy144CollectionId;
