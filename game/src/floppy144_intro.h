/*
 * BF20: authored four-scene procedural recovery intro.
 * Timings are centralised; drawing and login state are separate.
 */
#pragma once
#include "floppy144_draw.h"
#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_INTRO_DISCOVERY_END_MS  3800U
#define FLOPPY144_INTRO_INSERTION_END_MS  7600U
#define FLOPPY144_INTRO_PROGRAM_END_MS   15000U
#define FLOPPY144_INTRO_DURATION_MS      22800U
#define FLOPPY144_INTRO_DISK_CLICK_MS    10100U
#define FLOPPY144_INTRO_CRT_START_MS     11000U
#define FLOPPY144_INTRO_TEXT_MS_PER_CHAR 26U

typedef enum Floppy144IntroBeat {
    FLOPPY144_INTRO_DISCOVERY=0,
    FLOPPY144_INTRO_INSERTION,
    FLOPPY144_INTRO_PROGRAM_START,
    FLOPPY144_INTRO_GDR_CONNECTION,
    FLOPPY144_INTRO_COMPLETE
} Floppy144IntroBeat;

Floppy144IntroBeat Floppy144IntroBeatAt(uint32_t elapsed_ms);
/* No action may skip the essential boot-to-login transitions. */
bool Floppy144IntroActionSkips(F144Action action);
bool Floppy144IntroLoginReady(uint32_t elapsed_ms);
void Floppy144IntroDraw(Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate);
void Floppy144IntroDrawWithLogin(
    Floppy144Surface *surface,uint32_t elapsed_ms,uint32_t text_rate,
    const char *login_name,bool save_failed
);
