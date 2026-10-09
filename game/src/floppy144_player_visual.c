/*
 * FLOPPY//144 - lightweight directional player presentation.
 */

#include "floppy144_player_visual.h"

#include <stddef.h>
#include <math.h>

static int32_t Floppy144PlayerAbs(
    int32_t value
)
{
    return value < 0 ? -value : value;
}

static Floppy144PlayerFacing Floppy144PlayerFacingFromAction(
    F144Action action
)
{
    switch(action)
    {
        case F144_ACTION_MOVE_LEFT:
            return FLOPPY144_PLAYER_FACING_LEFT;

        case F144_ACTION_MOVE_RIGHT:
            return FLOPPY144_PLAYER_FACING_RIGHT;

        case F144_ACTION_MOVE_UP:
            return FLOPPY144_PLAYER_FACING_UP;

        case F144_ACTION_MOVE_DOWN:
            return FLOPPY144_PLAYER_FACING_DOWN;

        default:
            return FLOPPY144_PLAYER_FACING_COUNT;
    }
}

static bool Floppy144PlayerFacingMatchesVector(
    Floppy144PlayerFacing facing,
    int32_t x,
    int32_t y
)
{
    switch(facing)
    {
        case FLOPPY144_PLAYER_FACING_LEFT:
            return x < 0;

        case FLOPPY144_PLAYER_FACING_RIGHT:
            return x > 0;

        case FLOPPY144_PLAYER_FACING_UP:
            return y < 0;

        case FLOPPY144_PLAYER_FACING_DOWN:
            return y > 0;

        default:
            return false;
    }
}

void Floppy144PlayerVisualReset(
    Floppy144PlayerVisualState *state
)
{
    if(state == NULL)
    {
        return;
    }

    state->animation_accumulator_ms = 0U;
    state->facing = (uint8_t)FLOPPY144_PLAYER_FACING_DOWN;
    state->moving = 0U;
    state->walk_frame = 0U;
}

bool Floppy144PlayerVisualSetMovement(
    Floppy144PlayerVisualState *state,
    int32_t movement_x,
    int32_t movement_y,
    F144Action recent_action
)
{
    Floppy144PlayerFacing old_facing;
    Floppy144PlayerFacing facing;
    Floppy144PlayerFacing preferred;
    bool old_moving;
    bool moving;

    if(state == NULL)
    {
        return false;
    }

    old_facing =
        state->facing < (uint8_t)FLOPPY144_PLAYER_FACING_COUNT
            ? (Floppy144PlayerFacing)state->facing
            : FLOPPY144_PLAYER_FACING_DOWN;

    old_moving =
        state->moving != 0U;

    moving =
        movement_x != 0 ||
        movement_y != 0;

    if(!moving)
    {
        bool changed =
            old_moving ||
            state->walk_frame != 0U ||
            state->animation_accumulator_ms != 0U;

        state->facing =
            (uint8_t)old_facing;
        state->moving = 0U;
        state->walk_frame = 0U;
        state->animation_accumulator_ms = 0U;

        return changed;
    }

    preferred =
        Floppy144PlayerFacingFromAction(
            recent_action
        );

    if(
        preferred < FLOPPY144_PLAYER_FACING_COUNT &&
        Floppy144PlayerFacingMatchesVector(
            preferred,
            movement_x,
            movement_y
        )
    )
    {
        facing =
            preferred;
    }
    else if(
        Floppy144PlayerFacingMatchesVector(
            old_facing,
            movement_x,
            movement_y
        )
    )
    {
        facing =
            old_facing;
    }
    else if(movement_x < 0)
    {
        facing =
            FLOPPY144_PLAYER_FACING_LEFT;
    }
    else if(movement_x > 0)
    {
        facing =
            FLOPPY144_PLAYER_FACING_RIGHT;
    }
    else if(movement_y < 0)
    {
        facing =
            FLOPPY144_PLAYER_FACING_UP;
    }
    else
    {
        facing =
            FLOPPY144_PLAYER_FACING_DOWN;
    }

    if(
        !old_moving ||
        facing != old_facing
    )
    {
        state->walk_frame = 0U;
        state->animation_accumulator_ms = 0U;
    }

    state->facing =
        (uint8_t)facing;
    state->moving = 1U;

    return
        !old_moving ||
        facing != old_facing;
}

