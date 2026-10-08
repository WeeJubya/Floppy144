/*
 * FLOPPY//144 procedural startup intro.
 *
 * The intro is pure presentation: it reads persisted presentation settings,
 * draws into the software framebuffer and maps logical skip actions. It owns
 * no profile, save, recovery or progression state.
 */
#pragma once

#include "floppy144_draw.h"
#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_INTRO_DISCOVERY_END_MS 2600U
#define FLOPPY144_INTRO_INSERTION_END_MS 4800U
#define FLOPPY144_INTRO_PROGRAM_END_MS   7000U
#define FLOPPY144_INTRO_DURATION_MS     10400U

typedef enum Floppy144IntroBeat
{
    FLOPPY144_INTRO_DISCOVERY = 0,
    FLOPPY144_INTRO_INSERTION,
    FLOPPY144_INTRO_PROGRAM_START,
    FLOPPY144_INTRO_GDR_CONNECTION,
    FLOPPY144_INTRO_COMPLETE
} Floppy144IntroBeat;

Floppy144IntroBeat Floppy144IntroBeatAt(
    uint32_t elapsed_ms
);

bool Floppy144IntroActionSkips(
    F144Action action
);

void Floppy144IntroDraw(
    Floppy144Surface *surface,
    uint32_t elapsed_ms,
    uint32_t text_rate
);
