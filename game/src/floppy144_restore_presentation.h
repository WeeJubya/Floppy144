/* BF23 transient presentation only; never used in capacity or save data. */
#pragma once
#include <stdint.h>
static inline unsigned Floppy144RestoreWeight(unsigned p)
{
    if(p<70U)return 20U;
    if(p<90U)return 20U-(p-70U)*12U/19U;
    if(p<95U)return 25U+(p-90U)*5U;
    if(p<99U)return 80U+(p-95U)*(p-95U)*15U;
    return 400U;
}
static inline unsigned Floppy144RestoreProgress(uint32_t elapsed,uint32_t duration)
{
    unsigned p,total=0U,weight=0U;
    if(duration==0U)return 0U;
    if(elapsed>=duration)return 100U;
    for(p=0U;p<100U;p++)total+=Floppy144RestoreWeight(p);
    for(p=0U;p<100U;p++) {
        weight+=Floppy144RestoreWeight(p);
        if((uint64_t)elapsed*total<(uint64_t)duration*weight)return p;
    }
    return 99U;
}
static inline unsigned Floppy144RestoreTempo(unsigned progress)
{
    return progress<90U?100U:(progress>=100U?100U:100U-(progress-90U)*6U);
}
/* Tick checkpoints follow visible updates, with denser checkpoints as the
   loader accelerates. This avoids a fixed 50 ms audio cadence in early phases. */
static inline unsigned Floppy144RestoreTickStep(unsigned progress)
{
    return progress<70U?4U:(progress<90U?4U-(progress-70U)*3U/19U:1U);
}
static inline int Floppy144RestoreTickDue(unsigned previous,unsigned progress)
{
    return progress<100U && progress>previous &&
        progress-previous>=Floppy144RestoreTickStep(progress);
}
