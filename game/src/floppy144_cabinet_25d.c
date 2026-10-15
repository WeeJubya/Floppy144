/*
 * FLOPPY//144 - compact pseudo-isometric Inspection parent renderer.
 *
 * The renderer deliberately avoids external assets. Convex faces are filled
 * with a tiny scanline routine and class identity is layered onto a shared
 * isometric box vocabulary.
 */

#include "floppy144_cabinet_25d.h"
#include "floppy144_double_door.h"

#include <stddef.h>
#include <string.h>

#define F144_25D_RGB(r,g,b) FLOPPY144_RGB((r),(g),(b))

#define F144_25D_PANEL_RIGHT 306U

static void Floppy144Cabinet25DFillRect(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t colour
)
{
    uint32_t right;

    if(
        surface == NULL ||
        surface->pixels == NULL ||
        width == 0U ||
        height == 0U ||
        x >= F144_25D_PANEL_RIGHT
    )
    {
        return;
    }

    right=x+width;

    if(
        right < x ||
        right > F144_25D_PANEL_RIGHT
    )
    {
        right=F144_25D_PANEL_RIGHT;
    }

    Floppy144DrawFillRect(
        surface,
        x,
        y,
        right-x,
        height,
        colour
    );
}

static void Floppy144Cabinet25DRect(
    Floppy144Surface *surface,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t colour
)
{
    if(width==0U||height==0U)
    {
        return;
    }

    Floppy144Cabinet25DFillRect(surface,x,y,width,1U,colour);

    if(height>1U)
    {
        Floppy144Cabinet25DFillRect(
            surface,
            x,
            y+height-1U,
            width,
            1U,
            colour
        );
    }

    if(height>2U)
    {
        Floppy144Cabinet25DFillRect(
            surface,
            x,
            y+1U,
            1U,
            height-2U,
            colour
        );

        if(width>1U)
        {
            Floppy144Cabinet25DFillRect(
                surface,
                x+width-1U,
                y+1U,
                1U,
                height-2U,
                colour
            );
        }
    }
}

typedef struct Floppy144Cabinet25DPoint
{
    int32_t x;
    int32_t y;
}
Floppy144Cabinet25DPoint;

typedef struct Floppy144Cabinet25DBox
{
    Floppy144Cabinet25DPoint a;
    Floppy144Cabinet25DPoint b;
    Floppy144Cabinet25DPoint c;
    Floppy144Cabinet25DPoint d;
    int32_t height;
}
Floppy144Cabinet25DBox;

static bool Floppy144Cabinet25DStringEqual(
    const char *a,
    const char *b
)
{
    return a != NULL && b != NULL && strcmp(a,b) == 0;
}

static int32_t Floppy144Cabinet25DAbs(
    int32_t value
)
{
    return value < 0 ? -value : value;
}

static void Floppy144Cabinet25DPixel(
    Floppy144Surface *surface,
    int32_t x,
    int32_t y,
    uint32_t colour
)
{
    if(
        surface == NULL ||
        surface->pixels == NULL ||
        x < 0 ||
        y < 0 ||
        (uint32_t)x >= F144_25D_PANEL_RIGHT ||
        (uint32_t)x >= surface->width ||
        (uint32_t)y >= surface->height
    )
    {
        return;
    }

    surface->pixels[
        (uint32_t)y * surface->width +
        (uint32_t)x
    ] = colour;
}

static void Floppy144Cabinet25DLine(
    Floppy144Surface *surface,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    uint32_t colour
)
{
    int32_t dx = Floppy144Cabinet25DAbs(x1-x0);
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t dy = -Floppy144Cabinet25DAbs(y1-y0);
    int32_t sy = y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;

    for(;;)
    {
        Floppy144Cabinet25DPixel(surface,x0,y0,colour);

        if(x0 == x1 && y0 == y1)
        {
            break;
        }

        {
            int32_t twice = error * 2;

            if(twice >= dy)
            {
                error += dy;
                x0 += sx;
            }

            if(twice <= dx)
            {
                error += dx;
                y0 += sy;
            }
        }
    }
}

static void Floppy144Cabinet25DFillQuad(
    Floppy144Surface *surface,
    Floppy144Cabinet25DPoint p0,
    Floppy144Cabinet25DPoint p1,
    Floppy144Cabinet25DPoint p2,
    Floppy144Cabinet25DPoint p3,
    uint32_t colour
)
{
    Floppy144Cabinet25DPoint points[4]={p0,p1,p2,p3};
    int32_t min_y=p0.y;
    int32_t max_y=p0.y;
    int32_t y;

    if(surface == NULL || surface->pixels == NULL)
    {
        return;
    }

    {
        uint32_t i;

        for(i=1U;i<4U;++i)
        {
            if(points[i].y < min_y) min_y=points[i].y;
            if(points[i].y > max_y) max_y=points[i].y;
        }
    }

    for(y=min_y;y<=max_y;++y)
    {
        int32_t min_x=0;
        int32_t max_x=0;
        uint32_t intersections=0U;
        uint32_t edge;

        for(edge=0U;edge<4U;++edge)
        {
            Floppy144Cabinet25DPoint a=points[edge];
            Floppy144Cabinet25DPoint b=points[(edge+1U)%4U];
            int32_t low_y;
            int32_t high_y;
            int32_t x;

            if(a.y == b.y)
            {
                continue;
            }

            low_y = a.y < b.y ? a.y : b.y;
            high_y = a.y < b.y ? b.y : a.y;

            if(y < low_y || y >= high_y)
            {
                continue;
            }

            x =
                a.x +
                (int32_t)(
                    (
                        (int64_t)(y-a.y) *
                        (int64_t)(b.x-a.x)
                    ) /
                    (int64_t)(b.y-a.y)
                );

            if(intersections == 0U)
            {
                min_x=max_x=x;
            }
            else
            {
                if(x<min_x)min_x=x;
                if(x>max_x)max_x=x;
            }

            ++intersections;
        }

        if(intersections >= 2U && max_x >= min_x)
        {
            if(min_x < 0) min_x=0;
            if(max_x >= (int32_t)surface->width)
                max_x=(int32_t)surface->width-1;

            if(
                y >= 0 &&
                y < (int32_t)surface->height &&
                max_x >= min_x
            )
            {
                Floppy144Cabinet25DFillRect(
                    surface,
                    (uint32_t)min_x,
                    (uint32_t)y,
                    (uint32_t)(max_x-min_x+1),
                    1U,
                    colour
                );
            }
        }
    }
}

static void Floppy144Cabinet25DOutlineQuad(
    Floppy144Surface *surface,
    Floppy144Cabinet25DPoint p0,
    Floppy144Cabinet25DPoint p1,
    Floppy144Cabinet25DPoint p2,
    Floppy144Cabinet25DPoint p3,
    uint32_t colour
)
{
    Floppy144Cabinet25DLine(surface,p0.x,p0.y,p1.x,p1.y,colour);
    Floppy144Cabinet25DLine(surface,p1.x,p1.y,p2.x,p2.y,colour);
    Floppy144Cabinet25DLine(surface,p2.x,p2.y,p3.x,p3.y,colour);
    Floppy144Cabinet25DLine(surface,p3.x,p3.y,p0.x,p0.y,colour);
}

