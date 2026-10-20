/*
 * FLOPPY//144 Stage 4D pseudo-isometric Inspection presentation regression.
 */

#define _CRT_SECURE_NO_WARNINGS 1
#include "floppy144_cabinet_25d.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_WIDTH 640U
#define TEST_HEIGHT 360U
#define TEST_PIXEL_COUNT (TEST_WIDTH * TEST_HEIGHT)
#define ARRAY_COUNT(v) ((uint32_t)(sizeof(v)/sizeof((v)[0])))

static uint32_t g_pixels[TEST_PIXEL_COUNT];
static int g_failures;

#define CHECK(condition,label)                                      \
    do                                                              \
    {                                                               \
        if(!(condition))                                            \
        {                                                           \
            ++g_failures;                                           \
            printf("FAIL: %s\n",(label));                         \
        }                                                           \
    }                                                               \
    while(0)

/*
 * Pull the generated catalogue directly into the test without linking the
 * complete game-data engine. This keeps the presentation test focused while
 * still exercising every real furniture/fixture variant in canonical data.
 */
#define FLOPPY144_DATA_RECORD(kind,id,a,b,c,d,e,f,n0,n1,n2,n3,n4,n5,b0) \
    {kind,id,a,b,c,d,e,f,n0,n1,n2,n3,n4,n5,(uint8_t)(b0)},
static const Floppy144DataRecord g_records[] =
{
#include "floppy144_game_data.generated.inc"
};
#undef FLOPPY144_DATA_RECORD

static uint32_t SurfaceHash(void)
{
    uint32_t hash=2166136261U;
    uint32_t i;

    for(i=0U;i<TEST_PIXEL_COUNT;++i)
    {
        hash^=g_pixels[i];
        hash*=16777619U;
    }

    return hash;
}

static uint32_t RegionPixelCount(
    uint32_t x0,
    uint32_t y0,
    uint32_t x1,
    uint32_t y1
)
{
    uint32_t count=0U;
    uint32_t x;
    uint32_t y;

    if(x1>TEST_WIDTH)x1=TEST_WIDTH;
    if(y1>TEST_HEIGHT)y1=TEST_HEIGHT;

    for(y=y0;y<y1;++y)
    {
        for(x=x0;x<x1;++x)
        {
            if(g_pixels[y*TEST_WIDTH+x]!=0U)
            {
                ++count;
            }
        }
    }

    return count;
}

static const Floppy144DataRecord *FindParent(
    const char *id
)
{
    uint32_t i;

    for(i=0U;i<ARRAY_COUNT(g_records);++i)
    {
        if(
            (
                g_records[i].eKind==FLOPPY144_DATA_FURNITURE ||
                g_records[i].eKind==FLOPPY144_DATA_FIXTURE
            ) &&
            g_records[i].pszId!=NULL &&
            strcmp(g_records[i].pszId,id)==0
        )
        {
            return &g_records[i];
        }
    }

    return NULL;
}

static uint32_t DrawParent(
    const Floppy144DataRecord *parent,
    int16_t rotation,
    uint32_t content_count,
    uint32_t selected,
    bool *recognized
)
{
    Floppy144Surface surface={g_pixels,TEST_WIDTH,TEST_HEIGHT};
    Floppy144SiteRect rect;
    bool result;

    memset(g_pixels,0,sizeof(g_pixels));
    memset(&rect,0,sizeof(rect));

    if(parent!=NULL)
    {
        rect.width=(uint8_t)(parent->n2>0?parent->n2:1);
        rect.height=(uint8_t)(parent->n3>0?parent->n3:1);
        rect.authored_width16=(uint16_t)(rect.width*FLOPPY144_SITE_FIXED_ONE);
        rect.authored_height16=(uint16_t)(rect.height*FLOPPY144_SITE_FIXED_ONE);
    }
    else
    {
        rect.width=4U;
        rect.height=3U;
        rect.authored_width16=64U;
        rect.authored_height16=48U;
    }

    rect.rotation=rotation;

    result=Floppy144Cabinet25DDraw(
        &surface,
        parent,
        &rect,
        content_count,
        selected
    );

    if(recognized!=NULL)
    {
        *recognized=result;
    }

    return SurfaceHash();
}