bool Floppy144PlayerVisualAdvance(
    Floppy144PlayerVisualState *state,
    uint32_t elapsed_ms
)
{
    uint64_t accumulated;
    uint32_t frames;

    if(
        state == NULL ||
        state->moving == 0U ||
        elapsed_ms == 0U
    )
    {
        return false;
    }

    accumulated =
        (uint64_t)state->animation_accumulator_ms +
        (uint64_t)elapsed_ms;

    frames =
        (uint32_t)(
            accumulated /
            FLOPPY144_PLAYER_WALK_FRAME_MS
        );

    state->animation_accumulator_ms =
        (uint32_t)(
            accumulated %
            FLOPPY144_PLAYER_WALK_FRAME_MS
        );

    if(frames == 0U)
    {
        return false;
    }

    state->walk_frame = (uint8_t)((state->walk_frame + frames * 8U) % 64U);
    return true;
}


/*
 * Fixed-size, painter-ordered vector display list.
 * All dimensions below are local to the sprite and use float until rasterised.
 * Coordinates are clamped by the target surface AND the caller clip rectangle.
 */
#define F144_CHARACTER_MAX_PRIMITIVES 96U
#define F144_CHARACTER_TAU 6.2831853071795864769f

typedef struct { float x, y; } F144Vec2;
typedef struct { uint8_t r, g, b, a; } F144RGBA;
typedef struct { F144Vec2 a, b; float width; } F144Segment;
typedef struct { F144Vec2 centre; float radius; } F144Circle;
typedef struct { F144Segment axis; } F144Capsule;
typedef enum { F144_VEC_LINE, F144_VEC_CIRCLE, F144_VEC_CAPSULE } F144PrimitiveType;
typedef struct {
    F144PrimitiveType type;
    F144RGBA colour;
    union { F144Segment line; F144Circle circle; F144Capsule capsule; } shape;
} F144Primitive;
typedef struct {
    F144Primitive primitives[F144_CHARACTER_MAX_PRIMITIVES];
    unsigned count;
} RenderBuffer;

typedef struct {
    Floppy144Surface *surface;
    int32_t x0, y0, x1, y1;
} F144Raster;

