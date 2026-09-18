#pragma once

#include "floppy144_game_data.h"

#include <stdbool.h>

/* Resolve one canonical physical source to its generated interaction ID. */
Floppy144InteractionId Floppy144InteractionForPhysicalSource(
    const char *pszPhysicalSource
);

/* Return the immutable generated interaction row for one interaction ID. */
const Floppy144DataRecord *Floppy144InteractionRecord(
    Floppy144InteractionId eInteraction
);

bool Floppy144InteractionCanRun(
    const Floppy144RunState *pState,
    Floppy144InteractionId eInteraction
);

bool Floppy144InteractionTryRun(
    Floppy144WorldState *pWorld,
    Floppy144RunState *pState,
    Floppy144InteractionId eInteraction
);
