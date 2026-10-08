#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * S4E-04: immutable flavour text for the Staff Room noticeboard P-330.
 * Three hard-wrapped lines at most 54 characters each (Cabinet detail width).
 * Stateless: the saved recovery seed and stable feature namespaces are all
 * that determine the menu. No date, platform, global PRNG or save fields.
 */
#define FLOPPY144_TAKEAWAY_MENU_CAPACITY 192U
#define FLOPPY144_TAKEAWAY_MAX_LINE_LENGTH 54U

bool Floppy144TakeawayMenuGenerate(
    uint32_t recovery_seed,
    char *output,
    uint32_t capacity
);
