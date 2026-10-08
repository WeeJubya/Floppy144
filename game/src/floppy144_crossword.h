#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_CROSSWORD_GRID_SIZE 5U
#define FLOPPY144_CROSSWORD_VARIANT_COUNT 6U

/*
 * Staff Room P-074: an immutable inspection-only 5x5 crossing.
 * '.' is an unoccupied cell, '_' an unsolved cell, A-Z a pencilled letter.
 * Neither solutions nor fill state are persistent or entered by the player.
 */
typedef struct Floppy144CrosswordView
{
    char grid[FLOPPY144_CROSSWORD_GRID_SIZE][FLOPPY144_CROSSWORD_GRID_SIZE + 1U];
    const char *across_answer;
    const char *down_answer;
    const char *across_clue;
    const char *down_clue;
    const char *annotation;
    uint8_t variant;
} Floppy144CrosswordView;

bool Floppy144CrosswordGenerate(
    uint32_t recovery_seed,
    Floppy144CrosswordView *view
);
