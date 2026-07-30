/*
 * Floppy//144 - record catalogue interface
 *
 * Provides the shared browser used by restored collections. The catalogue
 * generates record listings and opens the few records recovered in full.
 */

#pragma once

#include "river2D_main.h"

#include "floppy144_collection.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Catalogue-local state
 *
 * collection selects the active archive vocabulary and authored records.
 * selected_index is the highlighted record, top_index controls scrolling,
 * and document_open switches between list and document views.
 */

typedef struct Floppy144CatalogueState
{
    Floppy144CollectionId collection;
    uint32_t selected_index;
    uint32_t top_index;
    bool document_open;
} Floppy144CatalogueState;

/*
 * Catalogue operations
 *
 * Reset opens a collection at its first record. Move and Page navigate.
 * The evidence query tells main.c whether opening the selected record
 * should unlock knowledge in the world state.
 */

void Floppy144CatalogueReset(
    Floppy144CatalogueState *catalogue,
    Floppy144CollectionId collection
);

void Floppy144CatalogueMove(
    Floppy144CatalogueState *catalogue,
    int32_t direction
);

void Floppy144CataloguePage(
    Floppy144CatalogueState *catalogue,
    int32_t direction
);

void Floppy144CatalogueOpenDocument(
    Floppy144CatalogueState *catalogue
);

void Floppy144CatalogueCloseDocument(
    Floppy144CatalogueState *catalogue
);

bool Floppy144CatalogueDocumentOpen(
    const Floppy144CatalogueState *catalogue
);

void Floppy144CatalogueDraw(
    EngineData *engine,
    const Floppy144CatalogueState *catalogue
);