static uint32_t Floppy144Cabinet25DNormalisedRotation(
    const Floppy144SiteRect *rect
)
{
    int32_t rotation;

    if(rect == NULL)
    {
        return 0U;
    }

    rotation =
        (int32_t)rect->rotation % 360;

    if(rotation < 0)
    {
        rotation += 360;
    }

    return (uint32_t)rotation;
}

static void Floppy144Cabinet25DDimensions(
    const Floppy144DataRecord *parent,
    const Floppy144SiteRect *rect,
    int32_t *width_units,
    int32_t *depth_units,
    bool *mirror,
    uint32_t *octant
)
{
    int32_t width=4;
    int32_t depth=3;
    uint32_t rotation=Floppy144Cabinet25DNormalisedRotation(rect);
    uint32_t direction=((rotation+22U)/45U)%8U;

    if(parent != NULL)
    {
        if(parent->n2 > 0) width=parent->n2;
        if(parent->n3 > 0) depth=parent->n3;
    }

    /*
     * Preserve the authored aspect ratio. Diagonal orientations use the Site
     * rectangle's signed rotation to decide which authored axis leads in the
     * pseudo-isometric view. Cardinal layouts still differ naturally through
     * their width/depth values.
     */
    if(
        direction == 1U ||
        direction == 2U ||
        direction == 5U ||
        direction == 6U
    )
    {
        int32_t swap=width;
        width=depth;
        depth=swap;
    }

    if(width < 1) width=1;
    if(depth < 1) depth=1;

    *width_units=width;
    *depth_units=depth;
    *mirror=
        direction == 2U ||
        direction == 3U ||
        direction == 4U ||
        direction == 5U;
    *octant=direction;
}

static Floppy144Cabinet25DBox Floppy144Cabinet25DMakeBox(
    int32_t width_units,
    int32_t depth_units,
    int32_t vertical_height,
    bool mirror
)
{
    Floppy144Cabinet25DBox box;
    int32_t total=width_units+depth_units;
    int32_t step;
    int32_t centre_x=166;
    int32_t top_y=88;
    int32_t direction=mirror?-1:1;

    if(total < 1) total=1;

    step=210/total;

    if(step<8)step=8;
    if(step>24)step=24;

    /*
     * BF10/BF30: constrain the common world-width projection to its actual
     * 2.5D panel. A long paired threshold must not bleed beyond the furniture
     * frame just because its long axis happens to point towards the gutter.
     * This is shared geometry, not per-object corrective offsets.
     */
    {
        int32_t right_units=mirror?depth_units:width_units;
        int32_t left_units=mirror?width_units:depth_units;
        int32_t left_available=centre_x-20;
        int32_t right_available=(int32_t)F144_25D_PANEL_RIGHT-12-centre_x;
        if(right_units>0 && step>right_available/right_units)
            step=right_available/right_units;
        if(left_units>0 && step>left_available/left_units)
            step=left_available/left_units;
        if(step<1)step=1;
    }

    box.a.x=centre_x;
    box.a.y=top_y;

    box.b.x=centre_x+direction*width_units*step;
    box.b.y=top_y+width_units*step/2;

    box.d.x=centre_x-direction*depth_units*step;
    box.d.y=top_y+depth_units*step/2;

    box.c.x=box.b.x+box.d.x-box.a.x;
    box.c.y=box.b.y+box.d.y-box.a.y;
    box.height=vertical_height;

    return box;
}

static void Floppy144Cabinet25DDrawBox(
    Floppy144Surface *surface,
    const Floppy144Cabinet25DBox *box,
    uint32_t top_colour,
    uint32_t front_colour,
    uint32_t side_colour,
    uint32_t edge_colour
)
{
    Floppy144Cabinet25DPoint c0,c1,d0,d1;

    if(surface==NULL||box==NULL)return;

    c0=box->c;
    c1=box->c;
    d0=box->d;
    d1=box->d;

    c1.y+=box->height;
    d1.y+=box->height;

    Floppy144Cabinet25DFillQuad(
        surface,
        box->d,
        box->c,
        c1,
        d1,
        front_colour
    );

    Floppy144Cabinet25DFillQuad(
        surface,
        box->b,
        box->c,
        c1,
        (Floppy144Cabinet25DPoint){box->b.x,box->b.y+box->height},
        side_colour
    );

    Floppy144Cabinet25DFillQuad(
        surface,
        box->a,
        box->b,
        box->c,
        box->d,
        top_colour
    );

    Floppy144Cabinet25DOutlineQuad(
        surface,
        box->a,
        box->b,
        box->c,
        box->d,
        edge_colour
    );

    Floppy144Cabinet25DLine(
        surface,
        box->b.x,
        box->b.y,
        box->b.x,
        box->b.y+box->height,
        edge_colour
    );

    Floppy144Cabinet25DLine(
        surface,
        box->c.x,
        box->c.y,
        box->c.x,
        box->c.y+box->height,
        edge_colour
    );

    Floppy144Cabinet25DLine(
        surface,
        box->d.x,
        box->d.y,
        box->d.x,
        box->d.y+box->height,
        edge_colour
    );

    Floppy144Cabinet25DLine(
        surface,
        d1.x,d1.y,
        c1.x,c1.y,
        edge_colour
    );

    (void)c0;
    (void)d0;
}

static int32_t Floppy144Cabinet25DMinX(
    const Floppy144Cabinet25DBox *box
)
{
    int32_t value=box->a.x;

    if(box->b.x<value)value=box->b.x;
    if(box->c.x<value)value=box->c.x;
    if(box->d.x<value)value=box->d.x;

    return value;
}

static int32_t Floppy144Cabinet25DMaxX(
    const Floppy144Cabinet25DBox *box
)
{
    int32_t value=box->a.x;

    if(box->b.x>value)value=box->b.x;
    if(box->c.x>value)value=box->c.x;
    if(box->d.x>value)value=box->d.x;

    return value;
}

static int32_t Floppy144Cabinet25DBottomY(
    const Floppy144Cabinet25DBox *box
)
{
    int32_t value=box->a.y;

    if(box->b.y>value)value=box->b.y;
    if(box->c.y>value)value=box->c.y;
    if(box->d.y>value)value=box->d.y;

    return value+box->height;
}

/*
 * Projection-neutral child placements. Physical items own a parent ID in the
 * canonical data, but no screen coordinates; never interpret drawing_definition
 * (an item illustration class) as a surface location.
 *
 * Each parent family selects a real face of the same 2.5D box used to render
 * that furniture. Child corners are bilinearly interpolated on that quad, so
 * orientation, dimensions and perspective are inherited without screen-space
 * rectangles. A one-pixel lift avoids fighting the parent-face outline.
 */