static void TestEveryCanonicalParentVariant(void)
{
    const char *seen[64]={0};
    uint32_t seen_count=0U;
    uint32_t i;

    for(i=0U;i<ARRAY_COUNT(g_records);++i)
    {
        const Floppy144DataRecord *parent=&g_records[i];
        const char *variant;
        uint32_t s;
        bool duplicate=false;
        bool recognized=false;
        uint32_t hash;

        if(
            parent->eKind!=FLOPPY144_DATA_FURNITURE &&
            parent->eKind!=FLOPPY144_DATA_FIXTURE
        )
        {
            continue;
        }

        variant=parent->pszC!=NULL?parent->pszC:parent->pszB;

        for(s=0U;s<seen_count;++s)
        {
            if(
                seen[s]!=NULL &&
                variant!=NULL &&
                strcmp(seen[s],variant)==0
            )
            {
                duplicate=true;
                break;
            }
        }

        if(duplicate)
        {
            continue;
        }

        CHECK(
            seen_count<ARRAY_COUNT(seen),
            "canonical parent-variant set fits regression table"
        );

        if(seen_count>=ARRAY_COUNT(seen))
        {
            return;
        }

        seen[seen_count++]=variant;

        hash=DrawParent(
            parent,
            0,
            1U,
            0U,
            &recognized
        );

        CHECK(
            recognized,
            "every current canonical parent variant has a specialised family renderer"
        );

        CHECK(
            hash!=0U &&
            RegionPixelCount(30U,70U,302U,306U)>0U,
            "every canonical parent variant produces visible left-panel geometry"
        );

        CHECK(
            RegionPixelCount(306U,0U,TEST_WIDTH,TEST_HEIGHT)==0U,
            "2.5D parent renderer never invades the right-hand contents list"
        );
    }

    CHECK(
        seen_count>=30U,
        "catalogue regression covers the full current furniture/fixture taxonomy"
    );
}

static void TestDimensionsOrientationAndWallFixtures(void)
{
    const Floppy144DataRecord *desk=
        FindParent("MAIN_OFFICE_DESK_01");
    const Floppy144DataRecord *bookcase=
        FindParent("IT_SUPPORT_BOOKCASE");
    const Floppy144DataRecord *rotated=
        FindParent("SECRETARY_OFFICE_DESK");
    const Floppy144DataRecord *wall=
        FindParent("IT_SUPPORT_PATCH_PANEL");
    const Floppy144DataRecord *wide=
        FindParent("STAFF_ROOM_WORKTOP");
    uint32_t desk_hash;
    uint32_t bookcase_hash;
    uint32_t rotation_zero;
    uint32_t rotation_90;
    uint32_t wall_hash;
    uint32_t wide_hash;

    CHECK(desk!=NULL,"6x4 desk fixture exists");
    CHECK(bookcase!=NULL,"2x6 bookcase fixture exists");
    CHECK(rotated!=NULL,"rotated Secretary desk fixture exists");
    CHECK(wall!=NULL,"wall-mounted patch-panel fixture exists");
    CHECK(wide!=NULL,"unusually wide worktop fixture exists");

    desk_hash=DrawParent(desk,0,3U,1U,NULL);
    bookcase_hash=DrawParent(bookcase,0,3U,1U,NULL);

    CHECK(
        desk_hash!=bookcase_hash,
        "6x4 desk and 2x6 bookcase retain distinct dimensional silhouettes"
    );

    rotation_zero=DrawParent(rotated,0,2U,0U,NULL);
    rotation_90=DrawParent(rotated,90,2U,0U,NULL);

    CHECK(
        rotation_zero!=rotation_90,
        "cardinal orientation changes pseudo-isometric Inspection perspective"
    );

    wall_hash=DrawParent(wall,0,4U,2U,NULL);
    wide_hash=DrawParent(wide,0,4U,2U,NULL);

    CHECK(
        wall_hash!=wide_hash,
        "wall-mounted shallow fixture is not rendered as a freestanding worktop"
    );

    CHECK(
        wide!=NULL &&
        wide->n2==18 &&
        wide->n3==5,
        "wide worktop keeps its unusual 18x5 authored dimensions"
    );
}