static F144Vec2 f144Vec(float x, float y) { F144Vec2 p = { x, y }; return p; }
static F144RGBA f144Colour(uint8_t r, uint8_t g, uint8_t b) {
    F144RGBA c = { r, g, b, 255 }; return c;
}
static int32_t f144Round(float x) { return (int32_t)(x + (x >= 0 ? .5f : -.5f)); }
static float f144Positive(float x) { return x > 0.f ? x : 0.f; }
static void f144AddLine(RenderBuffer *b, F144Vec2 a, F144Vec2 z, float width, F144RGBA c, F144PrimitiveType t) {
    F144Primitive *p;
    if(b->count >= F144_CHARACTER_MAX_PRIMITIVES) return;
    p = &b->primitives[b->count++];
    p->type = t; p->colour = c;
    p->shape.line.a = a; p->shape.line.b = z; p->shape.line.width = width;
}
static void f144AddCircle(RenderBuffer *b, F144Vec2 centre, float radius, F144RGBA c) {
    F144Primitive *p;
    if(b->count >= F144_CHARACTER_MAX_PRIMITIVES || radius <= 0.f) return;
    p = &b->primitives[b->count++];
    p->type = F144_VEC_CIRCLE; p->colour = c;
    p->shape.circle.centre = centre; p->shape.circle.radius = radius;
}
static void f144CapsuleAdd(RenderBuffer *b, float x0, float y0, float x1, float y1, float width, F144RGBA c) {
    f144AddLine(b, f144Vec(x0,y0), f144Vec(x1,y1), width, c, F144_VEC_CAPSULE);
}
static void f144Pixel(const F144Raster *r, int32_t x, int32_t y, F144RGBA c) {
    if(x >= r->x0 && x < r->x1 && y >= r->y0 && y < r->y1) {
        Floppy144DrawFillRect(r->surface, (uint32_t)x, (uint32_t)y, 1U, 1U,
            FLOPPY144_RGB(c.r,c.g,c.b));
    }
}
static void f144Disk(const F144Raster *r, int32_t cx, int32_t cy, int32_t radius, F144RGBA c) {
    int32_t x, y;
    if(radius < 0) return;
    /* Bound iterations even for malformed scale or off-screen geometry. */
    if(radius > 256) radius = 256;
    for(y = -radius; y <= radius; ++y) {
        if(cy+y < r->y0 || cy+y >= r->y1) continue;
        for(x = -radius; x <= radius; ++x)
            if(x*x+y*y <= radius*radius) f144Pixel(r,cx+x,cy+y,c);
    }
}
/* Integer Bresenham centreline, with a circular pen for configurable thickness. */
static void f144Stroke(const F144Raster *r, F144Segment s, F144RGBA c) {
    int32_t x0=f144Round(s.a.x), y0=f144Round(s.a.y);
    int32_t x1=f144Round(s.b.x), y1=f144Round(s.b.y);
    int32_t dx=Floppy144PlayerAbs(x1-x0), dy=-Floppy144PlayerAbs(y1-y0);
    int32_t sx=x0<x1?1:-1, sy=y0<y1?1:-1, err=dx+dy;
    int32_t radius=f144Round(s.width*.5f);
    int32_t steps=0;
    /* Reject absurd coordinates rather than allowing an unbounded loop. */
    if(dx > 8192 || -dy > 8192 || radius > 256) return;
    for(;;) {
        f144Disk(r,x0,y0,radius,c);
        if(x0==x1 && y0==y1) break;
        if(++steps > 8193) break;
        {
            int32_t e2=2*err;
            if(e2>=dy) {err+=dy; x0+=sx;}
            if(e2<=dx) {err+=dx; y0+=sy;}
        }
    }
}
static void f144Rasterize(const RenderBuffer *b, const F144Raster *r) {
    unsigned i;
    for(i=0;i<b->count;++i) {
        const F144Primitive *p=&b->primitives[i];
        if(p->type==F144_VEC_CIRCLE)
            f144Disk(r,f144Round(p->shape.circle.centre.x),
                f144Round(p->shape.circle.centre.y),
                f144Round(p->shape.circle.radius),p->colour);
        else f144Stroke(r,p->shape.line,p->colour);
    }
}
/*
 * Cosmetic flags are intentionally independent of saved body-style identity.
 * A game UI may later expose them without touching movement/collision state.
 */
