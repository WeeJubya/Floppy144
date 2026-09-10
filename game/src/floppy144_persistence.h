#pragma once

#include "floppy144_run_state.h"
#include "floppy144_profile.h"
#include "floppy144_settings.h"

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

/*
 * Run-state payload size.
 *
 * Keep this derived from the actual persisted arrays rather than a manually
 * maintained byte count. Registry growth can change bitset storage width.
 *
 * Fixed scalar prefix:
 *   recovery_seed                 4
 *   act/branch/projection/init    4
 *   player_site_x                 4
 *   player_site_y                 4
 *                                --
 *                                16 bytes
 */
#define FLOPPY144_SAVE_PAYLOAD_V1_SIZE                            \
(                                                                 \
    16U +                                                         \
    sizeof(((Floppy144RunState *)0)->rooms) +                     \
    sizeof(((Floppy144RunState *)0)->objects_visible) +           \
    sizeof(((Floppy144RunState *)0)->objects_unlocked) +          \
    sizeof(((Floppy144RunState *)0)->objects_open) +              \
    sizeof(((Floppy144RunState *)0)->collections) +               \
    sizeof(((Floppy144RunState *)0)->triggers) +                  \
    sizeof(((Floppy144RunState *)0)->interactions) +              \
    sizeof(((Floppy144RunState *)0)->evidence) +                  \
    sizeof(((Floppy144RunState *)0)->notebook) +                  \
    sizeof(((Floppy144RunState *)0)->capabilities)                \
)

#define FLOPPY144_SAVE_HEADER_SIZE     16U
#define FLOPPY144_SAVE_FILE_V1_SIZE    \
    (FLOPPY144_SAVE_HEADER_SIZE + FLOPPY144_SAVE_PAYLOAD_V1_SIZE)

#define FLOPPY144_PROFILE_MAGIC       0x34343150U
#define FLOPPY144_PROFILE_VERSION     1U

#define FLOPPY144_PROFILE_PAYLOAD_V1_SIZE 64U
#define FLOPPY144_PROFILE_FILE_V1_SIZE    \
    (FLOPPY144_SAVE_HEADER_SIZE + FLOPPY144_PROFILE_PAYLOAD_V1_SIZE)

#define FLOPPY144_SETTINGS_MAGIC       0x34343153U
#define FLOPPY144_SETTINGS_VERSION     1U

#define FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE 16U
#define FLOPPY144_SETTINGS_FILE_V1_SIZE    \
    (FLOPPY144_SAVE_HEADER_SIZE + FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE)

bool Floppy144PersistenceEncodeRunState
(
    const Floppy144RunState *state,
 uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceEncodeProfile
(
    const Floppy144DiscoveryProfile *profile,
 uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceDecodeRunState
(
    Floppy144RunState *state,
 const uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceDecodeProfile
(
    Floppy144DiscoveryProfile *profile,
 const uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceSaveProfile
(
    const char *path,
 Floppy144DiscoveryProfile *profile
);

bool Floppy144PersistenceLoadProfile
(
    const char *path,
 Floppy144DiscoveryProfile *profile
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

bool Floppy144PersistenceEncodeSettings
(
    const Floppy144Settings *settings,
 uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceDecodeSettings
(
    Floppy144Settings *settings,
 const uint8_t *payload,
 uint32_t payload_size
);

bool Floppy144PersistenceSaveSettings
(
    const char *path,
 Floppy144Settings *settings
);

bool Floppy144PersistenceLoadSettings
(
    const char *path,
 Floppy144Settings *settings
);
