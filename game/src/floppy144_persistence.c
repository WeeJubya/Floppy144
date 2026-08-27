#include "floppy144_persistence.h"

#include <stddef.h>
#include <stdio.h>

static void Floppy144PersistenceWriteU32
(
    uint8_t *destination,
 uint32_t value
){
    destination[0] =
    (uint8_t)(value & 0xffU);

    destination[1] =
    (uint8_t)((value >> 8U) & 0xffU);

    destination[2] =
    (uint8_t)((value >> 16U) & 0xffU);

    destination[3] =
    (uint8_t)((value >> 24U) & 0xffU);
}

static uint32_t Floppy144PersistenceReadU32
(
    const uint8_t *source
){
    return
    (uint32_t)source[0] |
    ((uint32_t)source[1] << 8U) |
    ((uint32_t)source[2] << 16U) |
    ((uint32_t)source[3] << 24U);
}

static void Floppy144PersistenceEncodeHeader
(
    uint8_t *destination,
 const Floppy144SaveHeader *header
){
    Floppy144PersistenceWriteU32(
        &destination[0],
        header->magic
    );

    Floppy144PersistenceWriteU32(
        &destination[4],
        header->version
    );

    Floppy144PersistenceWriteU32(
        &destination[8],
        header->payload_size
    );

    Floppy144PersistenceWriteU32(
        &destination[12],
        header->checksum
    );
}

static void Floppy144PersistenceDecodeHeader
(
    Floppy144SaveHeader *header,
 const uint8_t *source
){
    header->magic =
    Floppy144PersistenceReadU32(
        &source[0]
    );

    header->version =
    Floppy144PersistenceReadU32(
        &source[4]
    );

    header->payload_size =
    Floppy144PersistenceReadU32(
        &source[8]
    );

    header->checksum =
    Floppy144PersistenceReadU32(
        &source[12]
    );
}

