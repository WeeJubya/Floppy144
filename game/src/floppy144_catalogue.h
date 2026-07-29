#pragma once

#include "river2D_main.h"

#include "floppy144_collection.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144CatalogueState
{
    Floppy144CollectionId collection;
    uint32_t selected_index;
    uint32_t top_index;
    bool document_open;
} Floppy144CatalogueState;

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

bool Floppy144CatalogueSelectedProvidesEvidence(
    const Floppy144CatalogueState *catalogue
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
