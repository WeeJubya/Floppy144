/* BF24 review-only synthesis. Not compiled into the game executable.
   Original object layers retain event ordering; A reduces harshness and adds
   a body resonance, B uses restrained sample-and-hold/quantisation. */
static unsigned g_review_variant;
static unsigned g_review_foot;
static const char *const g_review_names[F144_SFX_COUNT]={
    "Keyboard", "Acceptance", "Error / buzzer", "Relay",
    "Filing cabinet / cupboard", "CRT wake", "Power failure",
    "Footstep", "File / collection restored", "Door blocked",
    "Evidence discovered", "Door open", "Floppy insertion",
    "Backspace", "Restoration tick", "Final restoration tick"
};
static unsigned generate_review(F144Voice *v,F144SFX effect,unsigned variant,unsigned foot)
{
    static const unsigned body_hz[F144_SFX_COUNT]={
        340,660,165,220,84,310,70,66,294,68,392,84,190,340,320,380
    };
    unsigned i,n;
    uint32_t phase=0U;
    int filtered=0,held=0;
    /* Original footstep stays auditionable even though gameplay uses A. */
    if(effect==F144_SFX_FOOTSTEP && variant==0U)
        return make_footstep_original(v);
    if(effect==F144_SFX_FOOTSTEP && variant==1U)
        return make_footstep_approved(v,foot);
    if(variant==0U)return generate(v,effect);
    if(effect==F144_SFX_TERMINAL_BACKSPACE) {
        n=generate_review(v,F144_SFX_TERMINAL_KEY,variant,foot);
        for(i=0U;i<n/2U;i++) {
            int16_t s=v->samples[i];
            v->samples[i]=v->samples[n-1U-i];v->samples[n-1U-i]=s;
        }
        return n;
    }
    if(effect==F144_SFX_FOOTSTEP) {
        /* Only the unapproved retro B prototype reaches this recipe. */
        uint32_t toe=0U;
        n=ms_samples(105U);clear_voice(v);
        for(i=0U;i<n;i++) {
            float heel=env_attack_release(i,n,ms_samples(3U));
            float sole=i>=ms_samples(32U)?env_attack_release(i-ms_samples(32U),
                n-ms_samples(32U),ms_samples(4U)):0.0f;
            filtered+=(noise_signed()-filtered)/(variant==1U?12:7);
            held=(int)(triangle_sample(&phase,64U+foot*5U,5200)*heel*heel+
                triangle_sample(&toe,105U+foot*7U,1400)*sole+
                (filtered/9)*heel);
            if(variant==2U)held=(held/128)*128;
            v->samples[i]=clamp16(scaled(held,variant==1U?0.45f:0.40f));
        }
        return n;
    }
    n=generate(v,effect);
    for(i=0U;i<n;i++) {
        int original=v->samples[i];
        float e=env_attack_release(i,n,ms_samples(2U));
        unsigned divisor=3U;
        filtered+=(original-filtered)/(int)divisor;
        if(variant==1U) {
            int resonance=body_hz[effect]?triangle_sample(&phase,body_hz[effect],1000):0;
            v->samples[i]=clamp16((int)(filtered*0.85f)+scaled(resonance,0.20f*e));
        } else {
            if((i&1U)==0U)held=(filtered/96)*96;
            v->samples[i]=clamp16((int)(held*0.85f));
        }
    }
    return n;
}
void F144_SFXReviewVariant(unsigned variant)
{
    g_review_variant=variant<=2U?variant:0U;
}
const char *F144_SFXReviewName(F144SFX effect)
{
    return effect>=0&&effect<F144_SFX_COUNT?g_review_names[effect]:"Unknown";
}
