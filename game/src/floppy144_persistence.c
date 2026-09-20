#include "floppy144_persistence.h"
#include "floppy144_site.h"
#include "floppy144_game_data.h"

#include <windows.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

static uint16_t Floppy144PersistenceReadU16(const uint8_t *pSource)
{
    return (uint16_t)((uint16_t)pSource[0] | ((uint16_t)pSource[1] << 8U));
}

static void Floppy144PersistenceWriteU16(uint8_t *pDestination,uint16_t uValue)
{
    pDestination[0]=(uint8_t)(uValue & 0xffU);
    pDestination[1]=(uint8_t)((uValue >> 8U) & 0xffU);
}

static bool Floppy144PersistenceReplaceFile
(
    const char *path,
    const uint8_t *data,
    uint32_t data_size
)
{
    char temporary_path[MAX_PATH];

    size_t path_length;

    FILE *file =
    NULL;

    size_t written;

    if(
        path == NULL ||
        data == NULL ||
        data_size == 0U
    )
    {
        return false;
    }

    path_length =
    strlen(path);

    /*
     * Four characters for ".tmp" plus the terminating NUL.
     */
    if(
        path_length + 5U >
        sizeof(temporary_path)
    )
    {
        return false;
    }

    memcpy(
        temporary_path,
        path,
        path_length
    );

    memcpy(
        &temporary_path[path_length],
        ".tmp",
        5U
    );

    /*
     * Write alongside the destination so the final rename remains on the
     * same filesystem. A stale temporary file from an interrupted previous
     * attempt is safe to overwrite.
     */
    if(
        fopen_s(
            &file,
            temporary_path,
            "wb"
        ) != 0 ||
        file == NULL
    )
    {
        return false;
    }

    written =
    fwrite(
        data,
        1U,
        (size_t)data_size,
           file
    );

    if(
        fclose(file) != 0 ||
        written != (size_t)data_size
    )
    {
        DeleteFileA(
            temporary_path
        );

        return false;
    }

    /*
     * The old valid destination remains intact until the new temporary file
     * has been written and closed successfully.
     */
    if(
        !MoveFileExA(
            temporary_path,
            path,
            MOVEFILE_REPLACE_EXISTING |
            MOVEFILE_WRITE_THROUGH
        )
    )
    {
        DeleteFileA(
            temporary_path
        );

        return false;
    }

    return true;
}

