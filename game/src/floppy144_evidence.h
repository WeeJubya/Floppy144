#pragma once

#include <stdint.h>

/*
 * Floppy//144 Evidence Ledger
 *
 * Evidence IDs identify persistent facts established by physical inspection
 * and synthesis during one recovery session.
 *
 * The stable E001-E023 numbering matches the design ledger. Definition order
 * is significant and must remain stable once save persistence relies on these
 * numeric IDs.
 */

typedef enum Floppy144EvidenceId
{
    #define FLOPPY144_EVIDENCE(symbol, code) \
    FLOPPY144_EVIDENCE_##symbol,

    #include "floppy144_evidence.def"

    #undef FLOPPY144_EVIDENCE

    FLOPPY144_EVIDENCE_COUNT
}
Floppy144EvidenceId;
