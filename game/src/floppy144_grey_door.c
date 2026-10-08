/*
 * S4G-02: completely non-destructive corridor wall overlay.
 *
 * Candidate source and collision are the generated Site rectangles. We walk
 * exposed edges of corridor floor rectangles, reject every conflicting
 * non-floor rectangle with a 2U safety halo, and verify a reachable player
 * collision footprint in the Corridor for the standard 0.5U interaction range.
 * No new generated door, opening, transition or walkable cell is introduced.
 */
#include "floppy144_grey_door.h"
#include "floppy144_site_rooms.h"
#include "floppy144_variation.h"
#include <stddef.h>
#include <string.h>

#define FLOPPY144_GREY_DOOR_MAX_CANDIDATES 256U
#define FLOPPY144_GREY_DOOR_LENGTH 4
#define FLOPPY144_GREY_DOOR_HALO 2

/* A small stable immutable cache; overfull candidate sets fail closed. */
static Floppy144GreyDoorCandidate grey_candidates[FLOPPY144_GREY_DOOR_MAX_CANDIDATES];
static uint32_t grey_candidate_count;
static bool grey_candidates_ready;
static bool grey_candidates_overflow;

uint32_t Floppy144RunStateGreyDoorPlacementSlot(
    const Floppy144RunState *state, uint32_t candidate_count
)
{
    if(state == NULL || candidate_count == 0U) return 0U;
    return Floppy144VariationRange(
        state->recovery_seed, "GREY_DOOR", "CORRIDOR_WALL_V1", candidate_count
    );
}

static bool GreyDoorIsCorridor(int32_t x, int32_t y)
{
    if(x < 0 || y < 0 ||
       x >= FLOPPY144_SITE_SIZE_UNITS || y >= FLOPPY144_SITE_SIZE_UNITS)
        return false;
    return Floppy144SiteRoomAtCell((uint8_t)x,(uint8_t)y) ==
        FLOPPY144_ROOM_CORRIDOR;
}

/* Revalidate every possible placement, including ones that will never be
   selected by sample seeds. A future plan update cannot silently place the
   door across a fixture, plaque, directory or existing doorway. */
bool Floppy144GreyDoorCandidateSafe(const Floppy144GreyDoorCandidate *candidate)
{
    const Floppy144SiteRect *r;
    int32_t x, y, w, h;
    int32_t i;
    uint32_t n;

    if(candidate == NULL) return false;
    r=&candidate->rect;
    x=r->x; y=r->y; w=r->width; h=r->height;
    if(r->room != (uint8_t)FLOPPY144_ROOM_CORRIDOR ||
       r->type != (uint8_t)FLOPPY144_SITE_DOOR ||
       (w != FLOPPY144_GREY_DOOR_LENGTH || h != 1) &&
       (h != FLOPPY144_GREY_DOOR_LENGTH || w != 1))
        return false;
    if(x < FLOPPY144_GREY_DOOR_HALO ||
       y < FLOPPY144_GREY_DOOR_HALO ||
       x+w+FLOPPY144_GREY_DOOR_HALO > FLOPPY144_SITE_SIZE_UNITS ||
       y+h+FLOPPY144_GREY_DOOR_HALO > FLOPPY144_SITE_SIZE_UNITS)
        return false;

    /* Four wall cells must be outside the Corridor floor and have a whole
       adjacent Corridor floor edge. Reject seams between two floor pieces. */
    for(i=0;i<FLOPPY144_GREY_DOOR_LENGTH;++i)
    {
        int32_t wall_x=x+(w>h?i:0);
        int32_t wall_y=y+(h>w?i:0);
        bool inside=false;
        if(GreyDoorIsCorridor(wall_x,wall_y)) return false;
        if(w>h)
            inside=GreyDoorIsCorridor(wall_x,wall_y-1) ||
                   GreyDoorIsCorridor(wall_x,wall_y+1);
        else
            inside=GreyDoorIsCorridor(wall_x-1,wall_y) ||
                   GreyDoorIsCorridor(wall_x+1,wall_y);
        if(!inside) return false;
    }

    /* Protect every authored non-floor rectangle, whether currently revealed
       or not. This includes doors, windows, fixtures, wall-hangings,
       directory panels, room plaques, notices and furniture on either side. */
    for(n=0U;n<Floppy144SiteRectCount();++n)
    {
        const Floppy144SiteRect *other=Floppy144SiteRectAt(n);
        if(other==NULL || other->type <= (uint8_t)FLOPPY144_SITE_FLOOR_D)
            continue;
        if((int32_t)other->x < x+w+FLOPPY144_GREY_DOOR_HALO &&
           (int32_t)other->x+(int32_t)other->width >
               x-FLOPPY144_GREY_DOOR_HALO &&
           (int32_t)other->y < y+h+FLOPPY144_GREY_DOOR_HALO &&
           (int32_t)other->y+(int32_t)other->height >
               y-FLOPPY144_GREY_DOOR_HALO)
            return false;
    }
    if(Floppy144SiteRoomAtPosition(
            candidate->stand_x16,candidate->stand_y16) !=
            FLOPPY144_ROOM_CORRIDOR ||
       Floppy144SitePositionBlocked(
            candidate->stand_x16,candidate->stand_y16))
        return false;
    return true;
}