static void TestContentMarkerStates(void)
{
    const Floppy144DataRecord *desk=
        FindParent("MAIN_OFFICE_DESK_01");
    uint32_t empty_hash;
    uint32_t one_hash;
    uint32_t many_hash;
    uint32_t selected_hash;

    empty_hash=DrawParent(desk,0,0U,0U,NULL);
    one_hash=DrawParent(desk,0,1U,0U,NULL);
    many_hash=DrawParent(desk,0,18U,0U,NULL);
    selected_hash=DrawParent(desk,0,18U,12U,NULL);

    CHECK(
        empty_hash!=one_hash,
        "empty and one-item parent presentations differ"
    );

    CHECK(
        one_hash!=many_hash,
        "one-item and many-item parent presentations differ"
    );

    CHECK(
        many_hash!=selected_hash,
        "selected physical-item marker remains visible in dense contents"
    );
}


/*
 * BUG FIX 10: every authored physical child must resolve to a real inspectable
 * parent. The renderer now chooses a projected parent face by furniture
 * family, rather than drawing a disconnected screen-space marker grid.
 */
static void TestAllAuthoredPhysicalItemParents(void)
{
    const char *seen[256]={0};
    uint32_t inspected=0U;
    uint32_t categories=0U;
    uint32_t i,j;
    const char *variants[64]={0};

    for(i=0U;i<ARRAY_COUNT(g_records);++i)
    {
        const Floppy144DataRecord *child=&g_records[i];
        const Floppy144DataRecord *parent;
        const char *id;
        const char *variant;
        bool duplicate=false;
        uint32_t empty_hash,filled_hash;

        if(child->eKind!=FLOPPY144_DATA_PHYSICAL_ITEM)continue;
        id=child->pszC; /* Authored physical item's parent_id. */
        for(j=0U;j<inspected;++j)
        {
            if(seen[j]!=NULL && id!=NULL &&
               strcmp(seen[j],id)==0)
            {
                duplicate=true;
                break;
            }
        }
        if(duplicate)continue;
        CHECK(id!=NULL && inspected<ARRAY_COUNT(seen),
              "every child has an inspectable parent identifier");
        if(id==NULL || inspected>=ARRAY_COUNT(seen))continue;
        seen[inspected++]=id;
        parent=FindParent(id);
        CHECK(parent!=NULL,"physical item parent resolves to canonical furniture");
        if(parent==NULL)continue;
        variant=parent->pszC!=NULL?parent->pszC:parent->pszB;
        for(j=0U;j<categories;++j)
            if(variants[j]!=NULL && variant!=NULL &&
               strcmp(variants[j],variant)==0)break;
        if(j==categories && categories<ARRAY_COUNT(variants))
            variants[categories++]=variant;

        empty_hash=DrawParent(parent,0,0U,0U,NULL);
        filled_hash=DrawParent(parent,0,1U,0U,NULL);
        CHECK(empty_hash!=filled_hash,
              "authored parent children alter projected Inspection surface");
        CHECK(RegionPixelCount(306U,0U,TEST_WIDTH,TEST_HEIGHT)==0U,
              "surface-mapped children are clipped before the contents list");
    }
    CHECK(inspected==138U,
          "all 138 populated physical-item parents receive projected children (34 empty chairs)");
    CHECK(categories>=28U,
          "surface projection audits all 28 populated furniture categories");
}

/*
 * Pixel-level geometry check. Only NEW selected-child pixels are counted, so
 * a parent's existing orientation tick and built-in door fittings are ignored.
 * The Desk 06 top and corridor door front are tested separately; neither
 * child may appear on the floor below the furniture as before.
 */
static uint32_t g_child_baseline[TEST_PIXEL_COUNT];

