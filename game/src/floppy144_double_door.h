/* BF30: presentation-only identity for paired external doors.
 * No additional Site objects, physical children or save state are created.
 */
#pragma once
#include <stdbool.h>
#include <string.h>

/* Each leaf resolves to its pair's canonical visual identity. */
static inline const char *Floppy144DoubleDoorCanonicalId(const char *id)
{
    if(id==NULL)return NULL;
    if(strcmp(id,"ENTRY_01")==0 || strcmp(id,"ENTRY_02")==0)
        return "ENTRY_01";
    if(strcmp(id,"EMER_01")==0 || strcmp(id,"EMER_02")==0)
        return "EMER_01";
    return NULL;
}

/* Exactly one independent plaque per pair, never per leaf. */
static inline const char *Floppy144DoubleDoorSign(const char *id)
{
    const char *canonical=Floppy144DoubleDoorCanonicalId(id);
    if(canonical==NULL)return NULL;
    return strcmp(canonical,"EMER_01")==0 ?
        "EMERGENCY EXIT" : "MAIN ENTRANCE";
}

static inline bool Floppy144DoubleDoorIsPair(const char *id)
{
    return Floppy144DoubleDoorCanonicalId(id)!=NULL;
}
