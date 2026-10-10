#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <string.h>
#include "f144audio.h"

#pragma comment(lib, "winmm.lib")

/*
    F144MIDI v0.7
    =============
    30-minute Bureaucratic Loop preserving the original F144MIDI v0.7
    orchestration, with a staged final-minute return to the base theme.
    Layers develop every five minutes; corruption adds floor(restoration/3).

    80 BPM, 4/4
    1 bar = 3 seconds
    600 bars = 30 minutes

      Bars  0- 9 : Intro          0:00 - 0:30
      Bars 10-24 : Layer A        0:30 - 1:15
      Bars 25-39 : Layer B        1:15 - 2:00
      Bars 40-89 : Full Loop      2:00 - 4:30
      Bars 90-99 : Outro / Reset  4:30 - 5:00

    v0.7 outro:
      90-91 : full material starts receding
      92-93 : pad gone, melody + paperwork thinning
      94-95 : melody gone, paperwork thinning
      96-97 : pizzicato remnants only
      98-99 : bare clockwork, itself fading
*/

#define F144_BPM           80.0
#define F144_STEPS_PER_BAR 16u
#define F144_TOTAL_STEPS   9600u
#define F144_STEP_SECONDS  (60.0 / F144_BPM / 4.0)

#define CH_MARIMBA  0u
#define CH_BASS     1u
#define CH_PIZZ     2u
#define CH_CLARINET 3u
#define CH_PAD      4u
#define CH_DRUMS    9u
#define CH_COUNTER  5u
#define CH_EXTRA_PIZZ 6u

enum F144Section {
    SEC_INTRO = 0,
    SEC_LAYER_A,
    SEC_LAYER_B,
    SEC_FULL,
    SEC_OUTRO
};

static HMIDIOUT g_out = NULL;
static int      g_available = 0;
static int      g_playing = 0;
static unsigned g_step = 0;
static double   g_accum = 0.0;

static uint8_t g_last_note[16];
static uint8_t g_note_active[16];

static char g_device_name[128] = "Unavailable";

static float    g_music_volume = 0.85f;
static F144Act  g_act = F144_ACT_PROLOGUE;
static uint32_t g_seed = 0xF1441962u;
static unsigned g_restoration = 0u;
static uint8_t g_distortion = 0u;

static const uint8_t g_channel_volume[16] = {
    118,112,102,108,92,100,100,100,100,105,100,100,100,100,100,100
};

static uint8_t scale7(uint8_t value, float scale)
{
    float v = (float)value * scale;
    if (v < 0.0f) v = 0.0f;
    if (v > 127.0f) v = 127.0f;
    return (uint8_t)(v + 0.5f);
}

static void send_raw(uint8_t status, uint8_t d1, uint8_t d2)
{
    DWORD msg;

    if (!g_available || !g_out)
        return;

    msg = (DWORD)status |
          ((DWORD)d1 << 8) |
          ((DWORD)d2 << 16);

    midiOutShortMsg(g_out, msg);
}

static void program(uint8_t ch, uint8_t p)
{
    send_raw((uint8_t)(0xC0u | ch), p, 0);
}

static void cc(uint8_t ch, uint8_t controller, uint8_t value)
{
    send_raw((uint8_t)(0xB0u | ch), controller, value);
}

static void apply_master_volume(void)
{
    unsigned ch;

    if (!g_available)
        return;

    for (ch = 0; ch < 16u; ++ch)
        cc((uint8_t)ch, 7u,
           scale7(g_channel_volume[ch], g_music_volume));
}

void F144_MusicSetVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    g_music_volume = volume;
    apply_master_volume();
}

float F144_MusicGetVolume(void)
{
    return g_music_volume;
}

void F144_MusicSetAct(F144Act act)
{
    if (act < F144_ACT_PROLOGUE)
        act = F144_ACT_PROLOGUE;

    if (act > F144_ACT_III)
        act = F144_ACT_III;

    g_act = act;
}

F144Act F144_MusicGetAct(void)
{
    return g_act;
}

void F144_MusicSetSeed(uint32_t seed)
{
    g_seed = seed ? seed : 0xF1441962u;
}