typedef enum Floppy144Cabinet25DChildFace
{
    F144_25D_CHILD_TOP,
    F144_25D_CHILD_FRONT,
    F144_25D_CHILD_SIDE,
    F144_25D_CHILD_WALL_PANEL,
    F144_25D_CHILD_SHELF
} Floppy144Cabinet25DChildFace;

static Floppy144Cabinet25DChildFace Floppy144Cabinet25DChildSurface(
    const Floppy144DataRecord *parent
)
{
    const char *v=parent!=NULL?parent->pszC:NULL;
    if(parent==NULL)return F144_25D_CHILD_TOP;
    if(Floppy144Cabinet25DStringEqual(parent->pszB,"WALL_MOUNTED_ITEM") ||
       Floppy144Cabinet25DStringEqual(v,"PARTITION_WALL") ||
       Floppy144Cabinet25DStringEqual(v,"WINDOW"))
        return F144_25D_CHILD_WALL_PANEL;
    if(Floppy144Cabinet25DStringEqual(v,"BOOKCASE") ||
       Floppy144Cabinet25DStringEqual(v,"SHELVING") ||
       Floppy144Cabinet25DStringEqual(v,"SHELVING_FULL"))
        return F144_25D_CHILD_SHELF;
    if(Floppy144Cabinet25DStringEqual(v,"DOOR") ||
       Floppy144Cabinet25DStringEqual(v,"SECURE_CABINET") ||
       Floppy144Cabinet25DStringEqual(v,"NONSECURE_CABINET") ||
       Floppy144Cabinet25DStringEqual(v,"SERVER") ||
       Floppy144Cabinet25DStringEqual(v,"FRIDGE") ||
       Floppy144Cabinet25DStringEqual(v,"COFFEE_MAKER"))
        return F144_25D_CHILD_FRONT;
    /* Desks, tables, sinks, benches, chairs, sofas and trolleys. */
    return F144_25D_CHILD_TOP;
}

static Floppy144Cabinet25DPoint Floppy144Cabinet25DOnFace(
    const Floppy144Cabinet25DPoint face[4],
    int32_t u, int32_t v
)
{
    Floppy144Cabinet25DPoint p;
    /* Normalised integer [0,1024] surface coordinates, without floats. */
    int32_t u0=1024-u,v0=1024-v;
    p.x=(face[0].x*u0*v0+face[1].x*u*v0+
         face[2].x*u*v+face[3].x*u0*v)/1048576;
    p.y=(face[0].y*u0*v0+face[1].y*u*v0+
         face[2].y*u*v+face[3].y*u0*v)/1048576;
    return p;
}

static void Floppy144Cabinet25DFaceCorners(
    const Floppy144DataRecord *parent,
    const Floppy144Cabinet25DBox *box,
    Floppy144Cabinet25DPoint face[4],
    Floppy144Cabinet25DChildFace surface,
    int32_t width,
    int32_t depth
)
{
    if(surface==F144_25D_CHILD_WALL_PANEL)
    {
        int32_t axis=width>depth?width:depth;
        int32_t panel_width=80+axis*8,panel_height=70+axis*5;
        int32_t x,y=96;
        (void)parent;
        if(panel_width>224)panel_width=224;
        if(panel_height>154)panel_height=154;
        if(panel_width<96)panel_width=96;
        x=166-panel_width/2;
        face[0]=(Floppy144Cabinet25DPoint){x,y};
        face[1]=(Floppy144Cabinet25DPoint){x+panel_width,y};
        face[2]=(Floppy144Cabinet25DPoint){x+panel_width,y+panel_height};
        face[3]=(Floppy144Cabinet25DPoint){x,y+panel_height};
        return;
    }

    if(surface==F144_25D_CHILD_TOP || surface==F144_25D_CHILD_SHELF)
    {
        face[0]=box->a;face[1]=box->b;
        face[2]=box->c;face[3]=box->d;
        if(parent!=NULL &&
           (Floppy144Cabinet25DStringEqual(parent->pszC,"CHAIR") ||
            Floppy144Cabinet25DStringEqual(parent->pszC,"SOFA")))
        {
            uint32_t i;
            for(i=0U;i<4U;++i)face[i].y+=44;
        }
        return;
    }

    face[0]=box->d;face[1]=box->c;
    face[2]=(Floppy144Cabinet25DPoint){box->c.x,box->c.y+box->height};
    face[3]=(Floppy144Cabinet25DPoint){box->d.x,box->d.y+box->height};

    if(surface==F144_25D_CHILD_SIDE)
    {
        face[0]=box->b;face[1]=box->c;
        face[2]=(Floppy144Cabinet25DPoint){box->c.x,box->c.y+box->height};
        face[3]=(Floppy144Cabinet25DPoint){box->b.x,box->b.y+box->height};
    }
}

static void Floppy144Cabinet25DDrawSurfaceMarker(
    Floppy144Surface *surface,
    const Floppy144Cabinet25DPoint face[4],
    int32_t u0,int32_t v0,int32_t u1,int32_t v1,
    uint32_t fill,uint32_t edge
)
{
    Floppy144Cabinet25DPoint p0=Floppy144Cabinet25DOnFace(face,u0,v0);
    Floppy144Cabinet25DPoint p1=Floppy144Cabinet25DOnFace(face,u1,v0);
    Floppy144Cabinet25DPoint p2=Floppy144Cabinet25DOnFace(face,u1,v1);
    Floppy144Cabinet25DPoint p3=Floppy144Cabinet25DOnFace(face,u0,v1);
    p0.y-=1;p1.y-=1;p2.y-=1;p3.y-=1;
    Floppy144Cabinet25DFillQuad(surface,p0,p1,p2,p3,fill);
    Floppy144Cabinet25DOutlineQuad(surface,p0,p1,p2,p3,edge);
}

static void Floppy144Cabinet25DDrawFaceChildren(
    Floppy144Surface *surface,
    const Floppy144Cabinet25DPoint face[4],
    uint32_t count,
    uint32_t selected
)
{
    const uint32_t muted=F144_25D_RGB(115,132,122);
    const uint32_t bright=F144_25D_RGB(216,239,220);
    const uint32_t trim=F144_25D_RGB(39,53,48);
    uint32_t columns,rows,i;

    if(count==0U)return;
    columns=count<5U?count:5U;
    rows=(count+columns-1U)/columns;
    if(rows>14U)
    {
        columns=(count+13U)/14U;
        rows=(count+columns-1U)/columns;
    }
    for(i=0U;i<count;++i)
    {
        uint32_t row=i/columns,column=i%columns;
        int32_t uc=90+(int32_t)((column*2U+1U)*844U/
            (2U*columns));
        int32_t vc=90+(int32_t)((row*2U+1U)*844U/
            (2U*rows));
        int32_t half_u=(int32_t)(844U/(3U*columns));
        int32_t half_v=(int32_t)(844U/(3U*rows));
        int32_t u0,u1,v0,v1;
        if(half_u>100)half_u=100;
        if(half_v>100)half_v=100;
        u0=uc-half_u;u1=uc+half_u;
        v0=vc-half_v;v1=vc+half_v;
        if(u1<=u0)u1=u0+5;
        if(v1<=v0)v1=v0+5;
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,face,u0,v0,u1,v1,
            i==selected?bright:muted,trim
        );
    }
}