static void GreyDoorGenerate(void)
{
    uint32_t n;
    if(grey_candidates_ready) return;
    grey_candidates_ready=true;
    grey_candidate_count=0U;
    for(n=0U;n<Floppy144SiteRectCount();++n)
    {
        const Floppy144SiteRect *floor=Floppy144SiteRectAt(n);
        int32_t side;
        if(floor == NULL ||
           floor->room != (uint8_t)FLOPPY144_ROOM_CORRIDOR ||
           floor->type > (uint8_t)FLOPPY144_SITE_FLOOR_D)
            continue;
        for(side=0;side<4;++side)
        {
            const bool horizontal=side<2;
            const int32_t span=horizontal?floor->width:floor->height;
            int32_t p;
            for(p=FLOPPY144_GREY_DOOR_HALO;
                p+FLOPPY144_GREY_DOOR_LENGTH+FLOPPY144_GREY_DOOR_HALO<=span;
                ++p)
            {
                Floppy144GreyDoorCandidate candidate;
                int32_t x=horizontal?floor->x+p:
                    (side==2?(int32_t)floor->x-1:
                       (int32_t)floor->x+floor->width);
                int32_t y=horizontal?
                    (side==0?(int32_t)floor->y-1:
                       (int32_t)floor->y+floor->height):
                    floor->y+p;
                int32_t w=horizontal?FLOPPY144_GREY_DOOR_LENGTH:1;
                int32_t h=horizontal?1:FLOPPY144_GREY_DOOR_LENGTH;
                memset(&candidate,0,sizeof(candidate));
                if(x<0 || y<0 || x+w>FLOPPY144_SITE_SIZE_UNITS ||
                   y+h>FLOPPY144_SITE_SIZE_UNITS) continue;
                candidate.rect.type=(uint8_t)FLOPPY144_SITE_DOOR;
                candidate.rect.room=(uint8_t)FLOPPY144_ROOM_CORRIDOR;
                candidate.rect.from_room=(uint8_t)FLOPPY144_ROOM_CORRIDOR;
                candidate.rect.to_room=(uint8_t)FLOPPY144_ROOM_CORRIDOR;
                candidate.rect.x=(uint8_t)x;
                candidate.rect.y=(uint8_t)y;
                candidate.rect.width=(uint8_t)w;
                candidate.rect.height=(uint8_t)h;
                candidate.rect.authored_width16=(uint16_t)(w*FLOPPY144_SITE_FIXED_ONE);
                candidate.rect.authored_height16=(uint16_t)(h*FLOPPY144_SITE_FIXED_ONE);
                candidate.stand_x16=(horizontal?(x+2):
                    (side==2?x+2:x-1))*FLOPPY144_SITE_FIXED_ONE;
                candidate.stand_y16=(horizontal?
                    (side==0?y+3:y):y+3)*FLOPPY144_SITE_FIXED_ONE;

                /* No duplicate slots if touching floor rectangles share an
                   edge and happen to describe the same external wall. */
                if(Floppy144GreyDoorCandidateSafe(&candidate))
                {
                    uint32_t k;
                    bool duplicate=false;
                    for(k=0U;k<grey_candidate_count;++k)
                    {
                        const Floppy144SiteRect *old=&grey_candidates[k].rect;
                        if(old->x==candidate.rect.x && old->y==candidate.rect.y &&
                           old->width==candidate.rect.width &&
                           old->height==candidate.rect.height)
                        {
                            duplicate=true;
                            break;
                        }
                    }
                    if(duplicate) continue;
                    if(grey_candidate_count==FLOPPY144_GREY_DOOR_MAX_CANDIDATES)
                    {
                        grey_candidates_overflow=true;
                        grey_candidate_count=0U;
                        return;
                    }
                    grey_candidates[grey_candidate_count++]=candidate;
                }
            }
        }
    }
}

uint32_t Floppy144GreyDoorCandidateCount(void)
{
    GreyDoorGenerate();
    return grey_candidates_overflow?0U:grey_candidate_count;
}

bool Floppy144GreyDoorCandidateAt(
    uint32_t index, Floppy144GreyDoorCandidate *out
)
{
    GreyDoorGenerate();
    if(out==NULL || grey_candidates_overflow ||
       index>=grey_candidate_count) return false;
    *out=grey_candidates[index];
    return true;
}

/* Hathaway-only wall directly LEFT of the Corridor Site Directory.
   Directory: (35,56) 6x1; forced panel: (29,56) 4x1. The overlay
   leaves the existing wall, collision map and Directory untouched. */