uint32_t F144_MusicGetSeed(void)
{
    return g_seed;
}

void F144_MusicSetRestoration(unsigned percent)
{
    g_restoration = percent;
    g_distortion = (uint8_t)((percent / 3u) > 100u ? 100u : percent / 3u);
}

unsigned F144_MusicGetRestoration(void) { return g_restoration; }
unsigned F144_MusicGetDistortion(void) { return (unsigned)g_distortion; }

/* Deterministic corruption hash. */
static uint32_t hash32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

static uint32_t event_hash(
    uint8_t ch, uint8_t note, unsigned bar, unsigned step)
{
    uint32_t x = g_seed;

    x ^= (uint32_t)ch   * 0x9e3779b9u;
    x ^= (uint32_t)note * 0x85ebca6bu;
    x ^= (uint32_t)bar  * 0xc2b2ae35u;
    x ^= (uint32_t)step * 0x27d4eb2du;

    return hash32(x);
}

static unsigned pitch_chance(uint8_t ch)
{
    switch (g_act) {
        case F144_ACT_PROLOGUE:
            return 0u;

        case F144_ACT_I:
            return (ch == CH_CLARINET) ? 55u : 0u;

        case F144_ACT_II:
            if (ch == CH_CLARINET) return 120u;
            if (ch == CH_PIZZ)     return 45u;
            if (ch == CH_BASS)     return 25u;
            return 0u;

        case F144_ACT_III:
            if (ch == CH_CLARINET) return 230u;
            if (ch == CH_PIZZ)     return 115u;
            if (ch == CH_BASS)     return 80u;
            if (ch == CH_MARIMBA)  return 60u;
            return 0u;
    }

    return 0u;
}

static int corrupt_note(
    uint8_t ch, uint8_t *note, unsigned bar, unsigned step)
{
    uint32_t h;
    unsigned chance;
    unsigned mode;
    int n;

    if (ch == CH_DRUMS)
        return 1;

    h = event_hash(ch, *note, bar, step);
    chance = pitch_chance(ch);
    /* Restoration increases distortion by floor(percent/3), independent
       of Act-specific baseline corruption. Prologue remains clean at 0%. */
    chance += (unsigned)g_distortion * 6u;
    if (chance > 930u) chance = 930u;

    if ((h % 1000u) >= chance)
        return 1;

    mode = (h >> 16) % 100u;
    n = *note;

    if (g_act == F144_ACT_PROLOGUE || g_act == F144_ACT_I) {
        n += (mode & 1u) ? 1 : -1;
    }
    else if (g_act == F144_ACT_II) {
        if (mode < 85u)
            n += (mode & 1u) ? 1 : -1;
        else if (mode < 95u)
            n += (mode & 1u) ? 2 : -2;
        else
            n += (mode & 1u) ? 12 : -12;
    }
    else {
        if (ch == CH_BASS && mode >= 78u)
            n += 6;
        else if (mode < 60u)
            n += (mode & 1u) ? 1 : -1;
        else if (mode < 82u)
            n += (mode & 1u) ? 2 : -2;
        else if (mode < 92u)
            n += (mode & 1u) ? 6 : -6;
        else
            n += (mode & 1u) ? 12 : -12;
    }

    if (n < 0)   n = 0;
    if (n > 127) n = 127;

    *note = (uint8_t)n;
    return 1;
}

static void note_off(uint8_t ch, uint8_t note)
{
    send_raw((uint8_t)(0x80u | ch), note, 0);
}

static void mono_note(
    uint8_t ch,
    uint8_t note,
    uint8_t velocity,
    float gain,
    unsigned bar,
    unsigned step)
{
    if (g_note_active[ch])
        note_off(ch, g_last_note[ch]);

    if (!corrupt_note(ch, &note, bar, step)) {
        g_note_active[ch] = 0;
        return;
    }

    velocity = scale7(velocity, 1.35f * gain);

    if (velocity == 0u) {
        g_note_active[ch] = 0;
        return;
    }

    send_raw((uint8_t)(0x90u | ch), note, velocity);

    g_last_note[ch] = note;
    g_note_active[ch] = 1;
}