static void Floppy144Cabinet25DDrawSurfaceChildren(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    int32_t width,int32_t depth,bool mirror,
    uint32_t count,uint32_t selected
)
{
    Floppy144Cabinet25DChildFace plane;
    Floppy144Cabinet25DBox box;
    Floppy144Cabinet25DPoint face[4];
    if(surface==NULL||parent==NULL||count==0U)return;
    plane=Floppy144Cabinet25DChildSurface(parent);
    if(Floppy144Cabinet25DStringEqual(parent->pszC,"DOOR") && width<depth)
        plane=F144_25D_CHILD_SIDE;
    box=Floppy144Cabinet25DMakeBox(width,depth,26,mirror);
    if(plane==F144_25D_CHILD_FRONT || plane==F144_25D_CHILD_SIDE || plane==F144_25D_CHILD_SHELF)
    {
        if(Floppy144Cabinet25DStringEqual(parent->pszC,"DOOR"))box.height=138;
        else if(Floppy144Cabinet25DStringEqual(parent->pszC,"SERVER") ||
                Floppy144Cabinet25DStringEqual(parent->pszC,"FRIDGE"))
            box.height=112;
        else if(Floppy144Cabinet25DStringEqual(parent->pszC,"COFFEE_MAKER"))
            box.height=82;
        else box.height=92;
    }
    Floppy144Cabinet25DFaceCorners(parent,&box,face,plane,width,depth);

    Floppy144Cabinet25DDrawFaceChildren(
        surface,face,count,selected
    );
}

static void Floppy144Cabinet25DDrawOrientationTick(
    Floppy144Surface *surface,
    const Floppy144Cabinet25DBox *box,
    uint32_t octant
)
{
    const uint32_t bright=F144_25D_RGB(216,239,220);
    Floppy144Cabinet25DPoint from=box->a;
    Floppy144Cabinet25DPoint to;

    switch(octant)
    {
        case 0U:
        case 1U:
            to=box->b;
            break;

        case 2U:
        case 3U:
            from=box->b;
            to=box->c;
            break;

        case 4U:
        case 5U:
            from=box->c;
            to=box->d;
            break;

        default:
            from=box->d;
            to=box->a;
            break;
    }

    Floppy144Cabinet25DLine(
        surface,
        (from.x*3+to.x)/4,
        (from.y*3+to.y)/4,
        (from.x+to.x*3)/4,
        (from.y+to.y*3)/4,
        bright
    );
}

static void Floppy144Cabinet25DDrawLegs(
    Floppy144Surface *surface,
    const Floppy144Cabinet25DBox *box,
    int32_t leg_height,
    uint32_t colour
)
{
    Floppy144Cabinet25DPoint feet[4]={
        box->a,box->b,box->c,box->d
    };
    uint32_t i;

    for(i=0U;i<4U;++i)
    {
        int32_t start_y=feet[i].y+box->height;

        Floppy144Cabinet25DLine(
            surface,
            feet[i].x,
            start_y,
            feet[i].x,
            start_y+leg_height,
            colour
        );

        Floppy144Cabinet25DLine(
            surface,
            feet[i].x+1,
            start_y,
            feet[i].x+1,
            start_y+leg_height,
            colour
        );
    }
}

/*
 * The screen is upright in the world, not upright in camera pixels: its
 * horizontal edges follow the parent's projected desk-width axis. Its stand
 * and foot are anchored to points on exactly the same top face as loose items.
 */
static void Floppy144Cabinet25DDrawTerminalMonitor(
    Floppy144Surface *surface,
    const Floppy144Cabinet25DBox *box,
    uint32_t edge
)
{
    Floppy144Cabinet25DPoint face[4]={box->a,box->b,box->c,box->d};
    Floppy144Cabinet25DPoint left=
        Floppy144Cabinet25DOnFace(face,270,305);
    Floppy144Cabinet25DPoint right=
        Floppy144Cabinet25DOnFace(face,730,305);
    Floppy144Cabinet25DPoint upper_left=left;
    Floppy144Cabinet25DPoint upper_right=right;
    Floppy144Cabinet25DPoint back_left=
        Floppy144Cabinet25DOnFace(face,270,380);
    Floppy144Cabinet25DPoint back_right=
        Floppy144Cabinet25DOnFace(face,730,380);
    Floppy144Cabinet25DPoint screen_face[4];
    Floppy144Cabinet25DPoint stem=
        Floppy144Cabinet25DOnFace(face,500,450);
    Floppy144Cabinet25DPoint mount=
        Floppy144Cabinet25DOnFace(face,500,305);

    upper_left.y-=43;
    upper_right.y-=43;
    back_left.y-=43;
    back_right.y-=43;

    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,395,435,605,595,
        F144_25D_RGB(74,84,78),edge
    );
    Floppy144Cabinet25DLine(
        surface,stem.x,stem.y,mount.x,mount.y-3,edge
    );
    Floppy144Cabinet25DFillQuad(
        surface,upper_left,upper_right,back_right,back_left,
        F144_25D_RGB(48,60,57)
    );
    Floppy144Cabinet25DFillQuad(
        surface,upper_left,upper_right,right,left,
        F144_25D_RGB(31,40,42)
    );
    Floppy144Cabinet25DOutlineQuad(
        surface,upper_left,upper_right,right,left,edge
    );

    screen_face[0]=upper_left;
    screen_face[1]=upper_right;
    screen_face[2]=right;
    screen_face[3]=left;
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,screen_face,85,105,915,865,
        F144_25D_RGB(26,44,45),F144_25D_RGB(65,91,84)
    );
}

static void Floppy144Cabinet25DDrawDeskFamily(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant,
    uint32_t count,
    uint32_t selected
)
{
    const uint32_t top=F144_25D_RGB(103,116,108);
    const uint32_t front=F144_25D_RGB(71,82,76);
    const uint32_t side=F144_25D_RGB(54,65,60);
    const uint32_t edge=F144_25D_RGB(145,160,151);
    Floppy144Cabinet25DBox box=
        Floppy144Cabinet25DMakeBox(width,depth,14,mirror);
    Floppy144Cabinet25DPoint front_face[4];

    Floppy144Cabinet25DDrawBox(surface,&box,top,front,side,edge);
    Floppy144Cabinet25DDrawLegs(surface,&box,46,edge);
    Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);
    Floppy144Cabinet25DFaceCorners(
        parent,&box,front_face,F144_25D_CHILD_FRONT,width,depth
    );

    /* Face detail is coplanar with the desk, not below its screen silhouette. */
    if(parent!=NULL &&
       Floppy144Cabinet25DStringEqual(parent->pszC,"WORKBENCH"))
    {
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,front_face,90,120,910,820,front,edge
        );
    }
    else
    {
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,front_face,90,160,390,860,front,edge
        );
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,front_face,610,160,910,860,front,edge
        );
    }

    /* Furniture overlays (especially the monitor) must occlude desk items. */
    Floppy144Cabinet25DDrawSurfaceChildren(
        surface,parent,width,depth,mirror,count,selected
    );
    if(parent!=NULL &&
       (Floppy144Cabinet25DStringEqual(parent->pszC,"GDR_TERMINAL") ||
        Floppy144Cabinet25DStringEqual(parent->pszC,"IT_TERMINAL")))
    {
        Floppy144Cabinet25DDrawTerminalMonitor(surface,&box,edge);
    }
}