static uint32_t TestNewChildColourInBounds(
    uint32_t x0,uint32_t y0,uint32_t x1,uint32_t y1,
    uint32_t *outside
)
{
    const uint32_t selected=0x00D8EFDCU;
    uint32_t x,y,inside=0U;
    *outside=0U;
    for(y=0U;y<TEST_HEIGHT;++y)
    for(x=0U;x<TEST_WIDTH;++x)
    {
        uint32_t at=y*TEST_WIDTH+x;
        if(g_pixels[at]!=selected ||
           g_child_baseline[at]==selected)continue;
        if(x>=x0 && x<=x1 && y>=y0 && y<=y1)++inside;
        else ++*outside;
    }
    return inside;
}

static void TestDeskAndDoorChildSurfaces(void)
{
    const Floppy144DataRecord *desk=FindParent("MAIN_OFFICE_DESK_06");
    const Floppy144DataRecord *door=FindParent("COR_REC");
    const Floppy144DataRecord *rotated=FindParent("SECRETARY_OFFICE_DESK");
    const Floppy144DataRecord *bookcase=FindParent("IT_SUPPORT_BOOKCASE");
    const Floppy144DataRecord *wall=FindParent("IT_SUPPORT_PATCH_PANEL");
    uint32_t inside,outside;

    CHECK(desk!=NULL && door!=NULL && rotated!=NULL,
          "Desk 06, corridor door and rotated desk exist");
    if(desk==NULL || door==NULL || rotated==NULL)return;

    (void)DrawParent(desk,0,0U,0U,NULL);
    memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
    (void)DrawParent(desk,0,10U,0U,NULL);
    inside=TestNewChildColourInBounds(79U,85U,295U,196U,&outside);
    CHECK(inside>0U && outside==0U,
          "Desk 06 child documents lie only on its 2.5D desktop plane");

    {
        const Floppy144DataRecord *panel=FindParent("MAIN_OFFICE_SUPPRESSION_PANEL");
        CHECK(panel!=NULL,"suppression panel exists");
        if(panel!=NULL) {
            (void)DrawParent(panel,0,0U,0U,NULL);
            CHECK(RegionPixelCount(230U,96U,305U,255U)==0U,
                  "suppression panel has no legacy blocks beyond its silhouette");
        }
    }

    /* Signed authored orientation still maps children to a rotated top. */
    (void)DrawParent(rotated,90,0U,0U,NULL);
    memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
    (void)DrawParent(rotated,90,5U,0U,NULL);
    inside=TestNewChildColourInBounds(30U,85U,303U,205U,&outside);
    CHECK(inside>0U && outside==0U,
          "cardinal desks inherit their orthogonal desktop perspective");

    (void)DrawParent(door,0,0U,0U,NULL);
    memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
    (void)DrawParent(door,0,3U,0U,NULL);
    inside=TestNewChildColourInBounds(75U,110U,195U,290U,&outside);
    CHECK(inside>0U && outside==0U,
          "corridor door children stay on the broad vertical door face");

    /* Four shelving levels are copies of the top plane at authored lips. */
    CHECK(bookcase!=NULL && wall!=NULL,
          "bookcase and wall fixture examples exist");
    if(bookcase!=NULL)
    {
        (void)DrawParent(bookcase,0,0U,0U,NULL);
        memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
        (void)DrawParent(bookcase,0,4U,3U,NULL);
        inside=TestNewChildColourInBounds(15U,200U,303U,306U,&outside);
        CHECK(inside>0U && outside==0U,
              "bottom shelf item follows the projected shelf plane");
    }

    /* Wall-mounted fixture labels use the existing panel surface. */
    if(wall!=NULL)
    {
        (void)DrawParent(wall,0,0U,0U,NULL);
        memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
        (void)DrawParent(wall,0,3U,0U,NULL);
        inside=TestNewChildColourInBounds(38U,92U,295U,255U,&outside);
        CHECK(inside>0U && outside==0U,
              "wall-mounted items inherit the existing fixture panel face");
    }
}

/*
 * Optional deterministic framebuffer captures for comparing projected
 * geometry. These are test outputs only, never shipped/stored game assets.
 * The actual Windows executable must still be visually accepted separately.
 */