static void mono_silence(uint8_t ch)
{
    if (g_note_active[ch]) {
        note_off(ch, g_last_note[ch]);
        g_note_active[ch] = 0;
    }
}

static void channel_silence(uint8_t ch)
{
    cc(ch, 123u, 0u);
    g_note_active[ch] = 0;
}

static void all_notes_off(void)
{
    unsigned ch;

    for (ch = 0; ch < 16u; ++ch) {
        cc((uint8_t)ch, 123u, 0u);
        g_note_active[ch] = 0;
    }
}

static enum F144Section section_for_bar(unsigned bar)
{
    if (bar >= 580u) return SEC_OUTRO;
    if (bar < 10u) return SEC_INTRO;
    if (bar < 25u) return SEC_LAYER_A;
    if (bar < 40u) return SEC_LAYER_B;
    return SEC_FULL;
}

/* One new secondary layer joins at each five-minute boundary. */
static unsigned layers_for_bar(unsigned bar)
{
    if (bar >= 580u) return 0u;
    return bar / 100u;
}

static uint8_t root_for_bar(unsigned bar)
{
    return ((bar / 2u) & 1u) ? 47u : 45u;
}

/* Clockwork remains the spine of the whole cue.
   It only starts fading during the final four bars. */
static float clockwork_gain(unsigned bar)
{
    if (bar < 592u) return 1.00f;
    return 0.9f - 0.08f * (float)(bar - 592u);
}

static void clockwork(unsigned bar, unsigned step)
{
    uint8_t root = root_for_bar(bar);
    uint8_t fifth = (uint8_t)(root + 7u);
    uint8_t mnote;
    float gain = clockwork_gain(bar);

    /* Final bar is deliberately sparse to leave a breath before restart. */
    if (bar == 599u && step >= 8u) {
        if (step == 8u) {
            mono_silence(CH_MARIMBA);
            mono_silence(CH_BASS);
        }
        return;
    }

    if ((step & 3u) == 0u) {
        switch (step >> 2) {
            case 0:
                mnote = (uint8_t)(root + 12u);
                break;
            case 1:
                mnote = (uint8_t)(fifth + 12u);
                break;
            case 2:
                mnote = (uint8_t)(root + 12u);
                break;
            default:
                mnote = (uint8_t)(fifth + 12u);
                break;
        }

        mono_note(
            CH_MARIMBA,
            mnote,
            (uint8_t)((step == 0u) ? 42u : 34u),
            gain,
            bar,
            step);
    }

    if ((step & 3u) == 2u)
        mono_silence(CH_MARIMBA);

    if (step == 0u)
        mono_note(CH_BASS, root, 42u, gain, bar, step);

    if (step == 8u)
        mono_note(CH_BASS, fifth, 34u, gain, bar, step);

    if (step == 6u || step == 14u)
        mono_silence(CH_BASS);
}

static void paperwork(
    unsigned bar,
    unsigned step,
    int fuller,
    float gain,
    int include_percussion)
{
    uint8_t root = root_for_bar(bar);
    uint8_t fifth = (uint8_t)(root + 7u);

    if (step == 2u || step == 10u)
        mono_note(
            CH_PIZZ,
            (uint8_t)(root + 12u),
            fuller ? 31u : 25u,
            gain,
            bar,
            step);

    if (step == 6u || step == 14u)
        mono_note(
            CH_PIZZ,
            (uint8_t)(fifth + 12u),
            fuller ? 29u : 23u,
            gain,
            bar,
            step);

    if (step == 3u || step == 7u ||
        step == 11u || step == 15u)
        mono_silence(CH_PIZZ);

    if (!include_percussion)
        return;

    if (step == 4u || step == 5u || step == 6u ||
        step == 12u || step == 13u) {

        uint8_t velocity =
            scale7(fuller ? 32u : 23u, gain);

        if (velocity)
            send_raw(
                (uint8_t)(0x90u | CH_DRUMS),
                37u,
                velocity);
    }

    if ((bar & 3u) == 3u && step == 14u) {
        uint8_t velocity =
            scale7(fuller ? 42u : 31u, gain);

        if (velocity)
            send_raw(
                (uint8_t)(0x90u | CH_DRUMS),
                76u,
                velocity);
    }
}

