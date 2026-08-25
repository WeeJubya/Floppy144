#pragma once

#include <stdint.h>

/*
 * Floppy//144 Capability Registry
 *
 * Capabilities identify persistent abilities or access rights acquired during
 * one recovery session.
 *
 * Definition order is significant and must remain stable once save
 * persistence relies on these numeric IDs.
 */

typedef enum Floppy144CapabilityId
{
    #define FLOPPY144_CAPABILITY(symbol) \
    FLOPPY144_CAPABILITY_##symbol,

    #include "floppy144_capabilities.def"

    #undef FLOPPY144_CAPABILITY

    FLOPPY144_CAPABILITY_COUNT
}
Floppy144CapabilityId;