static bool Floppy144PersistenceWordArrayValid
(
    const uint32_t *words,
    uint32_t word_count,
    uint32_t bit_count
)
{
    uint32_t used_word_count;
    uint32_t used_bits;
    uint32_t valid_mask;
    uint32_t index;

    if(
        words == NULL ||
        word_count == 0U ||
        bit_count == 0U
    )
    {
        return false;
    }

    used_word_count =
    (bit_count + FLOPPY144_RUN_WORD_BITS - 1U) /
    FLOPPY144_RUN_WORD_BITS;

    if(used_word_count > word_count)
    {
        return false;
    }

    used_bits =
    bit_count %
    FLOPPY144_RUN_WORD_BITS;

    if(used_bits != 0U)
    {
        valid_mask =
        (1U << used_bits) - 1U;

        if(
            (words[used_word_count - 1U] & ~valid_mask) != 0U
        )
        {
            return false;
        }
    }

    /*
     * Arrays may deliberately reserve more storage than the current registry
     * requires. Those unused words must remain clear.
     */

    for(
        index = used_word_count;
    index < word_count;
    ++index
    )
    {
        if(words[index] != 0U)
        {
            return false;
        }
    }

    return true;
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

static bool Floppy144PersistenceProfileHeaderValid
(
    const Floppy144SaveHeader *header
){
    if(header == NULL)
    {
        return false;
    }

    return
    header->magic ==
    FLOPPY144_PROFILE_MAGIC &&
    header->version ==
    FLOPPY144_PROFILE_VERSION &&
    header->payload_size ==
    FLOPPY144_PROFILE_PAYLOAD_V1_SIZE;
}

static bool Floppy144PersistenceSettingsHeaderValid
(
    const Floppy144SaveHeader *header
){
    if(header == NULL)
    {
        return false;
    }

    return
    header->magic ==
    FLOPPY144_SETTINGS_MAGIC &&
    header->version ==
    FLOPPY144_SETTINGS_VERSION &&
    header->payload_size ==
    FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE;
}

bool Floppy144PersistenceEncodeRunState
(
    const Floppy144RunState *state,
    uint8_t *payload,
    uint32_t payload_size
){
    uint32_t offset=0U;
    uint32_t index;
    bool bV2=payload_size==FLOPPY144_SAVE_PAYLOAD_V2_SIZE;

    if(
        state==NULL || payload==NULL ||
        (payload_size!=FLOPPY144_SAVE_PAYLOAD_V1_SIZE && !bV2)
    ) return false;

    Floppy144PersistenceWriteU32(&payload[offset],state->recovery_seed); offset+=4U;
    payload[offset++]=state->act;
    payload[offset++]=state->branch;
    payload[offset++]=state->projection;
    payload[offset++]=state->archive_services_initialised!=0U?1U:0U;
    Floppy144PersistenceWriteU32(&payload[offset],(uint32_t)state->player_site_x); offset+=4U;
    Floppy144PersistenceWriteU32(&payload[offset],(uint32_t)state->player_site_y); offset+=4U;
    Floppy144PersistenceWriteU32(&payload[offset],state->secure_cabinets_unlocked); offset+=4U;

    #define FLOPPY144_WRITE_WORD_ARRAY(array_name) \
    for(index=0U;index<(uint32_t)(sizeof(state->array_name)/sizeof(state->array_name[0]));++index) \
    { Floppy144PersistenceWriteU32(&payload[offset],state->array_name[index]); offset+=4U; }

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

    if(offset!=FLOPPY144_SAVE_PAYLOAD_V1_SIZE) return false;
    if(!bV2) return true;

    if(state->notebook_order_count>FLOPPY144_NOTEBOOK_ORDER_MAX) return false;
    for(index=0U;index<(uint32_t)state->notebook_order_count;++index)
    {
        uint32_t uOther;
        if((uint32_t)state->notebook_order[index]>=Floppy144GameDataNotebookRecordCount()) return false;
        for(uOther=0U;uOther<index;++uOther)
            if(state->notebook_order[uOther]==state->notebook_order[index]) return false;
    }

    Floppy144PersistenceWriteU16(&payload[offset],state->notebook_order_count); offset+=2U;
    for(index=0U;index<FLOPPY144_NOTEBOOK_ORDER_MAX;++index)
    {
        uint16_t uOrdinal=index<(uint32_t)state->notebook_order_count?state->notebook_order[index]:0U;
        Floppy144PersistenceWriteU16(&payload[offset],uOrdinal); offset+=2U;
    }
    return offset==FLOPPY144_SAVE_PAYLOAD_V2_SIZE;
}

bool Floppy144PersistenceDecodeRunState
(
    Floppy144RunState *state,
    const uint8_t *payload,
    uint32_t payload_size
){
    Floppy144RunState decoded;
    uint32_t offset=0U;
    uint32_t index;
    bool bV2=payload_size==FLOPPY144_SAVE_PAYLOAD_V2_SIZE;

    if(
        state==NULL || payload==NULL ||
        (payload_size!=FLOPPY144_SAVE_PAYLOAD_V1_SIZE && !bV2)
    ) return false;

    Floppy144RunStateReset(&decoded);
    decoded.recovery_seed=Floppy144PersistenceReadU32(&payload[offset]); offset+=4U;
    decoded.act=payload[offset++];
    decoded.branch=payload[offset++];
    decoded.projection=payload[offset++];
    decoded.archive_services_initialised=payload[offset++];
    decoded.player_site_x=(int32_t)Floppy144PersistenceReadU32(&payload[offset]); offset+=4U;
    decoded.player_site_y=(int32_t)Floppy144PersistenceReadU32(&payload[offset]); offset+=4U;
    decoded.secure_cabinets_unlocked=Floppy144PersistenceReadU32(&payload[offset]); offset+=4U;

    #define FLOPPY144_READ_WORD_ARRAY(array_name) \
    for(index=0U;index<(uint32_t)(sizeof(decoded.array_name)/sizeof(decoded.array_name[0]));++index) \
    { decoded.array_name[index]=Floppy144PersistenceReadU32(&payload[offset]); offset+=4U; }

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

    if(offset!=FLOPPY144_SAVE_PAYLOAD_V1_SIZE) return false;

    if(bV2)
    {
        uint16_t uCount=Floppy144PersistenceReadU16(&payload[offset]); offset+=2U;
        if((uint32_t)uCount>FLOPPY144_NOTEBOOK_ORDER_MAX) return false;
        decoded.notebook_order_count=uCount;
        for(index=0U;index<FLOPPY144_NOTEBOOK_ORDER_MAX;++index)
        {
            uint16_t uOrdinal=Floppy144PersistenceReadU16(&payload[offset]);
            uint32_t uOther;
            offset+=2U;
            if(index>=(uint32_t)uCount)
            {
                if(uOrdinal!=0U) return false;
                continue;
            }
            if((uint32_t)uOrdinal>=Floppy144GameDataNotebookRecordCount()) return false;
            for(uOther=0U;uOther<index;++uOther)
                if(decoded.notebook_order[uOther]==uOrdinal) return false;
            decoded.notebook_order[index]=uOrdinal;
        }
        if(offset!=FLOPPY144_SAVE_PAYLOAD_V2_SIZE) return false;
    }

    if(Floppy144SitePositionBlocked(decoded.player_site_x,decoded.player_site_y)) return false;

    if(
        !Floppy144PersistenceWordArrayValid(decoded.rooms,(uint32_t)(sizeof(decoded.rooms)/sizeof(decoded.rooms[0])),(uint32_t)FLOPPY144_ROOM_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.objects_visible,(uint32_t)(sizeof(decoded.objects_visible)/sizeof(decoded.objects_visible[0])),(uint32_t)FLOPPY144_OBJECT_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.objects_unlocked,(uint32_t)(sizeof(decoded.objects_unlocked)/sizeof(decoded.objects_unlocked[0])),(uint32_t)FLOPPY144_OBJECT_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.objects_open,(uint32_t)(sizeof(decoded.objects_open)/sizeof(decoded.objects_open[0])),(uint32_t)FLOPPY144_OBJECT_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.collections,(uint32_t)(sizeof(decoded.collections)/sizeof(decoded.collections[0])),(uint32_t)FLOPPY144_COLLECTION_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.triggers,(uint32_t)(sizeof(decoded.triggers)/sizeof(decoded.triggers[0])),(uint32_t)FLOPPY144_TRIGGER_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.interactions,(uint32_t)(sizeof(decoded.interactions)/sizeof(decoded.interactions[0])),(uint32_t)FLOPPY144_INTERACTION_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.evidence,(uint32_t)(sizeof(decoded.evidence)/sizeof(decoded.evidence[0])),(uint32_t)FLOPPY144_EVIDENCE_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.notebook,(uint32_t)(sizeof(decoded.notebook)/sizeof(decoded.notebook[0])),(uint32_t)FLOPPY144_NOTEBOOK_COUNT) ||
        !Floppy144PersistenceWordArrayValid(decoded.capabilities,(uint32_t)(sizeof(decoded.capabilities)/sizeof(decoded.capabilities[0])),(uint32_t)FLOPPY144_CAPABILITY_COUNT)
    ) return false;

    if(
        decoded.act>=(uint8_t)FLOPPY144_RUN_ACT_COMPLETE+1U ||
        decoded.branch>(uint8_t)FLOPPY144_RUN_BRANCH_TECHNOLOGY_FIRST ||
        decoded.projection>=(uint8_t)FLOPPY144_PROJECTION_COUNT ||
        decoded.archive_services_initialised>1U
    ) return false;

    if(!bV2)
    {
        /* V1 had no chronology. Reconstruct one deterministically from the
           persisted knowledge/progression state, then mark the migrated load clean. */
        Floppy144GameDataCaptureNewNotebookEntries(&decoded);
    }

    decoded.dirty=0U;
    *state=decoded;
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
    if(header==NULL || header->magic!=FLOPPY144_SAVE_MAGIC) return false;
    if(header->payload_size!=expected_payload_size) return false;
    if(header->version==FLOPPY144_SAVE_VERSION_V1)
        return expected_payload_size==FLOPPY144_SAVE_PAYLOAD_V1_SIZE;
    if(header->version==FLOPPY144_SAVE_VERSION)
        return expected_payload_size==FLOPPY144_SAVE_PAYLOAD_V2_SIZE;
    return false;
}

bool Floppy144PersistenceSaveRunState
(
    const char *path,
    Floppy144RunState *state
){
    uint8_t file_data[FLOPPY144_SAVE_FILE_V2_SIZE];
    uint8_t *payload=&file_data[FLOPPY144_SAVE_HEADER_SIZE];
    Floppy144SaveHeader header;

    if(path==NULL||state==NULL) return false;
    if(!Floppy144PersistenceEncodeRunState(state,payload,FLOPPY144_SAVE_PAYLOAD_V2_SIZE)) return false;

    header.magic=FLOPPY144_SAVE_MAGIC;
    header.version=FLOPPY144_SAVE_VERSION;
    header.payload_size=FLOPPY144_SAVE_PAYLOAD_V2_SIZE;
    header.checksum=Floppy144PersistenceChecksum(payload,FLOPPY144_SAVE_PAYLOAD_V2_SIZE);
    Floppy144PersistenceEncodeHeader(file_data,&header);
    if(!Floppy144PersistenceReplaceFile(path,file_data,(uint32_t)sizeof(file_data))) return false;
    state->dirty=0U;
    return true;
}

bool Floppy144PersistenceLoadRunState
(
    const char *path,
    Floppy144RunState *state
){
    uint8_t file_data[FLOPPY144_SAVE_FILE_V2_SIZE];
    const uint8_t *payload=&file_data[FLOPPY144_SAVE_HEADER_SIZE];
    Floppy144SaveHeader header;
    Floppy144RunState decoded;
    FILE *file=NULL;
    size_t read;
    int trailing_byte;
    uint32_t expected_payload;
    uint32_t expected_file_size;

    if(path==NULL||state==NULL) return false;
    if(fopen_s(&file,path,"rb")!=0||file==NULL) return false;
    read=fread(file_data,1U,sizeof(file_data),file);
    trailing_byte=fgetc(file);
    if(fclose(file)!=0) return false;
    if(read<FLOPPY144_SAVE_HEADER_SIZE||trailing_byte!=EOF) return false;

    Floppy144PersistenceDecodeHeader(&header,file_data);
    if(header.magic!=FLOPPY144_SAVE_MAGIC) return false;

    if(header.version==FLOPPY144_SAVE_VERSION_V1)
        expected_payload=FLOPPY144_SAVE_PAYLOAD_V1_SIZE;
    else if(header.version==FLOPPY144_SAVE_VERSION)
        expected_payload=FLOPPY144_SAVE_PAYLOAD_V2_SIZE;
    else
        return false;

    expected_file_size=FLOPPY144_SAVE_HEADER_SIZE+expected_payload;
    if(header.payload_size!=expected_payload||read!=(size_t)expected_file_size) return false;
    if(Floppy144PersistenceChecksum(payload,expected_payload)!=header.checksum) return false;
    if(!Floppy144PersistenceDecodeRunState(&decoded,payload,expected_payload)) return false;

    *state=decoded;
    return true;
}

bool Floppy144PersistenceEncodeProfile
(
    const Floppy144DiscoveryProfile *profile,
 uint8_t *payload,
 uint32_t payload_size
)
{
    uint32_t offset =
    0U;

    uint32_t index;

    if(
        profile == NULL ||
        payload == NULL ||
        payload_size !=
        FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    memset(
        payload,
        0,
        payload_size
    );

    memcpy(
        &payload[offset],
        profile->operator_name,
        FLOPPY144_PROFILE_NAME_CAPACITY
    );

    offset +=
    FLOPPY144_PROFILE_NAME_CAPACITY;

    payload[offset++] =
    profile->body_style;

    /*
     * Three reserved scalar bytes.
     */
    offset +=
    3U;

    Floppy144PersistenceWriteU32(
        &payload[offset],
        profile->recovery_sessions_begun
    );

    offset +=
    4U;

    for(
        index = 0U;
    index <
    (uint32_t)(
        sizeof(
            profile->collections_ever_restored
        ) /
        sizeof(
            profile->collections_ever_restored[0]
        )
    );
    ++index
    )
    {
        Floppy144PersistenceWriteU32(
            &payload[offset],
            profile->collections_ever_restored[index]
        );

        offset +=
        4U;
    }

    for(
        index = 0U;
    index <
    (uint32_t)(
        sizeof(
            profile->evidence_ever_established
        ) /
        sizeof(
            profile->evidence_ever_established[0]
        )
    );
    ++index
    )
    {
        Floppy144PersistenceWriteU32(
            &payload[offset],
            profile->evidence_ever_established[index]
        );

        offset +=
        4U;
    }

    /*
     * The remaining V1 bytes stay zero and are reserved for future
     * cumulative discovery fields.
     */
    return
    offset <=
    FLOPPY144_PROFILE_PAYLOAD_V1_SIZE;
}

bool Floppy144PersistenceSaveProfile
(
    const char *path,
 Floppy144DiscoveryProfile *profile
){
    uint8_t file_data[
        FLOPPY144_PROFILE_FILE_V1_SIZE
    ];

    uint8_t *payload =
    &file_data[FLOPPY144_SAVE_HEADER_SIZE];

    Floppy144SaveHeader header;

    if(
        path == NULL ||
        profile == NULL
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceEncodeProfile(
            profile,
            payload,
            FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    header.magic =
    FLOPPY144_PROFILE_MAGIC;

    header.version =
    FLOPPY144_PROFILE_VERSION;

    header.payload_size =
    FLOPPY144_PROFILE_PAYLOAD_V1_SIZE;

    header.checksum =
    Floppy144PersistenceChecksum(
        payload,
        FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
    );

    Floppy144PersistenceEncodeHeader(
        file_data,
        &header
    );

    if(
        !Floppy144PersistenceReplaceFile(
            path,
            file_data,
            (uint32_t)sizeof(file_data)
        )
    )
    {
        return false;
    }

    profile->dirty =
    0U;

    return true;
}

bool Floppy144PersistenceLoadProfile
(
    const char *path,
 Floppy144DiscoveryProfile *profile
){
    uint8_t file_data[
        FLOPPY144_PROFILE_FILE_V1_SIZE
    ];

    const uint8_t *payload =
    &file_data[FLOPPY144_SAVE_HEADER_SIZE];

    Floppy144SaveHeader header;

    Floppy144DiscoveryProfile decoded;

    FILE *file =
    NULL;

    size_t read;

    int trailing_byte;

    if(
        path == NULL ||
        profile == NULL
    )
    {
        return false;
    }

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
        !Floppy144PersistenceProfileHeaderValid(
            &header
        )
    )
    {
        return false;
    }

    if(
        Floppy144PersistenceChecksum(
            payload,
            FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
        ) != header.checksum
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceDecodeProfile(
            &decoded,
            payload,
            FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    *profile =
    decoded;

    return true;
}

bool Floppy144PersistenceDecodeProfile
(
    Floppy144DiscoveryProfile *profile,
 const uint8_t *payload,
 uint32_t payload_size
)
{
    Floppy144DiscoveryProfile decoded;

    uint32_t offset =
    0U;

    uint32_t index;

    bool name_terminated =
    false;

    if(
        profile == NULL ||
        payload == NULL ||
        payload_size !=
        FLOPPY144_PROFILE_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    Floppy144DiscoveryProfileReset(
        &decoded
    );

    memcpy(
        decoded.operator_name,
        &payload[offset],
        FLOPPY144_PROFILE_NAME_CAPACITY
    );

    offset +=
    FLOPPY144_PROFILE_NAME_CAPACITY;

    /*
     * Stored operator names must contain a terminator inside the fixed
     * profile field.
     */
    for(
        index = 0U;
    index <
    FLOPPY144_PROFILE_NAME_CAPACITY;
    ++index
    )
    {
        if(decoded.operator_name[index] == '\0')
        {
            name_terminated =
            true;

            break;
        }
    }

    if(!name_terminated)
    {
        return false;
    }

    decoded.body_style =
    payload[offset++];

    /*
     * Reserved V1 bytes must remain zero.
     */
    if(
        payload[offset] != 0U ||
        payload[offset + 1U] != 0U ||
        payload[offset + 2U] != 0U
    )
    {
        return false;
    }

    offset +=
    3U;

    decoded.recovery_sessions_begun =
    Floppy144PersistenceReadU32(
        &payload[offset]
    );

    offset +=
    4U;

    for(
        index = 0U;
    index <
    (uint32_t)(
        sizeof(
            decoded.collections_ever_restored
        ) /
        sizeof(
            decoded.collections_ever_restored[0]
        )
    );
    ++index
    )
    {
        decoded.collections_ever_restored[index] =
        Floppy144PersistenceReadU32(
            &payload[offset]
        );

        offset +=
        4U;
    }

    for(
        index = 0U;
    index <
    (uint32_t)(
        sizeof(
            decoded.evidence_ever_established
        ) /
        sizeof(
            decoded.evidence_ever_established[0]
        )
    );
    ++index
    )
    {
        decoded.evidence_ever_established[index] =
        Floppy144PersistenceReadU32(
            &payload[offset]
        );

        offset +=
        4U;
    }

    /*
     * Discovery storage deliberately has spare capacity. Bits referring to
     * collection or evidence IDs which do not currently exist must remain
     * clear.
     */
    if(
        !Floppy144PersistenceWordArrayValid(
            decoded.collections_ever_restored,
            (uint32_t)(
                sizeof(decoded.collections_ever_restored) /
                sizeof(decoded.collections_ever_restored[0])
            ),
            (uint32_t)FLOPPY144_COLLECTION_COUNT
        ) ||
        !Floppy144PersistenceWordArrayValid(
            decoded.evidence_ever_established,
            (uint32_t)(
                sizeof(decoded.evidence_ever_established) /
                sizeof(decoded.evidence_ever_established[0])
            ),
            (uint32_t)FLOPPY144_EVIDENCE_COUNT
        )
    )
    {
        return false;
    }

    /*
     * The remainder of the V1 payload is reserved and must remain zero.
     */
    for(
        ;
    offset <
    FLOPPY144_PROFILE_PAYLOAD_V1_SIZE;
    ++offset
    )
    {
        if(payload[offset] != 0U)
        {
            return false;
        }
    }

    if(
        decoded.body_style >=
        (uint8_t)FLOPPY144_OPERATOR_BODY_STYLE_COUNT
    )
    {
        return false;
    }

    decoded.dirty =
    0U;

    *profile =
    decoded;

    return true;
}

bool Floppy144PersistenceEncodeSettings
(
    const Floppy144Settings *settings,
 uint8_t *payload,
 uint32_t payload_size
){
    if(
        settings == NULL ||
        payload == NULL ||
        payload_size !=
        FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    memset(
        payload,
        0,
        payload_size
    );

    payload[0] =
    settings->crt_mode;

    payload[1] =
    settings->text_speed;

    payload[2] =
    settings->music_volume;

    payload[3] =
    settings->sfx_volume;

    payload[4] =
    settings->autosave_mode;

    return true;
}

bool Floppy144PersistenceDecodeSettings
(
    Floppy144Settings *settings,
 const uint8_t *payload,
 uint32_t payload_size
){
    Floppy144Settings decoded;

    if(
        settings == NULL ||
        payload == NULL ||
        payload_size !=
        FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
    )
    {
        return false;
    }

    Floppy144SettingsReset(
        &decoded
    );

    decoded.crt_mode =
    payload[0];

    decoded.text_speed =
    payload[1];

    decoded.music_volume =
    payload[2];

    decoded.sfx_volume =
    payload[3];

    decoded.autosave_mode =
    payload[4];

    if(
        decoded.crt_mode >=
        (uint8_t)FLOPPY144_CRT_COUNT ||
        decoded.text_speed >=
        (uint8_t)FLOPPY144_TEXT_SPEED_COUNT ||
        decoded.music_volume >
        FLOPPY144_SETTINGS_VOLUME_MAX ||
        decoded.sfx_volume >
        FLOPPY144_SETTINGS_VOLUME_MAX ||
        decoded.autosave_mode >=
        (uint8_t)FLOPPY144_AUTOSAVE_MODE_COUNT
    )
    {
        return false;
    }

    decoded.dirty =
    0U;

    *settings =
    decoded;

    return true;
}

bool Floppy144PersistenceSaveSettings
(
    const char *path,
 Floppy144Settings *settings
){
    uint8_t file_data[
        FLOPPY144_SETTINGS_FILE_V1_SIZE
    ];

    uint8_t *payload =
    &file_data[FLOPPY144_SAVE_HEADER_SIZE];

    Floppy144SaveHeader header;

    if(
        path == NULL ||
        settings == NULL
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceEncodeSettings(
            settings,
            payload,
            FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    header.magic =
    FLOPPY144_SETTINGS_MAGIC;

    header.version =
    FLOPPY144_SETTINGS_VERSION;

    header.payload_size =
    FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE;

    header.checksum =
    Floppy144PersistenceChecksum(
        payload,
        FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
    );

    Floppy144PersistenceEncodeHeader(
        file_data,
        &header
    );

    if(
        !Floppy144PersistenceReplaceFile(
            path,
            file_data,
            (uint32_t)sizeof(file_data)
        )
    )
    {
        return false;
    }

    settings->dirty =
    0U;

    return true;
}

bool Floppy144PersistenceLoadSettings
(
    const char *path,
 Floppy144Settings *settings
){
    uint8_t file_data[
        FLOPPY144_SETTINGS_FILE_V1_SIZE
    ];

    const uint8_t *payload =
    &file_data[FLOPPY144_SAVE_HEADER_SIZE];

    Floppy144SaveHeader header;

    Floppy144Settings decoded;

    FILE *file =
    NULL;

    size_t read;

    int trailing_byte;

    if(
        path == NULL ||
        settings == NULL
    )
    {
        return false;
    }

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
        !Floppy144PersistenceSettingsHeaderValid(
            &header
        )
    )
    {
        return false;
    }

    if(
        Floppy144PersistenceChecksum(
            payload,
            FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
        ) != header.checksum
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceDecodeSettings(
            &decoded,
            payload,
            FLOPPY144_SETTINGS_PAYLOAD_V1_SIZE
        )
    )
    {
        return false;
    }

    *settings =
    decoded;

    return true;
}