enum {
    F144_COSTUME_JACKET=1U, F144_COSTUME_TIE=2U,
    F144_COSTUME_SKIRT=4U, F144_COSTUME_LONG_HAIR=8U,
    F144_COSTUME_SHORT_HAIR=16U
};
typedef struct {
    uint32_t flags;
    F144RGBA skin, hair, cloth, shirt, trousers, shoes;
} F144Costume;
static F144Costume f144OfficeCostume(Floppy144OperatorBodyStyle style) {
    F144Costume c;
    c.flags = style==FLOPPY144_OPERATOR_BODY_STYLE_B
        ? (F144_COSTUME_JACKET|F144_COSTUME_LONG_HAIR)
        : (F144_COSTUME_JACKET|F144_COSTUME_TIE|F144_COSTUME_SHORT_HAIR);
    c.skin=f144Colour(205,186,158); c.hair=f144Colour(49,39,34);
    c.cloth=f144Colour(72,103,118); c.shirt=f144Colour(204,214,213);
    c.trousers=f144Colour(39,48,54); c.shoes=f144Colour(21,26,31);
    return c;
}
static void f144DrawLeg(RenderBuffer *b, float hipx, float hipy,
                        float stride, float lift, float scale, const F144Costume *c) {
    float kneeX=hipx+stride*.45f, footX=hipx+stride;
    float kneeY=hipy+scale*12.f-lift*.4f, footY=hipy+scale*25.f-lift;
    f144CapsuleAdd(b,hipx,hipy,kneeX,kneeY,scale*4.8f,c->trousers);
    f144CapsuleAdd(b,kneeX,kneeY,footX,footY,scale*4.4f,c->trousers);
    f144CapsuleAdd(b,footX-scale*2.f,footY,footX+scale*2.5f,footY,scale*3.f,c->shoes);
}
static void f144DrawArm(RenderBuffer *b, float sx, float sy, float swing,
                        float scale, const F144Costume *c) {
    float elbowX=sx+swing*.55f, elbowY=sy+scale*9.f;
    float handX=sx+swing, handY=sy+scale*18.f-f144Positive(-swing)*.12f;
    f144CapsuleAdd(b,sx,sy,elbowX,elbowY,scale*4.5f,c->cloth);
    f144CapsuleAdd(b,elbowX,elbowY,handX,handY-scale*2.f,scale*3.5f,c->cloth);
    f144AddCircle(b,f144Vec(handX,handY),scale*2.f,c->skin);
}
static void f144BuildOfficeCharacter(RenderBuffer *b, float cx, float footY,
                                      float width, float height,
                                      Floppy144PlayerFacing facing,
                                      float phase, int walking,
                                      const F144Costume *c) {
    float scale=height/68.f, stride=0.f, bob=0.f, sway=0.f;
    float top=footY-height, shoulders, hip, headY, shoulderWidth;
    int profile=facing==FLOPPY144_PLAYER_FACING_LEFT ||
                facing==FLOPPY144_PLAYER_FACING_RIGHT;
    int front=facing==FLOPPY144_PLAYER_FACING_DOWN;
    int back=facing==FLOPPY144_PLAYER_FACING_UP;
    float direction=facing==FLOPPY144_PLAYER_FACING_LEFT?-1.f:1.f;
    float liftA=0.f,liftB=0.f, hand=0.f, hipX;
    (void)width;
    if(walking) {
        stride=sinf(phase)*scale*6.f;
        hand=-stride*1.3f; /* contralateral arm movement */
        bob=(1.f-cosf(phase*2.f))*scale*.9f;
        sway=sinf(phase)*scale*.8f;
        liftA=f144Positive(sinf(phase))*scale*4.f;
        liftB=f144Positive(-sinf(phase))*scale*4.f;
    }
    top+=bob;
    shoulders=top+scale*27.f; hip=top+scale*43.f;
    headY=top+scale*11.f; hipX=cx+sway;
    shoulderWidth=(c->flags&F144_COSTUME_JACKET)?scale*8.f:scale*7.f;
    if(profile) {
        /* Far limbs first, then body, then near limbs: stable painter order. */
        f144DrawLeg(b,hipX-direction*scale,hip,-stride,liftB,scale,c);
        f144DrawArm(b,cx-direction*scale*2.f,shoulders,-hand,scale,c);
        f144CapsuleAdd(b,cx,shoulders,cx+sway*.3f,hip,scale*10.f,c->cloth);
        f144CapsuleAdd(b,cx,hip,cx,hip+scale*2.f,scale*8.f,c->cloth);
        if(c->flags&F144_COSTUME_SKIRT)
            f144CapsuleAdd(b,cx,hip+scale*2.f,cx,hip+scale*8.f,scale*13.f,c->cloth);
        f144DrawLeg(b,hipX+direction*scale,hip,stride,liftA,scale,c);
        f144DrawArm(b,cx+direction*scale*2.f,shoulders,hand,scale,c);
    } else {
        f144DrawLeg(b,hipX-scale*3.f,hip,stride*.42f,liftA,scale,c);
        f144DrawLeg(b,hipX+scale*3.f,hip,-stride*.42f,liftB,scale,c);
        f144DrawArm(b,cx-shoulderWidth,shoulders,-scale*1.5f+hand*.22f,scale,c);
        f144DrawArm(b,cx+shoulderWidth,shoulders,scale*1.5f-hand*.22f,scale,c);
        f144CapsuleAdd(b,cx,shoulders,cx+sway*.3f,hip,scale*14.f,c->cloth);
        if(c->flags&F144_COSTUME_SKIRT)
            f144CapsuleAdd(b,cx,hip,cx,hip+scale*6.f,scale*15.f,c->cloth);
        if(front) {
            f144CapsuleAdd(b,cx,shoulders+scale*2.f,cx,hip-scale*3.f,scale*4.f,c->shirt);
            if(c->flags&F144_COSTUME_TIE)
                f144CapsuleAdd(b,cx,shoulders+scale*5.f,cx,shoulders+scale*13.f,scale*1.6f,c->hair);
        }
    }
    if(c->flags&F144_COSTUME_LONG_HAIR)
        f144CapsuleAdd(b,cx,headY,cx,headY+scale*11.f,scale*11.f,c->hair);
    f144AddCircle(b,f144Vec(cx,headY),scale*8.f,c->skin);
    if(back) {
        f144CapsuleAdd(b,cx,headY-scale*3.f,cx,headY+scale*3.f,scale*14.f,c->hair);
    } else {
        f144CapsuleAdd(b,cx,headY-scale*6.f,cx,headY-scale*7.f,scale*10.f,c->hair);
        if(profile) {
            f144AddCircle(b,f144Vec(cx+direction*scale*3.5f,headY-scale),scale*.85f,c->shoes);
            f144AddCircle(b,f144Vec(cx+direction*scale*7.5f,headY+scale*2.f),scale*1.45f,c->skin);
        } else {
            f144AddCircle(b,f144Vec(cx-scale*3.f,headY),scale*.85f,c->shoes);
            f144AddCircle(b,f144Vec(cx+scale*3.f,headY),scale*.85f,c->shoes);
        }
    }
}
void Floppy144PlayerVisualDraw(
    Floppy144Surface *surface, int32_t foot_x, int32_t foot_y,
    int32_t sprite_width, int32_t sprite_height,
    int32_t collision_shadow_width, Floppy144OperatorBodyStyle body_style,
    const Floppy144PlayerVisualState *state,
    int32_t clip_x, int32_t clip_y, int32_t clip_width, int32_t clip_height
) {
    RenderBuffer b={ { {0} },0U };
    F144Raster r;
    F144Costume costume;
    Floppy144PlayerFacing facing=FLOPPY144_PLAYER_FACING_DOWN;
    float phase=0.f;
    int walking=0;
    if(!surface || !surface->pixels || sprite_width<=0 || sprite_height<=0 ||
       clip_width<=0 || clip_height<=0 || collision_shadow_width<=0) return;
    r.surface=surface;
    r.x0=clip_x>0?clip_x:0; r.y0=clip_y>0?clip_y:0;
    r.x1=clip_x+clip_width<(int32_t)surface->width?clip_x+clip_width:(int32_t)surface->width;
    r.y1=clip_y+clip_height<(int32_t)surface->height?clip_y+clip_height:(int32_t)surface->height;
    if(r.x1<=r.x0 || r.y1<=r.y0) return;
    if(state && state->facing<FLOPPY144_PLAYER_FACING_COUNT) {
        facing=(Floppy144PlayerFacing)state->facing;
        walking=state->moving!=0U;
        if(walking)
            phase=F144_CHARACTER_TAU*
                ((float)state->walk_frame+
                 (float)state->animation_accumulator_ms/(float)FLOPPY144_PLAYER_WALK_FRAME_MS)/64.f;
    }
    if(body_style<0 || body_style>=FLOPPY144_OPERATOR_BODY_STYLE_COUNT)
        body_style=FLOPPY144_OPERATOR_BODY_STYLE_DEFAULT;
    costume=f144OfficeCostume(body_style);
    f144CapsuleAdd(&b,(float)foot_x-(float)collision_shadow_width*.45f,
                  (float)foot_y+1.f,(float)foot_x+(float)collision_shadow_width*.45f,
                  (float)foot_y+1.f,3.f,f144Colour(31,35,34));
    f144BuildOfficeCharacter(&b,(float)foot_x,(float)foot_y,
                            (float)sprite_width,(float)sprite_height,
                            facing,phase,walking,&costume);
    f144Rasterize(&b,&r);
}
