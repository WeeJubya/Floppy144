#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <string.h>
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

static unsigned make_typewriter(F144Voice *v)
{
    unsigned n = ms_samples(55), i;
    uint32_t phase = 0u;

    for (i = 0; i < n; ++i) {
        float e = env_linear(i, n);
        int body = noise_signed() / 3;
        int metal = triangle_sample(&phase, 2100u, 7000);

        if (i > ms_samples(18)) {
            body /= 3;
            metal /= 2;
        }

        v->samples[i] =
            clamp16(scaled((int)((body + metal) * e), 0.88f));
    }

    return n;
}

static unsigned make_stapler(F144Voice *v)
{
    unsigned n = ms_samples(145), i;
    uint32_t p_low = 0u, p_mid = 0u, p_hi = 0u;

    for (i = 0; i < n; ++i) {
        int s = 0;

        if (i < ms_samples(55)) {
            float e = env_linear(i, ms_samples(55));
            s += (int)(triangle_sample(&p_low, 72u, 17500) * e);
            s += (int)((noise_signed() / 7) * e);
        }

        if (i >= ms_samples(16) && i < ms_samples(42)) {
            unsigned j = i - ms_samples(16);
            float e = env_linear(j, ms_samples(26));
            s += (int)(triangle_sample(&p_hi, 2350u, 10500) * e);
        }

        if (i >= ms_samples(32)) {
            unsigned j = i - ms_samples(32);
            unsigned rem = n - ms_samples(32);
            float e = env_linear(j, rem);
            s += (int)(triangle_sample(&p_mid, 118u, 7000) * e);
        }

        v->samples[i] = clamp16(scaled(s, 0.98f));
    }

    return n;
}

static unsigned make_printer(F144Voice *v)
{
    unsigned n = ms_samples(1100), i;
    uint32_t motor = 0u, tick = 0u, carriage = 0u;

    if (n > F144_MAX_SAMPLES)
        n = F144_MAX_SAMPLES;

    for (i = 0; i < n; ++i) {
        float global =
            env_attack_release(i, n, ms_samples(55));

        int m = square_sample(&motor, 68u, 2600);
        int hiss = noise_signed() / 15;
        int chatter =
            ((i % 175u) < 28u)
            ? triangle_sample(&tick, 1750u, 7600)
            : 0;

        int c =
            ((i % 3000u) < 120u)
            ? triangle_sample(&carriage, 145u, 5200)
            : 0;

        if (i > n / 2u - ms_samples(50) &&
            i < n / 2u + ms_samples(35)) {
            chatter /= 4;
            m /= 2;
        }

        v->samples[i] =
            clamp16(scaled(
                m + hiss + chatter + c,
                0.72f * (0.40f + 0.60f * global)));
    }

    return n;
}