bool Floppy144PersistenceEncodeRunState
(
    const Floppy144RunState *state,
 uint8_t *payload,
 uint32_t payload_size
){
    uint32_t offset =
    0U;

    uint32_t index;

    if(
        state == NULL ||
        payload == NULL ||
        payload_size !=
        FLOPPY144_SAVE_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    Floppy144PersistenceWriteU32(
        &payload[offset],
        state->recovery_seed
    );

    offset += 4U;

    payload[offset++] =
    state->act;

    payload[offset++] =
    state->branch;

    payload[offset++] =
    state->projection;

    payload[offset++] =
    state->archive_services_initialised != 0U
    ? 1U
    : 0U;

    Floppy144PersistenceWriteU32(
        &payload[offset],
        (uint32_t)state->player_site_x
    );

    offset += 4U;

    Floppy144PersistenceWriteU32(
        &payload[offset],
        (uint32_t)state->player_site_y
    );

    offset += 4U;

    #define FLOPPY144_WRITE_WORD_ARRAY(array_name)                     \
    for(                                                           \
        index = 0U;                                                \
        index <                                                    \
        (uint32_t)(sizeof(state->array_name) /                 \
        sizeof(state->array_name[0]));                         \
        ++index                                                    \
    )                                                              \
    {                                                              \
        Floppy144PersistenceWriteU32(                              \
        &payload[offset],                                      \
        state->array_name[index]                               \
        );                                                         \
        offset += 4U;                                              \
    }

    FLOPPY144_WRITE_WORD_ARRAY(rooms)
    FLOPPY144_WRITE_WORD_ARRAY(objects_visible)
    FLOPPY144_WRITE_WORD_ARRAY(objects_unlocked)
    FLOPPY144_WRITE_WORD_ARRAY(objects_open)
    FLOPPY144_WRITE_WORD_ARRAY(collections)
    FLOPPY144_WRITE_WORD_ARRAY(triggers)
    FLOPPY144_WRITE_WORD_ARRAY(interactions)
    FLOPPY144_WRITE_WORD_ARRAY(evidence)
    FLOPPY144_WRITE_WORD_ARRAY(notebook)
    FLOPPY144_WRITE_WORD_ARRAY(capabilities)

    #undef FLOPPY144_WRITE_WORD_ARRAY

    return
    offset ==
    FLOPPY144_SAVE_PAYLOAD_V1_SIZE;
}

bool Floppy144PersistenceDecodeRunState
(
    Floppy144RunState *state,
 const uint8_t *payload,
 uint32_t payload_size
){
    Floppy144RunState decoded;

    uint32_t offset =
    0U;

    uint32_t index;

    if(
        state == NULL ||
        payload == NULL ||
        payload_size !=
        FLOPPY144_SAVE_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    Floppy144RunStateReset(
        &decoded
    );

    decoded.recovery_seed =
    Floppy144PersistenceReadU32(
        &payload[offset]
    );

    offset += 4U;

    decoded.act =
    payload[offset++];

    decoded.branch =
    payload[offset++];

    decoded.projection =
    payload[offset++];

    decoded.archive_services_initialised =
    payload[offset++];

    decoded.player_site_x =
    (int32_t)Floppy144PersistenceReadU32(
        &payload[offset]
    );

    offset += 4U;

    decoded.player_site_y =
    (int32_t)Floppy144PersistenceReadU32(
        &payload[offset]
    );

    offset += 4U;

    #define FLOPPY144_READ_WORD_ARRAY(array_name)                      \
    for(                                                           \
        index = 0U;                                                \
        index <                                                    \
        (uint32_t)(sizeof(decoded.array_name) /                \
        sizeof(decoded.array_name[0]));                        \
        ++index                                                    \
    )                                                              \
    {                                                              \
        decoded.array_name[index] =                                \
        Floppy144PersistenceReadU32(                           \
        &payload[offset]                                   \
        );                                                     \
        offset += 4U;                                              \
    }

    FLOPPY144_READ_WORD_ARRAY(rooms)
    FLOPPY144_READ_WORD_ARRAY(objects_visible)
    FLOPPY144_READ_WORD_ARRAY(objects_unlocked)
    FLOPPY144_READ_WORD_ARRAY(objects_open)
    FLOPPY144_READ_WORD_ARRAY(collections)
    FLOPPY144_READ_WORD_ARRAY(triggers)
    FLOPPY144_READ_WORD_ARRAY(interactions)
    FLOPPY144_READ_WORD_ARRAY(evidence)
    FLOPPY144_READ_WORD_ARRAY(notebook)
    FLOPPY144_READ_WORD_ARRAY(capabilities)

    #undef FLOPPY144_READ_WORD_ARRAY

    if(
        offset !=
        FLOPPY144_SAVE_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    if(
        decoded.act >=
        (uint8_t)FLOPPY144_RUN_ACT_COMPLETE + 1U ||
        decoded.branch >
        (uint8_t)FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST ||
        decoded.projection >=
        (uint8_t)FLOPPY144_PROJECTION_COUNT ||
        decoded.archive_services_initialised > 1U
    )
    {
        return false;
    }

    /*
     * A freshly loaded state matches its saved representation and therefore
     * begins clean.
     */

    decoded.dirty =
    0U;

    *state =
    decoded;

    return true;
}

uint32_t Floppy144PersistenceChecksum
(
    const void *data,
 uint32_t size
){
    const uint8_t *bytes =
    (const uint8_t *)data;

    uint32_t hash =
    2166136261U;

    uint32_t index;

    if(
        data == NULL &&
        size > 0U
    )
    {
        return 0U;
    }

    for(
        index = 0U;
    index < size;
    ++index
    )
    {
        hash ^=
        bytes[index];

        hash *=
        16777619U;
    }

    return hash;
}

bool Floppy144PersistenceHeaderValid
(
    const Floppy144SaveHeader *header,
 uint32_t expected_payload_size
){
    if(header == NULL)
    {
        return false;
    }

    return
    header->magic ==
    FLOPPY144_SAVE_MAGIC &&
    header->version ==
    FLOPPY144_SAVE_VERSION &&
    header->payload_size ==
    expected_payload_size;
}

bool Floppy144PersistenceSaveRunState
(
    const char *path,
 Floppy144RunState *state
){
    uint8_t file_data[
        FLOPPY144_SAVE_FILE_V1_SIZE
    ];

    uint8_t *payload =
    &file_data[FLOPPY144_SAVE_HEADER_SIZE];

    Floppy144SaveHeader header;

    FILE *file;

    size_t written;

    if(
        path == NULL ||
        state == NULL
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceEncodeRunState(
            state,
            payload,
            FLOPPY144_SAVE_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    header.magic =
    FLOPPY144_SAVE_MAGIC;

    header.version =
    FLOPPY144_SAVE_VERSION;

    header.payload_size =
    FLOPPY144_SAVE_PAYLOAD_V1_SIZE;

    header.checksum =
    Floppy144PersistenceChecksum(
        payload,
        FLOPPY144_SAVE_PAYLOAD_V1_SIZE
    );

    Floppy144PersistenceEncodeHeader(
        file_data,
        &header
    );

    file =
    NULL;

    if(
        fopen_s(
            &file,
            path,
            "wb"
        ) != 0 ||
        file == NULL
    )
    {
        return false;
    }

    written =
    fwrite(
        file_data,
        1U,
        sizeof(file_data),
           file
    );

    if(
        fclose(file) != 0 ||
        written != sizeof(file_data)
    )
    {
        return false;
    }

    state->dirty =
    0U;

    return true;
}

bool Floppy144PersistenceLoadRunState
(
    const char *path,
 Floppy144RunState *state
){
    uint8_t file_data[
        FLOPPY144_SAVE_FILE_V1_SIZE
    ];

    const uint8_t *payload =
    &file_data[FLOPPY144_SAVE_HEADER_SIZE];

    Floppy144SaveHeader header;

    Floppy144RunState decoded;

    FILE *file;

    size_t read;

    int trailing_byte;

    if(
        path == NULL ||
        state == NULL
    )
    {
        return false;
    }

    file =
    NULL;

    if(
        fopen_s(
            &file,
            path,
            "rb"
        ) != 0 ||
        file == NULL
    )
    {
        return false;
    }

    read =
    fread(
        file_data,
        1U,
        sizeof(file_data),
          file
    );

    trailing_byte =
    fgetc(file);

    if(fclose(file) != 0)
    {
        return false;
    }

    if(
        read != sizeof(file_data) ||
        trailing_byte != EOF
    )
    {
        return false;
    }

    Floppy144PersistenceDecodeHeader(
        &header,
        file_data
    );

    if(
        !Floppy144PersistenceHeaderValid(
            &header,
            FLOPPY144_SAVE_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    if(
        Floppy144PersistenceChecksum(
            payload,
            FLOPPY144_SAVE_PAYLOAD_V1_SIZE
        ) != header.checksum
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceDecodeRunState(
            &decoded,
            payload,
            FLOPPY144_SAVE_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    *state =
    decoded;

    return true;
}