/*
 * A storage fixture has a front opening on its broad visible side. The
 * catalogue's NONSECURE_CABINET class is also used for objects named
 * BOOKSHELF; choose the opening from canonical geometry/semantics, never a
 * per-object screen offset. Everything, including shelf lips and contents,
 * is drawn in the same face's integer UV coordinates.
 */
static bool Floppy144Cabinet25DIsBookShelf(
    const Floppy144DataRecord *parent
)
{
    const char *v=parent!=NULL?parent->pszC:NULL;
    return Floppy144Cabinet25DStringEqual(v,"BOOKCASE") ||
        Floppy144Cabinet25DStringEqual(v,"SHELVING") ||
        Floppy144Cabinet25DStringEqual(v,"SHELVING_FULL");
}

static void Floppy144Cabinet25DDrawStorageFamily(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant,
    uint32_t count,
    uint32_t selected
)
{
    const uint32_t top=F144_25D_RGB(94,108,101);
    const uint32_t front=F144_25D_RGB(62,75,69);
    const uint32_t side=F144_25D_RGB(45,56,52);
    const uint32_t edge=F144_25D_RGB(145,160,151);
    const uint32_t cavity=F144_25D_RGB(24,34,32);
    const uint32_t shelf=F144_25D_RGB(101,116,107);
    Floppy144Cabinet25DBox box=
        Floppy144Cabinet25DMakeBox(width,depth,92,mirror);
    Floppy144Cabinet25DPoint face[4];
    Floppy144Cabinet25DChildFace front_plane=
        width>=depth?F144_25D_CHILD_FRONT:F144_25D_CHILD_SIDE;
    bool open=Floppy144Cabinet25DIsBookShelf(parent);
    uint32_t row;

    Floppy144Cabinet25DDrawBox(surface,&box,top,front,side,edge);
    Floppy144Cabinet25DFaceCorners(
        parent,&box,face,front_plane,width,depth
    );

    if(!open)
    {
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,face,70,70,490,935,front,edge
        );
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,face,510,70,930,935,front,edge
        );
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,face,450,495,480,545,edge,edge
        );
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,face,520,495,550,545,edge,edge
        );
        Floppy144Cabinet25DDrawFaceChildren(
            surface,face,count,selected
        );
        Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);
        return;
    }

    /* Dark, bounded cavity. No construction line escapes the facade quad. */
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,55,55,945,945,cavity,F144_25D_RGB(49,65,59)
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,95,95,905,905,F144_25D_RGB(34,46,42),cavity
    );

    for(row=0U;row<4U;++row)
    {
        int32_t level=260+(int32_t)row*215;
        uint32_t i;
        /*
         * Upper bays are painted before lower bays. All child rectangles
         * lie behind the shelf front lip, and the casing is painted last.
         * Shelf assignment remains stable under selection/reveal changes.
         */
        for(i=row;i<count;i+=4U)
        {
            uint32_t group=(count+3U-row)/4U;
            uint32_t slot=i/4U;
            int32_t centre;
            int32_t half;
            int32_t u0,u1;
            Floppy144Cabinet25DPoint foot_l,foot_r,head_l,head_r;
            uint32_t fill=i==selected?
                F144_25D_RGB(216,239,220):
                F144_25D_RGB(115,132,122);

            if(group==0U)continue;
            centre=120+(int32_t)((slot*2U+1U)*784U/(2U*group));
            half=(int32_t)(784U/(3U*group));
            if(half>78)half=78;
            if(half<3)half=3;
            u0=centre-half;
            u1=centre+half;
            /* Upright volumes rest immediately behind their projected lip. */
            foot_l=Floppy144Cabinet25DOnFace(face,u0,level-25);
            foot_r=Floppy144Cabinet25DOnFace(face,u1,level-25);
            head_l=Floppy144Cabinet25DOnFace(face,u0,level-151);
            head_r=Floppy144Cabinet25DOnFace(face,u1,level-151);
            Floppy144Cabinet25DFillQuad(
                surface,head_l,head_r,foot_r,foot_l,fill
            );
            Floppy144Cabinet25DOutlineQuad(
                surface,head_l,head_r,foot_r,foot_l,
                F144_25D_RGB(39,53,48)
            );
            /* A shallow projected spine gives even one-page items volume. */
            Floppy144Cabinet25DLine(
                surface,(head_l.x*3+head_r.x)/4,
                (head_l.y*3+head_r.y)/4,
                (foot_l.x*3+foot_r.x)/4,
                (foot_l.y*3+foot_r.y)/4,
                F144_25D_RGB(65,86,77)
            );
        }

        /* The projecting plank occludes the bottom of each item. */
        Floppy144Cabinet25DDrawSurfaceMarker(
            surface,face,82,level-8,918,level+29,shelf,edge
        );
    }

    /* Projected structural jambs hide item edges instead of leaking pixels. */
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,15,0,92,1024,side,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,908,0,985,1024,front,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,15,0,985,72,top,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,15,928,985,1024,front,edge
    );
    Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);
}

static void Floppy144Cabinet25DDrawTableFamily(
    Floppy144Surface *surface,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t top=F144_25D_RGB(111,105,88);
    const uint32_t front=F144_25D_RGB(78,73,60);
    const uint32_t side=F144_25D_RGB(58,55,46);
    const uint32_t edge=F144_25D_RGB(151,143,119);
    Floppy144Cabinet25DBox box=
        Floppy144Cabinet25DMakeBox(width,depth,10,mirror);

    Floppy144Cabinet25DDrawBox(surface,&box,top,front,side,edge);
    Floppy144Cabinet25DDrawLegs(surface,&box,38,edge);
    Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);
}

static void Floppy144Cabinet25DDrawChair(
    Floppy144Surface *surface,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t top=F144_25D_RGB(91,101,96);
    const uint32_t front=F144_25D_RGB(61,71,67);
    const uint32_t side=F144_25D_RGB(48,57,54);
    const uint32_t edge=F144_25D_RGB(154,169,160);
    Floppy144Cabinet25DBox seat=
        Floppy144Cabinet25DMakeBox(width,depth,8,mirror);
    int32_t back_y;
    int32_t left;
    int32_t right;

    seat.a.y+=44;
    seat.b.y+=44;
    seat.c.y+=44;
    seat.d.y+=44;

    Floppy144Cabinet25DDrawBox(surface,&seat,top,front,side,edge);
    Floppy144Cabinet25DDrawLegs(surface,&seat,42,edge);
    Floppy144Cabinet25DDrawOrientationTick(surface,&seat,octant);

    left=Floppy144Cabinet25DMinX(&seat)+8;
    right=Floppy144Cabinet25DMaxX(&seat)-8;
    back_y=seat.a.y-42;

    Floppy144Cabinet25DLine(surface,left,seat.d.y,left,back_y,edge);
    Floppy144Cabinet25DLine(surface,right,seat.b.y,right,back_y,edge);
    Floppy144Cabinet25DLine(surface,left,back_y,right,back_y,edge);
    Floppy144Cabinet25DLine(surface,left,back_y+14,right,back_y+14,edge);
}