static void BF10Capture(const char *name)
{
    const char *dir=getenv("F144_BF10_CAPTURE_DIR");
    char path[512];
    FILE *out;
    uint32_t i;
    int size;

    if(dir==NULL||dir[0]=='\0')return;
    size=snprintf(path,sizeof(path),"%s/%s.ppm",dir,name);
    CHECK(size>0 && (size_t)size<sizeof(path),"BF10 capture path fits");
    if(size<=0||(size_t)size>=sizeof(path))return;

    out=fopen(path,"wb");
    CHECK(out!=NULL,"BF10 visual regression frame opens");
    if(out==NULL)return;
    fprintf(out,"P6\n%u %u\n255\n",TEST_WIDTH,TEST_HEIGHT);
    for(i=0U;i<TEST_PIXEL_COUNT;++i)
    {
        uint32_t colour=g_pixels[i];
        fputc((int)((colour>>16)&255U),out);
        fputc((int)((colour>>8)&255U),out);
        fputc((int)(colour&255U),out);
    }
    fclose(out);
}

static void TestBF10NamedVisualCases(void)
{
    const Floppy144DataRecord *terminal=
        FindParent("RECEPTION_TERMINAL_DESK");
    const Floppy144DataRecord *shelf=
        FindParent("MAIN_OFFICE_BOOKSHELF_03");
    const Floppy144DataRecord *desk=
        FindParent("MAIN_OFFICE_DESK_06");
    const Floppy144DataRecord *door=
        FindParent("COR_REC");
    uint32_t empty,filled;

    CHECK(terminal!=NULL && shelf!=NULL && desk!=NULL && door!=NULL,
          "BF10 has all four authored Windows visual-acceptance fixtures");
    CHECK(shelf!=NULL && shelf->pszC!=NULL && shelf->pszB!=NULL &&
          strcmp(shelf->pszC,"BOOKCASE")==0 &&
          strcmp(shelf->pszB,"BOOKCASE")==0,
          "Bookshelf 03 uses canonical BOOKCASE furniture/variant");
    {
        const Floppy144DataRecord *sibling1=FindParent("MAIN_OFFICE_BOOKSHELF_01");
        const Floppy144DataRecord *sibling2=FindParent("MAIN_OFFICE_BOOKSHELF_02");
        CHECK(sibling1!=NULL && sibling2!=NULL &&
              strcmp(sibling1->pszC,"BOOKCASE")==0 &&
              strcmp(sibling2->pszC,"BOOKCASE")==0,
              "all three Main Office bookshelves share BOOKCASE classification");
    }
    if(terminal==NULL||shelf==NULL||desk==NULL||door==NULL)return;

    empty=DrawParent(terminal,0,0U,0U,NULL);
    filled=DrawParent(terminal,0,1U,0U,NULL);
    CHECK(empty!=filled,"Reception Terminal Desk contents appear on desktop");
    BF10Capture("reception_terminal_desk");

    empty=DrawParent(shelf,0,0U,0U,NULL);
    memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
    filled=DrawParent(shelf,0,2U,1U,NULL);
    CHECK(empty!=filled,"Main Office Bookshelf 03 shows two shelf items");
    {
        uint32_t outside=0U,i;
        for(i=0U;i<TEST_PIXEL_COUNT;++i)
        {
            if(g_pixels[i]!=0U && g_child_baseline[i]==0U)++outside;
        }
        CHECK(outside==0U,"Bookshelf 03 children stay within furniture silhouette");
    }
    CHECK(RegionPixelCount(306U,0U,TEST_WIDTH,TEST_HEIGHT)==0U,
          "Bookshelf 03 construction is panel-clipped");
    BF10Capture("main_office_bookshelf_03");

    (void)DrawParent(desk,0,10U,3U,NULL);
    BF10Capture("main_office_desk_06");

    (void)DrawParent(door,0,3U,0U,NULL);
    BF10Capture("corridor_records_door");
}


/* BF30: both existing leaves of each physical pair must yield one identical
 * image. Two types have distinct single plaques and no PI markers. */
