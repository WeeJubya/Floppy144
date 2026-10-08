/*
 * FLOPPY//144 credits and runtime-attribution screen.
 */
#pragma once
#include "floppy144_draw.h"
#include <stdbool.h>
#include <stdint.h>

const char *Floppy144CreditsViewFooter(bool intro_replay_available);
void Floppy144CreditsViewDraw(Floppy144Surface *surface,bool intro_replay_available);
uint32_t Floppy144CreditsViewLineCount(void);
const char *Floppy144CreditsViewLineAt(uint32_t index);
uint32_t Floppy144CreditsViewMaxLineWidth(void);