static void Floppy144Cabinet25DDrawTrolley(
    Floppy144Surface *surface,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t top=F144_25D_RGB(99,111,105);
    const uint32_t edge=F144_25D_RGB(151,166,157);
    Floppy144Cabinet25DBox shelf=
        Floppy144Cabinet25DMakeBox(width,depth,6,mirror);
    uint32_t level;

    for(level=0U;level<3U;++level)
    {
        Floppy144Cabinet25DBox current=shelf;
        int32_t offset=(int32_t)level*34;

        current.a.y+=offset;
        current.b.y+=offset;
        current.c.y+=offset;
        current.d.y+=offset;

        Floppy144Cabinet25DDrawBox(
            surface,
            &current,
            top,
            F144_25D_RGB(65,76,71),
            F144_25D_RGB(48,58,54),
            edge
        );
    }

    Floppy144Cabinet25DDrawLegs(surface,&shelf,78,edge);
    Floppy144Cabinet25DDrawOrientationTick(surface,&shelf,octant);

    {
        int32_t bottom=Floppy144Cabinet25DBottomY(&shelf)+80;
        int32_t left=Floppy144Cabinet25DMinX(&shelf)+12;
        int32_t right=Floppy144Cabinet25DMaxX(&shelf)-12;

        Floppy144Cabinet25DFillRect(surface,(uint32_t)left,(uint32_t)bottom,10U,5U,edge);
        Floppy144Cabinet25DFillRect(surface,(uint32_t)(right-10),(uint32_t)bottom,10U,5U,edge);
    }
}

static void Floppy144Cabinet25DDrawSofa(
    Floppy144Surface *surface,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    Floppy144Cabinet25DBox seat=
        Floppy144Cabinet25DMakeBox(width,depth,22,mirror);
    Floppy144Cabinet25DBox back=seat;

    seat.a.y+=44;seat.b.y+=44;seat.c.y+=44;seat.d.y+=44;

    Floppy144Cabinet25DDrawBox(
        surface,&seat,
        F144_25D_RGB(87,103,105),
        F144_25D_RGB(55,69,72),
        F144_25D_RGB(42,55,58),
        F144_25D_RGB(137,157,159)
    );

    back.height=58;
    back.a.y+=4;back.b.y+=4;back.c.y+=4;back.d.y+=4;

    Floppy144Cabinet25DDrawBox(
        surface,&back,
        F144_25D_RGB(78,94,96),
        F144_25D_RGB(51,64,67),
        F144_25D_RGB(39,51,54),
        F144_25D_RGB(137,157,159)
    );

    Floppy144Cabinet25DDrawOrientationTick(surface,&seat,octant);
}

static void Floppy144Cabinet25DDrawServerOrFridge(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t edge=F144_25D_RGB(139,157,151);
    Floppy144Cabinet25DBox box=
        Floppy144Cabinet25DMakeBox(width,depth,112,mirror);
    int32_t min_x;
    int32_t max_x;
    uint32_t i;

    Floppy144Cabinet25DDrawBox(
        surface,&box,
        F144_25D_RGB(83,96,91),
        F144_25D_RGB(49,61,57),
        F144_25D_RGB(37,48,44),
        edge
    );

    Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);

    min_x=Floppy144Cabinet25DMinX(&box);
    max_x=Floppy144Cabinet25DMaxX(&box);

    if(
        parent != NULL &&
        Floppy144Cabinet25DStringEqual(parent->pszC,"SERVER")
    )
    {
        for(i=0U;i<7U;++i)
        {
            Floppy144Cabinet25DRect(
                surface,
                (uint32_t)(min_x+10),
                (uint32_t)(box.c.y+12+(int32_t)i*13),
                (uint32_t)(max_x-min_x-20),
                9U,
                edge
            );
        }
    }
    else
    {
        Floppy144Cabinet25DLine(
            surface,
            min_x+8,
            box.c.y+42,
            max_x-8,
            box.c.y+42,
            edge
        );

        Floppy144Cabinet25DFillRect(
            surface,
            (uint32_t)(max_x-18),
            (uint32_t)(box.c.y+16),
            4U,18U,edge
        );
        Floppy144Cabinet25DFillRect(
            surface,
            (uint32_t)(max_x-18),
            (uint32_t)(box.c.y+55),
            4U,32U,edge
        );
    }
}