static unsigned make_paper(F144Voice *v)
{
    unsigned n = ms_samples(380), i;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0;

    for (i = 0; i < n; ++i) {
        int raw = noise_signed();
        int smooth;
        float e, flutter;

        s5 = s4;
        s4 = s3;
        s3 = s2;
        s2 = s1;
        s1 = raw;

        smooth = (s1 + s2 + s3 + s4 + s5) / 5;
        e = env_attack_release(i, n, ms_samples(90));
        flutter = ((i / 170u) & 1u) ? 0.86f : 1.0f;

        v->samples[i] =
            clamp16(scaled(
                smooth,
                0.18f * e * flutter));
    }

    return n;
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
static unsigned make_stamp(F144Voice *v)
{
    unsigned n = ms_samples(180), i;
    uint32_t p_drop = 0u;
    uint32_t p_body = 0u;
    uint32_t p_res = 0u;

    unsigned hit_start = ms_samples(28);
    unsigned hit_len = ms_samples(58);
    unsigned lift_start = ms_samples(125);

    for (i = 0; i < n; ++i) {
        int s = 0;

        /* Handle moving down. */
        if (i < hit_start) {
            float e = env_linear(i, hit_start);
            s += (int)(triangle_sample(&p_drop, 310u, 4200) * e);
            s += noise_signed() / 28;
        }

        /* Main rubber/paper impact. */
        if (i >= hit_start && i < hit_start + hit_len) {
            unsigned j = i - hit_start;
            float e = env_linear(j, hit_len);

            s += (int)(triangle_sample(&p_body, 92u, 17500) * e);
            s += (int)((noise_signed() / 9) * e);
        }

        /* Desk resonance after the stamp lands. */
        if (i >= hit_start + ms_samples(18) &&
            i < lift_start) {

            unsigned j =
                i - (hit_start + ms_samples(18));

            unsigned rem =
                lift_start - (hit_start + ms_samples(18));

            float e = env_linear(j, rem);

            s += (int)(
                triangle_sample(&p_res, 138u, 6200) * e);
        }

        /* Tiny tacky lift-off at the end. */
        if (i >= lift_start &&
            i < lift_start + ms_samples(22)) {

            unsigned j = i - lift_start;
            float e = env_linear(j, ms_samples(22));

            s += (int)((noise_signed() / 10) * e);
        }

        v->samples[i] =
            clamp16(scaled(s, 0.94f));
    }

    return n;
}


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
static unsigned make_door(F144Voice *v)
{
    unsigned n = ms_samples(470), i;
    uint32_t p_latch = 0u;
    uint32_t p_move = 0u;
    uint32_t p_thud = 0u;
    uint32_t p_room = 0u;

    unsigned move_start = ms_samples(45);
    unsigned thud_start = ms_samples(305);

    for (i = 0; i < n; ++i) {
        int s = 0;

        /* Latch release. */
        if (i < ms_samples(45)) {
            float e = env_linear(i, ms_samples(45));
            s += (int)(triangle_sample(&p_latch, 1200u, 6200) * e);
            s += (int)((noise_signed() / 12) * e);
        }

        /* Short heavy movement, more body than squeak. */
        if (i >= move_start && i < thud_start) {
            unsigned j = i - move_start;
            unsigned len = thud_start - move_start;
            float e = env_attack_release(j, len, ms_samples(65));

            int body = triangle_sample(&p_move, 63u, 3600);
            int texture = noise_signed() / 24;

            s += (int)((body + texture) * e);
        }

        /* Door meets frame. */
        if (i >= thud_start && i < thud_start + ms_samples(105)) {
            unsigned j = i - thud_start;
            float e = env_linear(j, ms_samples(105));

            s += (int)(triangle_sample(&p_thud, 58u, 18500) * e);
            s += (int)(triangle_sample(&p_room, 102u, 5200) * e);
            s += (int)((noise_signed() / 10) * e);
        }

        /* Small latch settle after the main impact. */
        if (i >= thud_start + ms_samples(80) &&
            i < thud_start + ms_samples(120)) {

            unsigned j = i - (thud_start + ms_samples(80));
            float e = env_linear(j, ms_samples(40));

            s += (int)(triangle_sample(&p_latch, 620u, 2800) * e);
        }

        v->samples[i] = clamp16(scaled(s, 0.88f));
    }

    return n;
}


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
static unsigned make_telephone(F144Voice *v)
{
    unsigned n = ms_samples(1120), i;
    uint32_t p_low = 0u;
    uint32_t p_high = 0u;
    uint32_t p_body = 0u;

    /*
        v0.9 telephone:
        lower electromechanical chirrup / warble.

        No continuous pitch sweep.
        Instead, two fixed bell tones alternate rapidly inside
        each chirrup, with a low metallic body underneath.

        Goal: "brrip-brrip", not whistle, not bird.
    */

    unsigned c1_start = 0u;
    unsigned c1_len   = ms_samples(280);
    unsigned c2_start = ms_samples(500);
    unsigned c2_len   = ms_samples(300);

    for (i = 0; i < n; ++i) {
        int s = 0;
        int active = 0;
        unsigned j = 0u;
        unsigned len = 1u;

        if (i >= c1_start && i < c1_start + c1_len) {
            active = 1;
            j = i - c1_start;
            len = c1_len;
        }
        else if (i >= c2_start && i < c2_start + c2_len) {
            active = 1;
            j = i - c2_start;
            len = c2_len;
        }

        if (active) {
            float attack;
            float release;
            float e;
            int tone;
            int body;
            unsigned gate;

            attack =
                (j < ms_samples(14))
                ? (float)j / (float)ms_samples(14)
                : 1.0f;

            release = env_linear(j, len);
            e = attack * (0.62f + 0.38f * release);

            /*
                Alternate the two tones every ~32 ms.
                This creates the chirrup without animal-like glissando.
            */
            gate = (j / ms_samples(32)) & 1u;

            if (gate == 0u)
                tone = triangle_sample(&p_low, 640u, 9000);
            else
                tone = triangle_sample(&p_high, 790u, 8200);

            /* Low metallic body keeps it feeling like a desk phone. */
            body = triangle_sample(&p_body, 165u, 2300);

            s += (int)((tone + body) * e);

            /* Brief striker click at the start of each chirrup. */
            if (j < ms_samples(11)) {
                float ce = env_linear(j, ms_samples(11));
                s += (int)((noise_signed() / 8) * ce);
            }
        }

        v->samples[i] = clamp16(scaled(s, 0.76f));
    }

    return n;
}

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

static unsigned make_footstep(F144Voice *v)
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

static unsigned make_fridge_open(F144Voice *v)
{
    unsigned n=ms_samples(515),i,seal=ms_samples(72);
    uint32_t hum=0u,hinge=0u;
    for(i=0u;i<n;i++) {
        float e=env_linear(i,n);
        int sample=triangle_sample(&hum,77u,2350);
        if(i<seal) sample+=noise_signed()/6;
        if(i>=seal) sample+=triangle_sample(&hinge,235u,2800);
        v->samples[i]=clamp16(scaled((int)(sample*e),0.59f));
    }
    return n;
}

static unsigned make_coffee_machine(F144Voice *v)
{
    unsigned n=ms_samples(1650),i;
    uint32_t motor=0u,bubble=0u;
    for(i=0u;i<n;i++) {
        float e=env_attack_release(i,n,ms_samples(160));
        int sample=triangle_sample(&motor,64u,2400)+noise_signed()/27;
        unsigned pulse=((i/330u)+(i/600u))%7u;
        if(pulse<2u) sample+=triangle_sample(&bubble,90u+(i%300u),4800);
        v->samples[i]=clamp16(scaled((int)(sample*e),0.60f));
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

static unsigned generate(F144Voice *v, F144SFX effect)
{
    clear_voice(v);

    switch (effect) {
        case F144_SFX_TERMINAL_KEY:
            return make_terminal_key(v);

        case F144_SFX_TERMINAL_OK:
            return make_terminal_ok(v);

        case F144_SFX_TERMINAL_ERROR:
            return make_terminal_error(v);

        case F144_SFX_TYPEWRITER:
            return make_typewriter(v);

        case F144_SFX_STAPLER:
            return make_stapler(v);

        case F144_SFX_PRINTER:
            return make_printer(v);

        case F144_SFX_PAPER:
            return make_paper(v);

        case F144_SFX_RELAY:
            return make_relay(v);

        case F144_SFX_STAMP:
            return make_stamp(v);

        case F144_SFX_FILING_CABINET:
            return make_filing_cabinet(v);

        case F144_SFX_DOOR:
            return make_door(v);

        case F144_SFX_CRT_WAKE:
            return make_crt_wake(v);

        case F144_SFX_TELEPHONE:
            return make_telephone(v);

        case F144_SFX_POWER_FAIL:
            return make_power_fail(v);

        case F144_SFX_FOOTSTEP: return make_footstep(v);
        case F144_SFX_FILE_RESTORE: return make_file_restore(v);
        case F144_SFX_DOOR_BLOCKED: return make_door_blocked(v);
        case F144_SFX_EVIDENCE_FOUND: return make_evidence_found(v);
        case F144_SFX_DOOR_OPEN: return make_door_open(v);
        case F144_SFX_FRIDGE_OPEN: return make_fridge_open(v);
        case F144_SFX_COFFEE_MACHINE: return make_coffee_machine(v);
        case F144_SFX_FLOPPY_INSERT: return make_floppy_insert(v);

        default:
            return 0u;
    }
}

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

    samples = generate(v, effect);

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