static void TestDoubleDoorAssemblies(void)
{
    const Floppy144DataRecord *entry_a=FindParent("ENTRY_01");
    const Floppy144DataRecord *entry_b=FindParent("ENTRY_02");
    const Floppy144DataRecord *emergency_a=FindParent("EMER_01");
    const Floppy144DataRecord *emergency_b=FindParent("EMER_02");
    uint32_t entry_first,entry_second,emergency_first,emergency_second;
    bool recognized=false;

    CHECK(entry_a!=NULL && entry_b!=NULL &&
          emergency_a!=NULL && emergency_b!=NULL,
          "BF30 all four authored external door leaves exist");
    if(entry_a==NULL||entry_b==NULL||emergency_a==NULL||emergency_b==NULL)
        return;
    entry_first=DrawParent(entry_a,0,0U,0U,&recognized);
    CHECK(recognized,"BF30 main entrance has a specialised shared container");
    CHECK(RegionPixelCount(70U,45U,260U,80U)>0U,
          "BF30 one main entrance sign occupies the header");
    CHECK(RegionPixelCount(306U,0U,640U,360U)==0U,
          "BF30 entrance geometry remains in the left-hand frame");
    BF10Capture("main_entrance_double_door");
    entry_second=DrawParent(entry_b,0,0U,0U,&recognized);
    CHECK(recognized && entry_first==entry_second,
          "BF30 either entrance leaf paints exactly the same assembly");

    emergency_first=DrawParent(emergency_a,0,0U,0U,&recognized);
    CHECK(recognized,"BF30 emergency exit has a specialised shared container");
    CHECK(RegionPixelCount(70U,45U,260U,80U)>0U,
          "BF30 one emergency exit sign occupies the header");
    BF10Capture("emergency_exit_double_door");
    emergency_second=DrawParent(emergency_b,0,0U,0U,&recognized);
    CHECK(recognized && emergency_first==emergency_second,
          "BF30 either emergency leaf paints exactly the same assembly");
    CHECK(entry_first!=emergency_first,
          "BF30 entrance and emergency signs differ visibly");
    CHECK(RegionPixelCount(306U,0U,640U,360U)==0U,
          "BF30 emergency construction cannot bleed into Contents");
    (void)DrawParent(emergency_a,90,0U,0U,&recognized);
    CHECK(recognized && RegionPixelCount(306U,0U,640U,360U)==0U,
          "BF30 rotated pair respects framebuffer clipping");
}

/*
 * BF40: cover the painter-order regression with pixel tests. Rear supports
 * begin underneath a projected face and must never repaint its solid top.
 * A synthetic 4x4 footprint keeps the sample points independent of authors'
 * future changes to particular room furniture.
 */
static void TestBF40FurnitureDepth(void)
{
    Floppy144DataRecord parent;
    uint32_t i;
    static const char *variants[3]={
        "STANDARD_DESK","BOARDROOM_TABLE","CHAIR"
    };
    static const uint32_t top_colours[3]={
        FLOPPY144_RGB(103,116,108),
        FLOPPY144_RGB(111,105,88),
        FLOPPY144_RGB(91,101,96)
    };

    memset(&parent,0,sizeof(parent));
    parent.eKind=FLOPPY144_DATA_FURNITURE;
    parent.pszB="FURNITURE";
    parent.n2=4;
    parent.n3=4;
    for(i=0U;i<3U;++i)
    {
        int16_t rotation;
        parent.pszC=variants[i];
        for(rotation=0;rotation<360;rotation+=90)
        {
            uint32_t y=i==2U?160U:120U;
            (void)DrawParent(&parent,rotation,0U,0U,NULL);
            CHECK(
                g_pixels[y*TEST_WIDTH+166U]==top_colours[i],
                "BF40 rear support cannot show through a solid top/seat"
            );
            CHECK(
                RegionPixelCount(306U,0U,TEST_WIDTH,TEST_HEIGHT)==0U,
                "BF40 rotated furniture remains inside the Inspection panel"
            );
        }
    }

    parent.pszC="GDR_TERMINAL";
    (void)DrawParent(&parent,0,0U,0U,NULL);
    CHECK(
        g_pixels[147U*TEST_WIDTH+166U]==FLOPPY144_RGB(103,116,108),
        "BF40 terminal rear leg stays behind the desktop"
    );
}

static uint32_t BF40CountColour(uint32_t colour)
{
    uint32_t i,count=0U;
    for(i=0U;i<TEST_PIXEL_COUNT;++i)
        if(g_pixels[i]==colour)++count;
    return count;
}

