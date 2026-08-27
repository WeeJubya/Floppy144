#pragma once

#include "floppy144_run_state.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Floppy//144 persistence
 *
 * Save files use an explicitly versioned binary format rather than writing
 * Floppy144RunState directly. This prevents compiler padding or later runtime
 * structure changes from silently changing the on-disk format.
 */

#define FLOPPY144_SAVE_MAGIC       0x34343146U
#define FLOPPY144_SAVE_VERSION     1U

#define FLOPPY144_SAVE_PAYLOAD_V1_SIZE 64U
#define FLOPPY144_SAVE_HEADER_SIZE     16U
#define FLOPPY144_SAVE_FILE_V1_SIZE    \
(FLOPPY144_SAVE_HEADER_SIZE + FLOPPY144_SAVE_PAYLOAD_V1_SIZE)

bool Floppy144PersistenceEncodeRunState
(
    const Floppy144RunState *state,
 uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceDecodeRunState
(
    Floppy144RunState *state,
 const uint8_t *payload,
 uint32_t payload_size
);

typedef struct Floppy144SaveHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t payload_size;
    uint32_t checksum;
}
Floppy144SaveHeader;

uint32_t Floppy144PersistenceChecksum
(
    const void *data,
 uint32_t size
);

bool Floppy144PersistenceHeaderValid
(
    const Floppy144SaveHeader *header,
 uint32_t expected_payload_size
);

bool Floppy144PersistenceSaveRunState
(
    const char *path,
 Floppy144RunState *state
);

bool Floppy144PersistenceLoadRunState
(
    const char *path,
 Floppy144RunState *state
);