static void melody(
    unsigned bar,
    unsigned step,
    int full,
    float gain)
{
    static const uint8_t am[8] =
        {69,72,76,71,72,69,67,64};

    static const uint8_t bm[8] =
        {71,74,78,73,74,71,69,66};

    const uint8_t *notes =
        ((bar / 2u) & 1u) ? bm : am;

    unsigned phrase_bar = bar & 7u;
    unsigned index;

    if (!full &&
        (phrase_bar == 3u || phrase_bar == 7u))
        return;

    if (step == 0u || step == 6u || step == 10u) {
        index =
            (phrase_bar * 2u +
             (step == 0u ? 0u :
              step == 6u ? 1u : 2u)) & 7u;

        mono_note(
            CH_CLARINET,
            notes[index],
            full ? 37u : 30u,
            gain,
            bar,
            step);
    }

    if (step == 4u || step == 9u || step == 14u)
        mono_silence(CH_CLARINET);
}

/* Pad is deliberately stable and quiet. */
static void pad(unsigned bar, unsigned step, float gain)
{
    uint8_t root;

    if (step != 0u)
        return;

    channel_silence(CH_PAD);

    root = (uint8_t)(root_for_bar(bar) + 12u);

    send_raw(
        (uint8_t)(0x90u | CH_PAD),
        root,
        scale7(30u, gain));

    send_raw(
        (uint8_t)(0x90u | CH_PAD),
        (uint8_t)(root + 3u),
        scale7(24u, gain));

    send_raw(
        (uint8_t)(0x90u | CH_PAD),
        (uint8_t)(root + 7u),
        scale7(22u, gain));

    if (bar >= 40u &&
        (((bar - 40u) & 3u) == 3u)) {

        send_raw(
            (uint8_t)(0x90u | CH_PAD),
            (uint8_t)(root + 1u),
            scale7(14u, gain));
    }
}

/*
    The v0.7 staged reset.

    There is deliberately no single "drop everything" moment.
*/
static void outro(unsigned bar, unsigned step)
{
    if (bar==580u && step==0u) {
        channel_silence(CH_COUNTER);
        channel_silence(CH_EXTRA_PIZZ);
    }
    if (bar < 584u) {
        paperwork(bar,step,1,0.78f,1);
        melody(bar,step,1,0.70f);
        pad(bar,step,0.45f);
    } else if (bar < 588u) {
        if (bar==584u && step==0u) channel_silence(CH_PAD);
        paperwork(bar,step,0,0.60f,1);
        melody(bar,step,0,0.50f);
    } else if (bar < 592u) {
        if (bar==588u && step==0u) channel_silence(CH_CLARINET);
        paperwork(bar,step,0,0.42f,1);
    } else if (bar < 596u) {
        paperwork(bar,step,0,0.24f,0);
    } else if (bar==596u && step==0u) {
        channel_silence(CH_PIZZ);
    }
}

/* Five progressively richer additions atop the approved v0.7 notes.
   They deliberately reuse its GM voices and stay below the main melody. */
static void evolving_layers(unsigned bar,unsigned step)
{
    unsigned layers=layers_for_bar(bar);
    uint8_t root=root_for_bar(bar);
    if (layers>=1u && (step==2u || step==10u))
        send_raw(0x99u,42u,14u);  /* stationery rustle / brush */
    if (layers>=2u && (bar%4u)==1u && step==4u)
        mono_note(CH_EXTRA_PIZZ,(uint8_t)(root+19u),24u,0.50f,bar,step);
    if (layers>=3u && (bar%8u)==6u && step==8u)
        mono_note(CH_COUNTER,(uint8_t)(root+24u),23u,0.45f,bar,step);
    if (layers>=4u && (bar%8u)==4u && step==0u)
        mono_note(CH_COUNTER,(uint8_t)(root+19u),26u,0.45f,bar,step);
    if (step==6u || step==14u) mono_silence(CH_EXTRA_PIZZ);
    if (step==12u) mono_silence(CH_COUNTER);
    if (layers>=5u && (bar%4u)==2u && step==12u)
        send_raw(0x99u,76u,19u); /* occasional soft office woodblock */
}