static void Floppy144Cabinet25DDrawWallFixture(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t edge=F144_25D_RGB(149,165,157);
    int32_t long_axis=width>depth?width:depth;
    int32_t panel_width=80+long_axis*8;
    int32_t panel_height=70+long_axis*5;
    int32_t x;
    int32_t y=96;
    uint32_t i;

    (void)mirror;
    (void)octant;

    if(panel_width>224)panel_width=224;
    if(panel_height>154)panel_height=154;
    if(panel_width<96)panel_width=96;

    x=166-panel_width/2;

    /* Wall shadow plus shallow right/bottom extrusion. */
    Floppy144Cabinet25DFillRect(
        surface,
        (uint32_t)(x+7),
        (uint32_t)(y+7),
        (uint32_t)panel_width,
        (uint32_t)panel_height,
        F144_25D_RGB(35,44,41)
    );
    Floppy144Cabinet25DFillRect(
        surface,
        (uint32_t)x,
        (uint32_t)y,
        (uint32_t)panel_width,
        (uint32_t)panel_height,
        F144_25D_RGB(65,78,72)
    );
    Floppy144Cabinet25DRect(
        surface,
        (uint32_t)x,
        (uint32_t)y,
        (uint32_t)panel_width,
        (uint32_t)panel_height,
        edge
    );

    if(
        parent != NULL &&
        (
            Floppy144Cabinet25DStringEqual(parent->pszC,"SITE_DIRECTORY") ||
            Floppy144Cabinet25DStringEqual(parent->pszC,"NOTICEBOARD")
        )
    )
    {
        Floppy144Cabinet25DRect(
            surface,
            (uint32_t)(x+14),
            (uint32_t)(y+14),
            (uint32_t)(panel_width-28),
            (uint32_t)(panel_height-28),
            edge
        );

        for(i=0U;i<4U;++i)
        {
            Floppy144Cabinet25DFillRect(
                surface,
                (uint32_t)(x+26),
                (uint32_t)(y+30+(int32_t)i*20),
                (uint32_t)(panel_width-52),
                2U,
                edge
            );
        }
    }
    else if(
        parent != NULL &&
        Floppy144Cabinet25DStringEqual(parent->pszC,"MONITOR_BANK")
    )
    {
        int32_t screen=(panel_width-40)/3;

        for(i=0U;i<3U;++i)
        {
            Floppy144Cabinet25DFillRect(
                surface,
                (uint32_t)(x+12+(int32_t)i*(screen+8)),
                (uint32_t)(y+22),
                (uint32_t)screen,
                (uint32_t)(panel_height-44),
                F144_25D_RGB(20,30,33)
            );
            Floppy144Cabinet25DRect(
                surface,
                (uint32_t)(x+12+(int32_t)i*(screen+8)),
                (uint32_t)(y+22),
                (uint32_t)screen,
                (uint32_t)(panel_height-44),
                edge
            );
        }
    }
    else if(
        parent != NULL &&
        Floppy144Cabinet25DStringEqual(parent->pszC,"PATCH_PANEL")
    )
    {
        for(i=0U;i<8U;++i)
        {
            int32_t column=(int32_t)(i%4U);
            int32_t row=(int32_t)(i/4U);

            Floppy144Cabinet25DRect(
                surface,
                (uint32_t)(x+24+column*(panel_width-48)/4),
                (uint32_t)(y+32+row*42),
                18U,18U,edge
            );
        }
    }
    else if(
        parent != NULL &&
        Floppy144Cabinet25DStringEqual(parent->pszC,"FIRST_AID_KIT")
    )
    {
        Floppy144Cabinet25DFillRect(
            surface,
            (uint32_t)(x+panel_width/2-8),
            (uint32_t)(y+24),
            16U,
            (uint32_t)(panel_height-48),
            edge
        );
        Floppy144Cabinet25DFillRect(
            surface,
            (uint32_t)(x+24),
            (uint32_t)(y+panel_height/2-8),
            (uint32_t)(panel_width-48),
            16U,
            edge
        );
    }
    else if(
        parent != NULL &&
        Floppy144Cabinet25DStringEqual(parent->pszC,"KEY_CABINET")
    )
    {
        for(i=0U;i<5U;++i)
        {
            int32_t hook=x+24+(int32_t)i*(panel_width-48)/5;

            Floppy144Cabinet25DLine(
                surface,
                hook,y+32,
                hook,y+panel_height-28,
                edge
            );
            Floppy144Cabinet25DFillRect(
                surface,
                (uint32_t)(hook-2),
                (uint32_t)(y+panel_height-36),
                7U,4U,edge
            );
        }
    }
    else if(
        parent != NULL &&
        Floppy144Cabinet25DStringEqual(parent->pszC,"CABLE_RISER")
    )
    {
        for(i=0U;i<5U;++i)
        {
            int32_t cable=x+26+(int32_t)i*(panel_width-52)/5;
            Floppy144Cabinet25DFillRect(
                surface,
                (uint32_t)cable,
                (uint32_t)(y+16),
                5U,
                (uint32_t)(panel_height-32),
                edge
            );
        }
    }
    else
    {
        Floppy144Cabinet25DRect(
            surface,
            (uint32_t)(x+16),
            (uint32_t)(y+16),
            (uint32_t)(panel_width-32),
            (uint32_t)(panel_height-32),
            edge
        );


    }
}

static void Floppy144Cabinet25DDrawDoor(
    Floppy144Surface *surface,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t edge=F144_25D_RGB(151,164,154);
    Floppy144Cabinet25DBox box=
        Floppy144Cabinet25DMakeBox(width,depth,138,mirror);
    Floppy144Cabinet25DPoint face[4];

    Floppy144Cabinet25DDrawBox(
        surface,&box,
        F144_25D_RGB(87,98,91),
        F144_25D_RGB(61,71,65),
        F144_25D_RGB(44,54,49),
        edge
    );

    /*
     * Door panelling and catch are fixings on the vertical door face.
     * Preserve the existing door cuboid, but project these through its
     * surface instead of using front-facing axis-aligned screen rectangles.
     */
    Floppy144Cabinet25DFaceCorners(
        NULL,&box,face,width>=depth?F144_25D_CHILD_FRONT:F144_25D_CHILD_SIDE,width,depth
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,130,100,870,360,
        F144_25D_RGB(61,71,65),edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,130,540,870,820,
        F144_25D_RGB(61,71,65),edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,790,455,850,515,edge,edge
    );

    Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);
}


/*
 * BF30 paired threshold. One projected frame encloses two full leaves,
 * separated at the centre joint. Both leaf IDs are normalised by the shared
 * double-door identity header and therefore render identically. The plaque
 * is drawn exactly once per container, not once per leaf. The sign text is
 * presentation configuration, never a physical item or connection effect.
 */
static void Floppy144Cabinet25DDrawDoubleDoor(
    Floppy144Surface *surface, const char *sign, bool mirror
)
{
    const uint32_t edge=F144_25D_RGB(161,176,166);
    const uint32_t frame=F144_25D_RGB(77,88,82);
    const uint32_t leaf=F144_25D_RGB(66,80,73);
    const uint32_t lower=F144_25D_RGB(51,65,60);
    Floppy144Cabinet25DBox box=Floppy144Cabinet25DMakeBox(9,1,124,mirror);
    Floppy144Cabinet25DPoint face[4];
    uint32_t word_width;
    uint32_t plaque_x=81U;
    uint32_t plaque_y=49U;
    uint32_t plaque_width=170U;

    Floppy144Cabinet25DDrawBox(
        surface,&box,F144_25D_RGB(99,112,104),frame,lower,edge
    );
    Floppy144Cabinet25DFaceCorners(
        NULL,&box,face,F144_25D_CHILD_FRONT,9,1
    );

    /* Single deep portal opening with two separate hinged leaves. */
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,25,25,999,1000,frame,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,65,72,498,955,leaf,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,502,72,935,955,lower,edge
    );

    /* Matching inset rails and distinct handles on the centre stile. */
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,115,145,440,395,lower,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,560,145,885,395,leaf,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,115,625,440,875,lower,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,560,625,885,875,leaf,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,447,505,476,575,edge,edge
    );
    Floppy144Cabinet25DDrawSurfaceMarker(
        surface,face,524,505,553,575,edge,edge
    );

    /* One overhead sign spanning the pair, outside either leaf's face. */
    Floppy144Cabinet25DFillRect(
        surface,plaque_x,plaque_y,plaque_width,23U,frame
    );
    Floppy144Cabinet25DRect(
        surface,plaque_x,plaque_y,plaque_width,23U,edge
    );
    word_width=Floppy144DrawTextWidth(sign,1U);
    Floppy144DrawText(surface,
        plaque_x+(plaque_width>word_width?
            (plaque_width-word_width)/2U:0U),
        plaque_y+9U,sign,1U,F144_25D_RGB(222,237,223)
    );
}

