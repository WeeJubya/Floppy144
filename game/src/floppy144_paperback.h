#pragma once

#include <stdbool.h>
#include <stdint.h>

/* The ordinary Cabinet detail panel shows three 54-glyph text lines. */
#define FLOPPY144_PAPERBACK_TEXT_CAPACITY 192U
#define FLOPPY144_PAPERBACK_LINE_LIMIT 54U
#define FLOPPY144_PAPERBACK_FORM_COUNT 5U
#define FLOPPY144_PAPERBACK_AUTHOR_GIVEN_COUNT 8U
#define FLOPPY144_PAPERBACK_AUTHOR_FAMILY_COUNT 8U
#define FLOPPY144_PAPERBACK_NOTE_COUNT 8U

/*
 * Pure, stateless S4E-06 generation. Recovery seed is already saved in V2.
 * Returns false and clears nonempty output buffer on any failure.
 */
bool Floppy144PaperbackGenerate(
    uint32_t recovery_seed,
    char *output,
    uint32_t capacity
);

/*
 * Pure format helper shared with tests to exhaustively validate all 156,160
 * distinct index combinations (not an alternative gameplay entry point).
 * Fragment indices are zero-based, selected from the form-specific pools.
 */
bool Floppy144PaperbackCompose(
    uint32_t form,
    uint32_t left,
    uint32_t right,
    uint32_t given_name,
    uint32_t family_name,
    uint32_t note,
    char *output,
    uint32_t capacity
);
