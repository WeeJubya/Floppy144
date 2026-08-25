#pragma once

#include <stdint.h>

/*
 * Floppy//144 Trigger Ledger
 *
 * Trigger IDs identify persistent document-driven world changes.
 *
 * The stable T001-T050 numbering matches the design ledger. Definition order
 * is therefore significant and must not be changed once save persistence
 * begins relying on these numeric IDs.
 */

typedef enum Floppy144TriggerId
{
    #define FLOPPY144_TRIGGER(symbol, code) \
    FLOPPY144_TRIGGER_##symbol,

    #include "floppy144_triggers.def"

    #undef FLOPPY144_TRIGGER

    FLOPPY144_TRIGGER_COUNT
}
Floppy144TriggerId;