static void Floppy144Cabinet25DDrawFallback(
    Floppy144Surface *surface,
    int32_t width,
    int32_t depth,
    bool mirror,
    uint32_t octant
)
{
    const uint32_t edge=F144_25D_RGB(143,157,149);
    Floppy144Cabinet25DBox box=
        Floppy144Cabinet25DMakeBox(width,depth,64,mirror);
    int32_t min_x;
    int32_t max_x;

    Floppy144Cabinet25DDrawBox(
        surface,&box,
        F144_25D_RGB(88,101,95),
        F144_25D_RGB(60,72,66),
        F144_25D_RGB(44,55,50),
        edge
    );

    min_x=Floppy144Cabinet25DMinX(&box);
    max_x=Floppy144Cabinet25DMaxX(&box);

    Floppy144Cabinet25DLine(
        surface,
        min_x+10,box.c.y+12,
        max_x-10,box.c.y+50,
        edge
    );
    Floppy144Cabinet25DLine(
        surface,
        max_x-10,box.c.y+12,
        min_x+10,box.c.y+50,
        edge
    );

    Floppy144Cabinet25DDrawOrientationTick(surface,&box,octant);
}

bool Floppy144Cabinet25DDraw(
    Floppy144Surface *surface,
    const Floppy144DataRecord *parent,
    const Floppy144SiteRect *site_rect,
    uint32_t content_count,
    uint32_t selected_content
)
{
    const char *type;
    const char *variant;
    int32_t width;
    int32_t depth;
    bool mirror;
    uint32_t octant;
    bool recognized=true;
    bool children_drawn=false;

    if(
        surface == NULL ||
        surface->pixels == NULL ||
        parent == NULL
    )
    {
        return false;
    }

    /*
     * The right-hand Contents list starts at x=318. Local 2.5D primitives
     * preserve the real framebuffer stride while clipping all presentation
     * pixels to x < 306, leaving a twelve-pixel safety gutter.
     */

    type=parent->pszB!=NULL?parent->pszB:"";
    variant=parent->pszC!=NULL?parent->pszC:"";

    Floppy144Cabinet25DDimensions(
        parent,
        site_rect,
        &width,
        &depth,
        &mirror,
        &octant
    );

    if(Floppy144Cabinet25DStringEqual(type,"WALL_MOUNTED_ITEM"))
    {
        Floppy144Cabinet25DDrawWallFixture(
            surface,parent,width,depth,mirror,octant
        );
    }
    else if(Floppy144DoubleDoorIsPair(parent->pszId))
    {
        Floppy144Cabinet25DDrawDoubleDoor(
            surface,Floppy144DoubleDoorSign(parent->pszId),mirror
        );
        children_drawn=true;
    }
    else if(Floppy144Cabinet25DStringEqual(variant,"DOOR"))
    {
        Floppy144Cabinet25DDrawDoor(
            surface,width,depth,mirror,octant
        );
    }
    else if(Floppy144Cabinet25DStringEqual(variant,"CHAIR"))
    {
        Floppy144Cabinet25DDrawChair(
            surface,width,depth,mirror,octant
        );
    }
    else if(
        Floppy144Cabinet25DStringEqual(variant,"STANDARD_DESK") ||
        Floppy144Cabinet25DStringEqual(variant,"GDR_TERMINAL") ||
        Floppy144Cabinet25DStringEqual(variant,"IT_TERMINAL") ||
        Floppy144Cabinet25DStringEqual(variant,"WORKBENCH")
    )
    {
        Floppy144Cabinet25DDrawDeskFamily(
            surface,parent,width,depth,mirror,octant,
            content_count,selected_content
        );
        children_drawn=true;
    }
    else if(
        Floppy144Cabinet25DStringEqual(variant,"SECURE_CABINET") ||
        Floppy144Cabinet25DStringEqual(variant,"NONSECURE_CABINET") ||
        Floppy144Cabinet25DStringEqual(variant,"BOOKCASE") ||
        Floppy144Cabinet25DStringEqual(variant,"SHELVING_FULL") ||
        Floppy144Cabinet25DStringEqual(variant,"SHELVING")
    )
    {
        Floppy144Cabinet25DDrawStorageFamily(
            surface,parent,width,depth,mirror,octant,
            content_count,selected_content
        );
        children_drawn=true;
    }
    else if(
        Floppy144Cabinet25DStringEqual(variant,"BOARDROOM_TABLE") ||
        Floppy144Cabinet25DStringEqual(variant,"DINING_TABLE") ||
        Floppy144Cabinet25DStringEqual(variant,"COFFEE_TABLE") ||
        Floppy144Cabinet25DStringEqual(variant,"RECEPTION_TABLE") ||
        Floppy144Cabinet25DStringEqual(variant,"WORKTOP") ||
        Floppy144Cabinet25DStringEqual(variant,"SINK")
    )
    {
        Floppy144Cabinet25DDrawTableFamily(
            surface,width,depth,mirror,octant
        );
    }
    else if(Floppy144Cabinet25DStringEqual(variant,"TROLLEY"))
    {
        Floppy144Cabinet25DDrawTrolley(
            surface,width,depth,mirror,octant
        );
    }
    else if(Floppy144Cabinet25DStringEqual(variant,"SOFA"))
    {
        Floppy144Cabinet25DDrawSofa(
            surface,width,depth,mirror,octant
        );
    }
    else if(
        Floppy144Cabinet25DStringEqual(variant,"SERVER") ||
        Floppy144Cabinet25DStringEqual(variant,"FRIDGE")
    )
    {
        Floppy144Cabinet25DDrawServerOrFridge(
            surface,parent,width,depth,mirror,octant
        );
    }
    else if(Floppy144Cabinet25DStringEqual(variant,"COFFEE_MAKER"))
    {
        Floppy144Cabinet25DBox appliance=
            Floppy144Cabinet25DMakeBox(width,depth,82,mirror);

        Floppy144Cabinet25DDrawBox(
            surface,&appliance,
            F144_25D_RGB(89,100,95),
            F144_25D_RGB(55,67,62),
            F144_25D_RGB(42,53,48),
            F144_25D_RGB(149,163,155)
        );

        Floppy144Cabinet25DRect(
            surface,
            (uint32_t)(Floppy144Cabinet25DMinX(&appliance)+14),
            (uint32_t)(appliance.c.y+22),
            (uint32_t)(Floppy144Cabinet25DMaxX(&appliance)-Floppy144Cabinet25DMinX(&appliance)-28),
            28U,
            F144_25D_RGB(149,163,155)
        );
    }
    else if(
        Floppy144Cabinet25DStringEqual(variant,"PARTITION_WALL") ||
        Floppy144Cabinet25DStringEqual(variant,"WINDOW")
    )
    {
        Floppy144Cabinet25DDrawWallFixture(
            surface,parent,width,depth,mirror,octant
        );
    }
    else
    {
        recognized=false;
        Floppy144Cabinet25DDrawFallback(
            surface,width,depth,mirror,octant
        );
    }

    /*
     * Occlusion-sensitive desks and storage paint their children in-depth.
     * Remaining families use the common projected surface helper.
     */
    if(!children_drawn)
    {
        Floppy144Cabinet25DDrawSurfaceChildren(
            surface,parent,width,depth,mirror,content_count,selected_content
        );
    }

    return recognized;
}

#undef F144_25D_RGB