static void TestBF40ChairBackVolume(void)
{
    Floppy144DataRecord chair;
    int16_t rotation;
    memset(&chair,0,sizeof(chair));
    chair.eKind=FLOPPY144_DATA_FURNITURE;
    chair.pszB="CHAIR";
    chair.pszC="CHAIR";
    chair.n2=4;
    chair.n3=4;
    for(rotation=0;rotation<360;rotation+=90)
    {
        (void)DrawParent(&chair,rotation,0U,0U,NULL);
        CHECK(
            BF40CountColour(FLOPPY144_RGB(78,93,85))>25U,
            "BF40 chair back has a filled vertical 2.5D face"
        );
        CHECK(
            BF40CountColour(FLOPPY144_RGB(115,129,120))>5U,
            "BF40 chair back has a projected solid top face"
        );
    }
}

static void TestBF40TerminalMarkerForeground(void)
{
    Floppy144DataRecord terminal;
    uint32_t i,overlap;
    static const char *variants[2]={"GDR_TERMINAL","IT_TERMINAL"};
    uint32_t variant_index;

    memset(&terminal,0,sizeof(terminal));
    terminal.eKind=FLOPPY144_DATA_FURNITURE;
    terminal.pszB="TERMINAL_DESK";
    terminal.n2=4;
    terminal.n3=4;

    for(variant_index=0U;variant_index<2U;++variant_index)
    {
        terminal.pszC=variants[variant_index];
        (void)DrawParent(&terminal,0,0U,0U,NULL);
        memcpy(g_child_baseline,g_pixels,sizeof(g_pixels));
        (void)DrawParent(&terminal,0,10U,2U,NULL);
        overlap=0U;
        for(i=0U;i<TEST_PIXEL_COUNT;++i)
        {
            if(
                g_child_baseline[i]==FLOPPY144_RGB(26,44,45) &&
                g_pixels[i]==FLOPPY144_RGB(216,239,220)
            )
            {
                ++overlap;
            }
        }
        CHECK(
            overlap>0U,
            "BF40 terminal inspection highlight is in front of monitor screen"
        );
        CHECK(
            RegionPixelCount(306U,0U,TEST_WIDTH,TEST_HEIGHT)==0U,
            "BF40 terminal highlight respects Contents gutter"
        );
    }
}

static void TestUnknownFallback(void)
{
    Floppy144DataRecord future_parent;
    bool recognized=true;
    uint32_t hash;

    memset(&future_parent,0,sizeof(future_parent));

    future_parent.eKind=FLOPPY144_DATA_FURNITURE;
    future_parent.pszId="FUTURE_UNKNOWN_PARENT";
    future_parent.pszA="MAIN_OFFICE";
    future_parent.pszB="FUTURE_FURNITURE";
    future_parent.pszC="QUANTUM_FILING_THING";
    future_parent.n2=7;
    future_parent.n3=3;

    hash=DrawParent(
        &future_parent,
        45,
        0U,
        0U,
        &recognized
    );

    CHECK(
        !recognized,
        "unknown future class reports generic fallback use"
    );

    CHECK(
        hash!=0U &&
        RegionPixelCount(30U,70U,302U,306U)>0U,
        "unknown future class still renders safe bounded 2.5D geometry"
    );
}

int main(void)
{
    TestEveryCanonicalParentVariant();
    TestDimensionsOrientationAndWallFixtures();
    TestContentMarkerStates();
    TestAllAuthoredPhysicalItemParents();
    TestDeskAndDoorChildSurfaces();
    TestBF10NamedVisualCases();
    TestDoubleDoorAssemblies();
    TestBF40FurnitureDepth();
    TestBF40ChairBackVolume();
    TestBF40TerminalMarkerForeground();
    TestUnknownFallback();

    if(g_failures!=0)
    {
        printf(
            "STAGE 4D INSPECTION 2.5D TESTS: FAIL (%d)\n",
            g_failures
        );
        return 1;
    }

    printf(
        "STAGE 4D INSPECTION 2.5D TESTS: PASS\n"
    );
    return 0;
}
