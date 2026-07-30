/*
 * Floppy//144 - collection identifiers
 *
 * Defines the stable IDs used to tell the terminal, catalogue and world
 * state which archive collection is being handled.
 */

#pragma once

/*
 * Collection identity
 *
 * XX-01 is mandatory recovery data. HR-02 and FA-03 are optional
 * collections currently implemented by the technical slice.
 * COUNT is used as the terminal navigation limit.
 */

typedef enum Floppy144CollectionId
{
    FLOPPY144_COLLECTION_XX01 = 0,
    FLOPPY144_COLLECTION_HR02 = 1,
    FLOPPY144_COLLECTION_FA03 = 2,
    FLOPPY144_COLLECTION_COUNT = 3
} Floppy144CollectionId;