static void process_step(unsigned absolute_step)
{
    unsigned bar =
        absolute_step / F144_STEPS_PER_BAR;

    unsigned step =
        absolute_step % F144_STEPS_PER_BAR;

    enum F144Section sec =
        section_for_bar(bar);

    clockwork(bar, step);

    if (sec == SEC_LAYER_A) {
        paperwork(bar, step, 0, 1.0f, 1);
    }
    else if (sec == SEC_LAYER_B) {
        paperwork(bar, step, 0, 1.0f, 1);
        melody(bar, step, 0, 1.0f);
    }
    else if (sec == SEC_FULL) {
        paperwork(bar, step, 1, 1.0f, 1);
        melody(bar, step, 1, 1.0f);
        pad(bar, step, 1.0f);
    }
    else if (sec == SEC_OUTRO) {
        outro(bar, step);
    }
    if (sec != SEC_OUTRO && sec != SEC_INTRO)
        evolving_layers(bar,step);
}

int F144_AudioInit(void)
{
    MMRESULT r;
    MIDIOUTCAPSA caps;
    UINT id = 0u;

    if (g_available)
        return 1;

    r = midiOutOpen(
        &g_out,
        MIDI_MAPPER,
        0,
        0,
        CALLBACK_NULL);

    if (r != MMSYSERR_NOERROR) {
        strcpy_s(
            g_device_name,
            sizeof(g_device_name),
            "MIDI_MAPPER open failed");

        return 0;
    }

    g_available = 1;

    if (midiOutGetID(g_out, &id) == MMSYSERR_NOERROR &&
        midiOutGetDevCapsA(
            id, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {

        strncpy_s(
            g_device_name,
            sizeof(g_device_name),
            caps.szPname,
            _TRUNCATE);
    }
    else {
        strcpy_s(
            g_device_name,
            sizeof(g_device_name),
            "MIDI_MAPPER");
    }

    program(CH_MARIMBA, 12u);
    program(CH_BASS, 32u);
    program(CH_PIZZ, 45u);
    program(CH_CLARINET, 71u);
    program(CH_PAD, 48u);
    program(CH_COUNTER, 71u);
    program(CH_EXTRA_PIZZ, 45u);

    apply_master_volume();

    return 1;
}

void F144_AudioShutdown(void)
{
    if (!g_available)
        return;

    F144_MusicStop();
    midiOutReset(g_out);
    midiOutClose(g_out);

    g_out = NULL;
    g_available = 0;
}

void F144_MusicRestart(void)
{
    if (!g_available)
        return;

    all_notes_off();
    apply_master_volume();

    g_step = 0u;
    g_accum = 0.0;
    g_playing = 1;

    process_step(0u);
}

void F144_MusicSeekBar(unsigned bar)
{
    if (!g_available) return;
    all_notes_off();
    g_step = (bar % 600u) * F144_STEPS_PER_BAR;
    g_accum = 0.0;
    g_playing = 1;
    process_step(g_step);
}

void F144_MusicStop(void)
{
    g_playing = 0;
    all_notes_off();
}

void F144_AudioUpdate(double dt_seconds)
{
    if (!g_available || !g_playing)
        return;

    if (dt_seconds < 0.0)
        return;

    if (dt_seconds > 0.5)
        dt_seconds = 0.5;

    g_accum += dt_seconds;

    while (g_accum >= F144_STEP_SECONDS) {
        g_accum -= F144_STEP_SECONDS;

        ++g_step;

        if (g_step >= F144_TOTAL_STEPS) {
            all_notes_off();
            apply_master_volume();
            g_step = 0u;
        }

        process_step(g_step);
    }
}

int F144_AudioIsAvailable(void)
{
    return g_available;
}

unsigned F144_AudioDeviceCount(void)
{
    return (unsigned)midiOutGetNumDevs();
}

const char *F144_AudioDeviceName(void)
{
    return g_device_name;
}