#pragma once

/*
 * Floppy//144 reconstructed Site rooms.
 *
 * IDs are generated from floppy144_rooms.def and provide stable indices for
 * persistent room reconstruction state.
 *
 * Definition order becomes significant once save persistence relies on these
 * numeric IDs.
 */

typedef enum Floppy144RoomId
{
    #define FLOPPY144_ROOM(symbol, name) \
    FLOPPY144_ROOM_##symbol,

    #include "floppy144_rooms.def"

    #undef FLOPPY144_ROOM

    FLOPPY144_ROOM_COUNT
}
Floppy144RoomId;
