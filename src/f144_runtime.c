#include "f144_runtime.h"
#include <stdlib.h>

#define BILLION 1000000000

f_internal void calcDelta
(
    const F144Time *time1,
    const F144Time *time2,
    int64_t      *deltaS,
    int64_t      *deltaNS
){
    if(time1->ns > BILLION)
    {
        fprintf(stderr, "\033[31mERROR: timestamp 0 is malformed.\033[0m\n");
    }
    if(time2->ns > BILLION)
    {
        fprintf(stderr, "\033[31mERROR: timestamp 1 is malformed.\033[0m\n");
    }

    // uninitialized times
    if(time1->s == INT64_MIN && time2->s > 0)
    {
        *deltaS = time2->s;
    }
    else
    {
        *deltaS  = time2->s  - time1->s;
    }

    if(time1->ns == INT64_MIN && time2->ns > 0)
    {
        *deltaNS = time2->ns;
    }
    else
    {
        *deltaNS = time2->ns - time1->ns;
    }

    if(*deltaNS < 0)
    {
        *deltaS  -= 1;
        *deltaNS += BILLION;
    }
}

F144Time f144DeltaTime
(
    const F144Time *time1,
    const F144Time *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        F144Time time = {0, 0};
        return time;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    F144Time delta = {deltaS, deltaNS};
    return delta;
}

float f144DeltaTime_ms
(
    const F144Time *time1,
    const F144Time *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (float)((float)deltaS * 1e3f + (float)deltaNS / 1e6f);
}

extern int64_t f144DeltaTime_ns
(
    const F144Time *time1,
    const F144Time *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (deltaS * BILLION + deltaNS);
}

F144Time f144DeltaTime_now
(
    const F144Time *time
){
    if(!time || !time->s)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        F144Time result = {-1, -1};
        return result;
    }

    F144Time current = f144QueryTime();
    int64_t   deltaS  = 0;
    int64_t   deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        F144Time result = {0, 0};
        return result;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    F144Time delta = {deltaS, deltaNS};
    return delta;
}

float f144DeltaTime_now_ms
(
    const F144Time *time
){
    if(!time)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    F144Time current = f144QueryTime();
    int64_t   deltaS  = 0;
    int64_t   deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        return -2;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return (float)((float)deltaS * 1e3f + (float)deltaNS / 1e6f);
}

extern uint64_t f144DeltaTime_now_ns
(
    const F144Time *time
){
    if(!time)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return BILLION + 1;
    }

    F144Time current = f144QueryTime();
    int64_t   deltaS  = 0;
    int64_t   deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        return BILLION + 2;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return(uint64_t)(deltaS * BILLION + deltaNS);
}

void f144AppendImage
(
    F144Runtime *runtime,
    F144Image *src,
    F144Image *dst,
    uint8_t    direction
){
    uint32_t og_width  = dst->width;
    uint32_t og_height = dst->height;

    uint32_t width  = dst->width;
    uint32_t height = dst->height;

    if(direction == F144_HORIZONTAL)
    {
        width = dst->width + src->width;

        if(dst->height < src->height)
        {
            height = src->height;
        }
    }
    else if(direction == F144_VERTICAL)
    {
        height = dst->height + src->height;

        if(dst->width < src->width)
        {
            width = src->width;
        }
    }
    else
    {
        fprintf(stderr, "\033[31m\nERROR: unkown direction, cannot append.\n\033[0m");
        return;
    }

    F144Image tmp = {0};
    f144CreateImage(runtime, &tmp, width, height);

    f144CompositeSettings comp = {0};
    comp.dst                 = &tmp;
    comp.pictop              = F144_PICTOP_OVER;

    if(dst->data)
    {
        comp.src        = dst;
        comp.cropWidth  = dst->width;
        comp.cropHeight = dst->height;

        f144CompositeImage(runtime, &comp);

        f144DestroyImage(dst);
    }

    comp.src        = src;
    comp.offsetDstX = direction == F144_HORIZONTAL ? og_width  : 0;
    comp.offsetDstY = direction == F144_VERTICAL   ? og_height : 0;
    comp.cropHeight = src->height;
    comp.cropWidth  = src->width;

    f144CompositeImage(runtime, &comp);

    dst->path     = tmp.path;
    dst->data     = tmp.data;
    dst->width    = tmp.width;
    dst->height   = tmp.height;
    dst->channels = tmp.channels;

    #ifdef BUILD_LINUX
    dst->pixmap  = tmp.pixmap;
    dst->picture = tmp.picture;
    #endif

    #ifdef BUILD_WINDOWS
    dst->info = tmp.info;
    #endif
}

void f144DestroyImage
(
    F144Image *image
){
    if(!image)
    {
        fprintf(stderr, "No image to be freed.\n");
        return;
    }

    if(image->data)
    {
        free(image->data);
        image->data = 0;
    }
}
