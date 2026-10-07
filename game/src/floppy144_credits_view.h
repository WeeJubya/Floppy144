/*
 * FLOPPY//144 credits and runtime-attribution screen.
 */
#pragma once
#include "floppy144_draw.h"
#include <stdint.h>

void Floppy144CreditsViewDraw(Floppy144Surface *surface);
uint32_t Floppy144CreditsViewLineCount(void);
const char *Floppy144CreditsViewLineAt(uint32_t index);
uint32_t Floppy144CreditsViewMaxLineWidth(void);
