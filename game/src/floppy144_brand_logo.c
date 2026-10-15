/*
 * Grey Door Republik, original architectural-portal logo.
 * The reference mark is vectorised from the supplied studio PNG:
 * pale circular disc, overlapping grey facade, black portal/door,
 * muted red foot, and two-line squared-wordmark.
 *
 * Deliberately all software FillRect/DrawText, no stored bitmap bytes.
 */
#include "floppy144_brand_logo.h"
#include <stddef.h>

typedef struct { int32_t x,y; } F144LogoPoint;
#define COL(R,G,B) FLOPPY144_RGB((R),(G),(B))
#define F144_LOGO_GREY COL(116,114,109)
#define F144_LOGO_BLACK COL(23,24,23)
#define F144_LOGO_DISC COL(219,213,202)
#define F144_LOGO_RED COL(162,62,43)

static void LogoPolygon(
    Floppy144Surface *surface,uint32_t ox,uint32_t oy,
    uint32_t w,uint32_t h,const F144LogoPoint *p,uint32_t count,
    uint32_t colour
)
{
    uint32_t row,i;
    if(surface==NULL||count<3U||w==0U||h==0U) return;
    for(row=0U;row<h;++row)
    {
        int32_t scan=(int32_t)((row*100U+h/2U)/h);
        int32_t left=101,right=-1;
        for(i=0U;i<count;++i)
        {
            const F144LogoPoint a=p[i],b=p[(i+1U)%count];
            int32_t low=a.y<b.y?a.y:b.y,high=a.y>b.y?a.y:b.y;
            if(scan>=low&&scan<high&&a.y!=b.y)
            {
                int32_t ix=a.x+(scan-a.y)*(b.x-a.x)/(b.y-a.y);
                if(ix<left)left=ix;
                if(ix>right)right=ix;
            }
        }
        if(right>=left)
        {
            uint32_t x0=ox+(uint32_t)(left<0?0:left)*w/100U;
            uint32_t x1=ox+(uint32_t)(right>100?100:right)*w/100U;
            if(x1<x0+1U)x1=x0+1U;
            Floppy144DrawFillRect(surface,x0,oy+row,x1-x0,1U,colour);
        }
    }
}
static void LogoDisc(
    Floppy144Surface *surface,uint32_t x,uint32_t y,
    uint32_t w,uint32_t h,uint32_t colour
)
{
    uint32_t row;
    for(row=0U;row<h;++row)
    {
        int32_t ny=(int32_t)((row*100U+h/2U)/h)-26;
        int32_t nx,r2=19*19-ny*ny;
        if(r2<0)continue;
        for(nx=0;nx<20&&nx*nx<=r2;++nx){}
        if(nx>0)--nx;
        {
            uint32_t left=x+(uint32_t)(70-nx)*w/100U;
            uint32_t right=x+(uint32_t)(70+nx)*w/100U;
            if(right>left)Floppy144DrawFillRect(surface,left,y+row,right-left,1U,colour);
        }
    }
}
static void LogoMark(
    Floppy144Surface *surface,uint32_t x,uint32_t y,
    uint32_t w,uint32_t h
)
{
    static const F144LogoPoint shaft[]={
        {44,15},{56,5},{56,79},{44,79}
    };
    static const F144LogoPoint facade[]={
        {11,44},{44,27},{44,79},{11,79}
    };
    static const F144LogoPoint portal[]={
        {59,33},{83,48},{83,73},{59,66}
    };
    static const F144LogoPoint accent[]={
        {56,69},{95,79},{56,79}
    };
    LogoDisc(surface,x,y,w,h,F144_LOGO_DISC);
    LogoPolygon(surface,x,y,w,h,shaft,4U,F144_LOGO_BLACK);
    LogoPolygon(surface,x,y,w,h,facade,4U,F144_LOGO_GREY);
    LogoPolygon(surface,x,y,w,h,portal,4U,F144_LOGO_BLACK);
    LogoPolygon(surface,x,y,w,h,accent,3U,F144_LOGO_RED);
}
static void LogoCentred(
    Floppy144Surface *s,uint32_t x,uint32_t width,
    uint32_t y,const char *line,uint32_t scale
)
{
    uint32_t size=Floppy144DrawTextWidth(line,scale);
    Floppy144DrawText(s,x+(size<width?(width-size)/2U:0U),y,line,scale,F144_LOGO_BLACK);
}
void Floppy144BrandLogoDraw(
    Floppy144Surface *surface,
    uint32_t x,uint32_t y,uint32_t width,uint32_t height
)
{
    uint32_t scale,mark;
    if(surface==NULL||surface->pixels==NULL||width<32U||height<24U)return;

    /*
     * One canonical mark and palette, with a horizontal lockup for the
     * narrow Grey Door office sign and a stacked lockup in the Credits.
     */
    if(width>=height*3U)
    {
        uint32_t remaining,tx,ty;
        mark=height;
        LogoMark(surface,x,y,mark,mark);
        tx=x+mark+8U;
        remaining=width-mark-8U;
        scale=remaining>=190U&&height>=37U?2U:1U;
        ty=y+(height>30U?(height-30U)/2U:2U);
        LogoCentred(surface,tx,remaining,ty,"GREY DOOR",scale);
        LogoCentred(surface,tx,remaining,ty+scale*10U,"REPUBLIK",scale);
        Floppy144DrawFillRect(surface,tx+remaining/2U-8U,
            ty+scale*21U,16U,2U,F144_LOGO_RED);
    }
    else
    {
        uint32_t words=height>90U?height/3U:24U;
        uint32_t mark_h=height-words;
        mark=mark_h<width?mark_h:width;
        LogoMark(surface,x+(width-mark)/2U,y,mark,mark);
        scale=width>=125U&&words>=36U?2U:1U;
        LogoCentred(surface,x,width,y+mark_h,"GREY DOOR",scale);
        LogoCentred(surface,x,width,y+mark_h+scale*10U,"REPUBLIK",scale);
        Floppy144DrawFillRect(surface,x+width/2U-8U,
            y+mark_h+scale*21U,16U,2U,F144_LOGO_RED);
    }
}
