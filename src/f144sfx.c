#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "f144sfx.h"

#pragma comment(lib, "winmm.lib")

#define F144_RATE 11025u
#define F144_VOICES 4u
#define F144_MAX_SAMPLES 26460u

typedef struct {
    WAVEHDR hdr;
    int16_t samples[F144_MAX_SAMPLES];
    int prepared;
} F144Voice;

static HWAVEOUT g_wave = NULL;
static F144Voice g_voice[F144_VOICES];
static int g_available = 0;
static float g_volume = 0.80f;
static uint32_t g_noise = 0xF14451F1u;
static unsigned g_footstep_side = 0u;

static int16_t clamp16(int v)
{
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

static uint32_t noise32(void)
{
    uint32_t x = g_noise;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_noise = x;
    return x;
}

static int noise_signed(void)
{
    return (int)((noise32() >> 16) & 0xFFFFu) - 32768;
}

static int square_sample(uint32_t *phase, unsigned freq, int amp)
{
    uint32_t inc = (uint32_t)(((uint64_t)freq << 16) / F144_RATE);
    *phase += inc;
    return ((*phase & 0x8000u) != 0u) ? amp : -amp;
}

static int triangle_sample(uint32_t *phase, unsigned freq, int amp)
{
    uint32_t inc = (uint32_t)(((uint64_t)freq << 16) / F144_RATE);
    uint32_t p;
    int tri;

    *phase += inc;
    p = *phase & 0xFFFFu;

    tri = (p < 32768u)
        ? ((int)p - 16384)
        : (49151 - (int)p);

    return (tri * amp) / 16384;
}

static float env_linear(unsigned i, unsigned total)
{
    if (!total) return 0.0f;
    return 1.0f - ((float)i / (float)total);
}

static float env_attack_release(unsigned i, unsigned total, unsigned attack)
{
    if (!total) return 0.0f;

    if (i < attack && attack > 0u)
        return (float)i / (float)attack;

    return 1.0f - ((float)(i - attack) /
        (float)((total > attack) ? (total - attack) : 1u));
}

static int scaled(int sample, float gain)
{
    float v = (float)sample * gain * g_volume;

    if (v > 32767.0f) v = 32767.0f;
    if (v < -32768.0f) v = -32768.0f;

    return (int)v;
}

static unsigned ms_samples(unsigned ms)
{
    return (unsigned)(((uint64_t)F144_RATE * ms) / 1000u);
}

static void clear_voice(F144Voice *v)
{
    memset(v->samples, 0, sizeof(v->samples));
}

static unsigned make_terminal_key(F144Voice *v)
{
    unsigned n = ms_samples(42), i;
    uint32_t phase = 0u;

    for (i = 0; i < n; ++i) {
        float e = env_linear(i, n);
        int click = (i < ms_samples(8)) ? noise_signed() / 3 : 0;
        int tone = triangle_sample(&phase, 1250u, 6500);

        v->samples[i] =
            clamp16(scaled((int)((click + tone) * e), 0.72f));
    }

    return n;
}


/* BF21: resynthesise the approved 42 ms key press, then reverse every
 * sample in memory. No stored asset or synthetic surrogate changes its timbre. */
static unsigned make_terminal_backspace(F144Voice *v)
{
    unsigned n=make_terminal_key(v);
    unsigned i;
    for(i=0U;i<n/2U;++i)
    {
        int16_t sample=v->samples[i];
        v->samples[i]=v->samples[n-1U-i];
        v->samples[n-1U-i]=sample;
    }
    return n;
}

static unsigned make_terminal_ok(F144Voice *v)
{
    unsigned n1 = ms_samples(70);
    unsigned gap = ms_samples(25);
    unsigned n2 = ms_samples(85);
    unsigned total = n1 + gap + n2;
    unsigned i;
    uint32_t p1 = 0u, p2 = 0u;

    for (i = 0; i < n1; ++i) {
        float e = env_linear(i, n1);
        v->samples[i] =
            clamp16(scaled(
                triangle_sample(&p1, 660u, 10500),
                0.78f * e));
    }

    for (i = 0; i < n2; ++i) {
        float e = env_linear(i, n2);
        v->samples[n1 + gap + i] =
            clamp16(scaled(
                triangle_sample(&p2, 880u, 11000),
                0.80f * e));
    }

    return total;
}

static unsigned make_terminal_error(F144Voice *v)
{
    unsigned b1 = ms_samples(85);
    unsigned gap = ms_samples(45);
    unsigned b2 = ms_samples(95);
    unsigned total = b1 + gap + b2;
    unsigned i;
    uint32_t p1 = 0u, p2 = 0u;

    for (i = 0; i < b1; ++i) {
        int tone = square_sample(&p1, 185u, 10500);
        int grit = noise_signed() / 14;
        v->samples[i] =
            clamp16(scaled(tone + grit, 0.82f));
    }

    for (i = 0; i < b2; ++i) {
        int tone = square_sample(&p2, 165u, 11500);
        int grit = noise_signed() / 13;
        v->samples[b1 + gap + i] =
            clamp16(scaled(tone + grit, 0.86f));
    }

    return total;
}





static unsigned make_relay(F144Voice *v)
{
    unsigned first = ms_samples(28);
    unsigned gap = ms_samples(42);
    unsigned second = ms_samples(34);
    unsigned total = first + gap + second;
    unsigned i;
    uint32_t phase = 0u;

    for (i = 0; i < first; ++i) {
        float e = env_linear(i, first);
        int s =
            noise_signed() / 4 +
            triangle_sample(&phase, 1850u, 7500);

        v->samples[i] =
            clamp16(scaled(s, 0.76f * e));
    }

    phase = 0u;

    for (i = 0; i < second; ++i) {
        float e = env_linear(i, second);
        int s =
            noise_signed() / 5 +
            triangle_sample(&phase, 1450u, 7200);

        v->samples[first + gap + i] =
            clamp16(scaled(s, 0.72f * e));
    }

    return total;
}

/*
    Rubber stamp:
      - short handle/mechanism descent
      - fat rubber-paper impact
      - subtle desk resonance
      - tiny lift-off noise at the end

    Distinct from stapler:
      softer high-frequency content,
      fuller low-mid impact,
      slightly longer "body".
*/


/*
    Filing cabinet:
      - small handle / latch click
      - metal drawer slides outward
      - low cabinet resonance
      - definite stop-clunk at the end

    Intended to sound like painted steel furniture, not a wooden drawer.
*/
static unsigned make_filing_cabinet(F144Voice *v)
{
    unsigned n = ms_samples(620), i;
    uint32_t p_slide = 0u;
    uint32_t p_body = 0u;
    uint32_t p_stop = 0u;

    unsigned latch_end = ms_samples(35);
    unsigned slide_start = ms_samples(30);
    unsigned stop_start = ms_samples(500);

    if (n > F144_MAX_SAMPLES)
        n = F144_MAX_SAMPLES;

    for (i = 0; i < n; ++i) {
        int s = 0;

        /* Handle/latch click. */
        if (i < latch_end) {
            float e = env_linear(i, latch_end);
            s += (int)(triangle_sample(&p_stop, 1450u, 6500) * e);
            s += (int)((noise_signed() / 9) * e);
        }

        /* Drawer runners: broad filtered-ish scrape plus low metallic body. */
        if (i >= slide_start && i < stop_start) {
            unsigned j = i - slide_start;
            unsigned len = stop_start - slide_start;
            float e = env_attack_release(j, len, ms_samples(70));

            int scrape = noise_signed() / 11;
            int rail = triangle_sample(&p_slide, 235u, 2600);
            int cabinet = triangle_sample(&p_body, 84u, 2100);

            /* Gentle irregularity in the runner noise. */
            if ((j / 180u) & 1u)
                scrape = (scrape * 3) / 4;

            s += (int)((scrape + rail + cabinet) * e);
        }

        /* Drawer hits its stop with a proper painted-steel clunk. */
        if (i >= stop_start && i < stop_start + ms_samples(90)) {
            unsigned j = i - stop_start;
            float e = env_linear(j, ms_samples(90));

            s += (int)(triangle_sample(&p_stop, 78u, 15000) * e);
            s += (int)((noise_signed() / 8) * e);
        }

        v->samples[i] = clamp16(scaled(s, 0.82f));
    }

    return n;
}

/*
    Institutional office door:
      - latch click
      - short hinge/body movement
      - heavy low door thud
      - tiny latch settle

    It is intentionally not a horror-film creak.
*/


/* CRT / terminal wake-up: relay click, chirp, fading whine. */
static unsigned make_crt_wake(F144Voice *v)
{
    unsigned n = ms_samples(620), i;
    uint32_t p_click = 0u, p_chirp = 0u, p_whine = 0u;
    unsigned chirp_start = ms_samples(36);
    unsigned whine_start = ms_samples(95);

    for (i = 0; i < n; ++i) {
        int s = 0;

        if (i < ms_samples(32)) {
            float e = env_linear(i, ms_samples(32));
            s += (int)(triangle_sample(&p_click, 1500u, 6200) * e);
            s += (int)((noise_signed() / 10) * e);
        }

        if (i >= chirp_start && i < chirp_start + ms_samples(90)) {
            unsigned j = i - chirp_start;
            unsigned len = ms_samples(90);
            unsigned freq = 520u + (unsigned)((900u * (uint64_t)j) / len);
            float e = env_linear(j, len);
            s += (int)(triangle_sample(&p_chirp, freq, 5200) * e);
        }

        if (i >= whine_start) {
            unsigned j = i - whine_start;
            unsigned len = n - whine_start;
            float e = env_linear(j, len);
            s += (int)(triangle_sample(&p_whine, 3850u, 2600) * e);
        }

        v->samples[i] = clamp16(scaled(s, 0.74f));
    }
    return n;
}

/* Old office telephone: classic mechanical double ring. */

static unsigned make_power_fail(F144Voice *v)
{
    unsigned n = ms_samples(1350), i;
    uint32_t p_relay = 0u, p_hum = 0u, p_click = 0u;
    unsigned collapse_start = ms_samples(220);
    unsigned click_start = ms_samples(1220);

    for (i = 0; i < n; ++i) {
        int s = 0;

        if (i < ms_samples(145)) {
            unsigned period = ms_samples(48);
            unsigned burst = ms_samples(17);
            if ((i % period) < burst) {
                float e = env_linear(i % period, burst);
                s += (int)(triangle_sample(&p_relay, 1250u, 5200) * e);
                s += (int)((noise_signed() / 12) * e);
            }
        }

        if (i >= ms_samples(70) && i < click_start) {
            unsigned freq;
            float gain;
            if (i < collapse_start) {
                freq = 118u; gain = 1.0f;
            } else {
                unsigned j = i - collapse_start;
                unsigned len = click_start - collapse_start;
                freq = 118u - (unsigned)((84u * (uint64_t)j) / len);
                gain = 1.0f - ((float)j / (float)len);
            }
            s += (int)(triangle_sample(&p_hum, freq, 7800) * gain);
            if (i >= collapse_start)
                s += (int)((noise_signed() / 22) * gain);
        }

        if (i >= click_start && i < click_start + ms_samples(55)) {
            unsigned j = i - click_start;
            float e = env_linear(j, ms_samples(55));
            s += (int)(triangle_sample(&p_click, 410u, 5000) * e);
            s += (int)((noise_signed() / 14) * e);
        }

        v->samples[i] = clamp16(scaled(s, 0.82f));
    }
    return n;
}

/* Original F144SFX v0.9 core above is intentionally unmodified.
   Additional gameplay cues are small synthesis variations, not stored data. */


#ifdef F144_SFX_REVIEW
static unsigned make_footstep_original(F144Voice *v)
{
    unsigned n=ms_samples(108),i;
    uint32_t low=0u;
    unsigned offset=(g_noise>>19u)&15u; /* deterministic variation */
    for(i=0u;i<n;i++) {
        float e=env_linear(i,n);
        int texture=noise_signed()/12;
        int impact=triangle_sample(&low,78u+offset,3000);
        v->samples[i]=clamp16(scaled((int)((texture+impact)*e),0.46f));
    }
    return n;
}
#endif /* F144_SFX_REVIEW: original footstep remains harness-only. */

/*
 * BF24: approved Sound Laboratory "Revised A" footfalls.
 * Port of the browser audition's 11,025 Hz rounded heel + quiet sole profile.
 * Reference audio is NOT embedded. Two subtly different shoes alternate at
 * successful playback, independently of collision/movement gating in main.c.
 */
static unsigned make_footstep_approved(F144Voice *v,unsigned side)
{
    const float heel_high_alpha=0.3213f,heel_low_alpha=0.03636f;
    const float sole_high_alpha=0.4344f,sole_low_alpha=0.05270f;
    const float shuffle_alpha=0.2262f;
    const double tau=6.2831853071795864769;
    unsigned n=ms_samples(175U),i;
    double shift=side?1.032:0.978;
    float level=side?1.045f:0.965f;
    double delay=side?0.084:0.078;
    float heel_hi=0.f,heel_lo=0.f,sole_hi=0.f,sole_lo=0.f,shuffle=0.f;
    for(i=0U;i<n;i++)
    {
        double t=(double)i/(double)F144_RATE;
        float raw=(float)noise_signed()/32768.0f;
        float heel,body,floor_body,signal;
        heel_hi+=heel_high_alpha*(raw-heel_hi);
        heel_lo+=heel_low_alpha*(raw-heel_lo);
        heel=(float)((1.0-exp(-t/0.0036))*exp(-t/0.027));
        body=(float)(0.78*sin(tau*131.0*shift*t+0.6)*exp(-t/0.038));
        floor_body=(float)(0.30*sin(tau*350.0*shift*t+0.4)*exp(-t/0.020));
        signal=heel*(1.25f*(heel_hi-heel_lo)+body+floor_body);
        if(t>=delay)
        {
            double u=t-delay;
            float raw_sole=(float)noise_signed()/32768.0f;
            float sole,sole_body;
            sole_hi+=sole_high_alpha*(raw_sole-sole_hi);
            sole_lo+=sole_low_alpha*(raw_sole-sole_lo);
            sole=(float)((1.0-exp(-u/0.003))*exp(-u/0.022));
            sole_body=(float)(0.17*sin(tau*183.0*shift*u+0.7)*exp(-u/0.028));
            signal+=0.38f*sole*((sole_hi-sole_lo)+sole_body);
        }
        raw=(float)noise_signed()/32768.0f;
        shuffle+=shuffle_alpha*(raw-shuffle);
        signal+=0.10f*shuffle*(float)((1.0-exp(-t/0.011))*exp(-t/0.057));
        v->samples[i]=clamp16(scaled((int)(signal*32767.0f),0.16f*level));
    }
    return n;
}

static unsigned make_file_restore(F144Voice *v)
{
    unsigned n=ms_samples(620),i,relay=ms_samples(65);
    uint32_t phase=0u,bell=0u;
    for(i=0u;i<n;i++) {
        int sample=0;
        if(i<relay) sample+=(int)(noise_signed()/9*env_linear(i,relay));
        if(i>=relay) {
            unsigned j=i-relay;
            unsigned stage=(j*4u)/(n-relay);
            unsigned tone=294u+stage*49u;
            float e=env_linear(j,n-relay);
            sample+=(int)(triangle_sample(&phase,tone,8000)*e);
            if(stage>=3u) sample+=(int)(triangle_sample(&bell,587u,4400)*e);
        }
        v->samples[i]=clamp16(scaled(sample,0.68f));
    }
    return n;
}

static unsigned make_door_blocked(F144Voice *v)
{
    unsigned n=ms_samples(205),i;
    uint32_t body=0u,latch=0u;
    for(i=0u;i<n;i++) {
        float e=env_linear(i,n);
        int sample=triangle_sample(&body,68u,15500);
        if(i<ms_samples(45)) sample+=noise_signed()/8;
        if(i>=ms_samples(80) && i<ms_samples(140))
            sample+=triangle_sample(&latch,510u,3600);
        v->samples[i]=clamp16(scaled((int)(sample*e),0.66f));
    }
    return n;
}

static unsigned make_evidence_found(F144Voice *v)
{
    unsigned n=ms_samples(680),i,p=0u;
    uint32_t melody_phase=0u,body_phase=0u;
    static const unsigned notes[5]={392u,440u,523u,587u,659u};
    for(i=0u;i<n;i++) {
        unsigned idx=(i*5u)/n;
        unsigned pos=(i*5u)%n;
        float e=env_linear(pos,n);
        int sample=triangle_sample(&melody_phase,notes[idx],8000);
        if(idx==4u) sample+=triangle_sample(&body_phase,330u,3400);
        v->samples[i]=clamp16(scaled((int)(sample*e),0.71f));
    }
    (void)p;
    return n;
}

static unsigned make_door_open(F144Voice *v)
{
    unsigned n=ms_samples(380),i,click=ms_samples(42);
    uint32_t hinge=0u,metal=0u;
    for(i=0u;i<n;i++) {
        float e=env_linear(i,n);
        int sample=triangle_sample(&hinge,84u,3800)+noise_signed()/31;
        if(i<click) sample+=triangle_sample(&metal,1100u,6800);
        v->samples[i]=clamp16(scaled((int)(sample*e),0.75f));
    }
    return n;
}



static unsigned make_floppy_insert(F144Voice *v)
{
    unsigned n=ms_samples(325),i,click=ms_samples(30);
    uint32_t rail=0u,clunk=0u;
    for(i=0u;i<n;i++) {
        float e=env_linear(i,n);
        int sample=triangle_sample(&rail,190u,2900)+noise_signed()/22;
        if(i<click || i>ms_samples(265))
            sample+=triangle_sample(&clunk,710u,5700);
        v->samples[i]=clamp16(scaled((int)(sample*e),0.80f));
    }
    return n;
}

static unsigned make_restore_tick(F144Voice *v,unsigned freq)
{
    unsigned i,n=ms_samples(14U);
    uint32_t phase=0U;
    int filtered=0;
    for(i=0U;i<n;i++) {
        float e=env_attack_release(i,n,ms_samples(2U));
        filtered+=(noise_signed()-filtered)/5;
        v->samples[i]=clamp16(scaled((int)((filtered/10+
            triangle_sample(&phase,freq,2400))*e),0.45f));
    }
    return n;
}

static unsigned generate(F144Voice *v, F144SFX effect)
{
    clear_voice(v);

    switch (effect) {
        case F144_SFX_RESTORE_TICK: return make_restore_tick(v,320U);
        case F144_SFX_RESTORE_TICK_HIGH: return make_restore_tick(v,380U);
        case F144_SFX_TERMINAL_KEY:
            return make_terminal_key(v);
        case F144_SFX_TERMINAL_BACKSPACE:
            return make_terminal_backspace(v);

        case F144_SFX_TERMINAL_OK:
            return make_terminal_ok(v);

        case F144_SFX_TERMINAL_ERROR:
            return make_terminal_error(v);





        case F144_SFX_RELAY:
            return make_relay(v);


        case F144_SFX_FILING_CABINET:
            return make_filing_cabinet(v);


        case F144_SFX_CRT_WAKE:
            return make_crt_wake(v);


        case F144_SFX_POWER_FAIL:
            return make_power_fail(v);

        case F144_SFX_FOOTSTEP: return make_footstep_approved(v,g_footstep_side);
        case F144_SFX_FILE_RESTORE: return make_file_restore(v);
        case F144_SFX_DOOR_BLOCKED: return make_door_blocked(v);
        case F144_SFX_EVIDENCE_FOUND: return make_evidence_found(v);
        case F144_SFX_DOOR_OPEN: return make_door_open(v);
        case F144_SFX_FLOPPY_INSERT: return make_floppy_insert(v);

        default:
            return 0u;
    }
}

#ifdef F144_SFX_REVIEW
#include "f144sfx_review.inc"
#endif

static F144Voice *find_voice(void)
{
    unsigned i;

    for (i = 0; i < F144_VOICES; ++i) {
        F144Voice *v = &g_voice[i];

        if (!v->prepared)
            return v;

        if ((v->hdr.dwFlags & WHDR_DONE) != 0u) {
            waveOutUnprepareHeader(
                g_wave,
                &v->hdr,
                sizeof(v->hdr));

            memset(&v->hdr, 0, sizeof(v->hdr));
            v->prepared = 0;

            return v;
        }
    }

    return NULL;
}

int F144_SFXInit(void)
{
    WAVEFORMATEX fmt;
    MMRESULT r;

    if (g_available)
        return 1;

    memset(&fmt, 0, sizeof(fmt));
    memset(g_voice, 0, sizeof(g_voice));
    g_footstep_side = 0u;

    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 1;
    fmt.nSamplesPerSec = F144_RATE;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = 2;
    fmt.nAvgBytesPerSec = F144_RATE * 2u;

    r = waveOutOpen(
        &g_wave,
        WAVE_MAPPER,
        &fmt,
        0,
        0,
        CALLBACK_NULL);

    if (r != MMSYSERR_NOERROR) {
        g_wave = NULL;
        g_available = 0;
        return 0;
    }

    g_available = 1;
    return 1;
}

void F144_SFXShutdown(void)
{
    unsigned i;

    if (!g_available)
        return;

    waveOutReset(g_wave);

    for (i = 0; i < F144_VOICES; ++i) {
        if (g_voice[i].prepared) {
            waveOutUnprepareHeader(
                g_wave,
                &g_voice[i].hdr,
                sizeof(g_voice[i].hdr));

            g_voice[i].prepared = 0;
        }
    }

    waveOutClose(g_wave);
    g_wave = NULL;
    g_available = 0;
}

void F144_SFXPlay(F144SFX effect)
{
    F144Voice *v;
    unsigned samples;
    MMRESULT r;

    if (!g_available)
        return;

    if (effect < 0 || effect >= F144_SFX_COUNT)
        return;

    v = find_voice();

    if (!v)
        return;

#ifdef F144_SFX_REVIEW
    /* Same seed for fair A/B replay; alternate only the two feet. */
    g_noise=0xF14451F1U;
    samples=generate_review(v,effect,g_review_variant,g_review_foot);
    if(effect==F144_SFX_FOOTSTEP)g_review_foot^=1U;
#else
    samples = generate(v, effect);
#endif

    if (!samples)
        return;

    memset(&v->hdr, 0, sizeof(v->hdr));

    v->hdr.lpData = (LPSTR)v->samples;
    v->hdr.dwBufferLength = samples * sizeof(int16_t);

    r = waveOutPrepareHeader(
        g_wave,
        &v->hdr,
        sizeof(v->hdr));

    if (r != MMSYSERR_NOERROR)
        return;

    v->prepared = 1;

    r = waveOutWrite(
        g_wave,
        &v->hdr,
        sizeof(v->hdr));

    if (r != MMSYSERR_NOERROR) {
        waveOutUnprepareHeader(
            g_wave,
            &v->hdr,
            sizeof(v->hdr));

        v->prepared = 0;
    }
    else if(effect == F144_SFX_FOOTSTEP) {
        g_footstep_side ^= 1u;
    }
}

void F144_SFXSetVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    g_volume = volume;
}

float F144_SFXGetVolume(void)
{
    return g_volume;
}

int F144_SFXIsAvailable(void)
{
    return g_available;
}