bool Floppy144GreyDoorHathawayCandidate(Floppy144GreyDoorCandidate *out)
{
    Floppy144GreyDoorCandidate test;
    uint32_t i;
    memset(&test,0,sizeof(test));
    test.rect.type=(uint8_t)FLOPPY144_SITE_DOOR;
    test.rect.room=(uint8_t)FLOPPY144_ROOM_CORRIDOR;
    test.rect.from_room=(uint8_t)FLOPPY144_ROOM_CORRIDOR;
    test.rect.to_room=(uint8_t)FLOPPY144_ROOM_CORRIDOR;
    test.rect.x=29U;
    test.rect.y=56U;
    test.rect.width=4U;
    test.rect.height=1U;
    test.rect.authored_width16=4U*FLOPPY144_SITE_FIXED_ONE;
    test.rect.authored_height16=FLOPPY144_SITE_FIXED_ONE;
    test.stand_x16=31*FLOPPY144_SITE_FIXED_ONE;
    test.stand_y16=55*FLOPPY144_SITE_FIXED_ONE;
    if(out==NULL ||
       Floppy144SiteRoomAtPosition(test.stand_x16,test.stand_y16) !=
           FLOPPY144_ROOM_CORRIDOR ||
       Floppy144SitePositionBlocked(test.stand_x16,test.stand_y16))
        return false;
    /* The logical room region includes the wall boundary: verify the
       actual generated walkable FLOOR, not the region's bounding box. */
    {
        bool interior_floor=false,wall_floor=false;
        for(i=0U;i<Floppy144SiteRectCount();++i)
        {
            const Floppy144SiteRect *f=Floppy144SiteRectAt(i);
            if(f==NULL || f->room!=(uint8_t)FLOPPY144_ROOM_CORRIDOR ||
               f->type>(uint8_t)FLOPPY144_SITE_FLOOR_D) continue;
            if(31>=(int32_t)f->x && 31<(int32_t)f->x+f->width)
            {
                if(55>=(int32_t)f->y && 55<(int32_t)f->y+f->height)
                    interior_floor=true;
                if(56>=(int32_t)f->y && 56<(int32_t)f->y+f->height)
                    wall_floor=true;
            }
        }
        if(!interior_floor || wall_floor) return false;
    }
    for(i=0U;i<Floppy144SiteRectCount();++i)
    {
        const Floppy144SiteRect *r=Floppy144SiteRectAt(i);
        if(r==NULL || r->type<=(uint8_t)FLOPPY144_SITE_FLOOR_D) continue;
        if((int32_t)r->x<33 && (int32_t)r->x+(int32_t)r->width>29 &&
           (int32_t)r->y<57 && (int32_t)r->y+(int32_t)r->height>56)
            return false;
    }
    *out=test;
    return true;
}

bool Floppy144GreyDoorForRun(
    const Floppy144RunState *state, Floppy144GreyDoorCandidate *out
)
{
    uint32_t count=Floppy144GreyDoorCandidateCount();
    if(state != NULL && state->hathaway_inspection != 0U &&
       Floppy144RunStateRoomReconstructed(state,FLOPPY144_ROOM_CORRIDOR))
        return Floppy144GreyDoorHathawayCandidate(out);
    if(state==NULL || out==NULL ||
       state->grey_door_state != (uint8_t)FLOPPY144_GREY_DOOR_AVAILABLE ||
       count==0U ||
       !Floppy144RunStateRoomReconstructed(state,FLOPPY144_ROOM_CORRIDOR))
        return false;
    return Floppy144GreyDoorCandidateAt(
        Floppy144RunStateGreyDoorPlacementSlot(state,count),out
    );
}

/* Same 0.5U collision-footprint-to-target proximity used by normal Site
   Inspect/Access targeting. Never permits access from the wall's far side. */
bool Floppy144GreyDoorNearby(const Floppy144RunState *state)
{
    Floppy144GreyDoorCandidate candidate;
    int32_t px0,px1,py0,py1,x0,x1,y0,y1,dx=0,dy=0;
    const Floppy144SiteRect *r;
    if(!Floppy144GreyDoorForRun(state,&candidate) ||
       Floppy144SiteRoomAtPosition(
           state->player_site_x,state->player_site_y) !=
           FLOPPY144_ROOM_CORRIDOR)
        return false;
    r=&candidate.rect;
    px0=state->player_site_x-FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16/2;
    px1=state->player_site_x+FLOPPY144_SITE_PLAYER_COLLISION_WIDTH_X16/2;
    py0=state->player_site_y-FLOPPY144_SITE_PLAYER_COLLISION_DEPTH_X16;
    py1=state->player_site_y;
    x0=r->x*FLOPPY144_SITE_FIXED_ONE;
    x1=(r->x+r->width)*FLOPPY144_SITE_FIXED_ONE;
    y0=r->y*FLOPPY144_SITE_FIXED_ONE;
    y1=(r->y+r->height)*FLOPPY144_SITE_FIXED_ONE;
    if(px1<x0) dx=x0-px1;
    else if(px0>x1) dx=px0-x1;
    if(py1<y0) dy=y0-py1;
    else if(py0>y1) dy=py0-y1;
    return dx*dx+dy*dy <=
        (FLOPPY144_SITE_FIXED_ONE/2)*(FLOPPY144_SITE_FIXED_ONE/2);
}
