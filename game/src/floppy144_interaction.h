#pragma once

#include <stdint.h>

/*
 * Floppy//144 Interaction Ledger
 *
 * Interaction IDs identify persistent player actions that establish access,
 * evidence or capabilities during one recovery session.
 *
 * The stable I001-I040 numbering matches the design ledger. Definition order
 * is significant and must remain stable once save persistence relies on these
 * numeric IDs.
 */

typedef enum Floppy144InteractionId
{
    #define FLOPPY144_INTERACTION(symbol, code) \
    FLOPPY144_INTERACTION_##symbol,

    #include "floppy144_interactions.def"

    #undef FLOPPY144_INTERACTION

    FLOPPY144_INTERACTION_COUNT
}
Floppy144InteractionId;
