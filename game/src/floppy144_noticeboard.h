#pragma once

#include "f144_startup_config.h"
#include "floppy144_game_data.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Read existing canonical AMB-NB date-window records. One contextual flyer
 * is displayed alongside, never instead of, authored permanent physical items.
 * Returns pointers to immutable authored and flavour-only text.
 */
bool Floppy144NoticeboardSelect(
    F144CalendarDate date,
    uint32_t recovery_seed,
    const Floppy144DataRecord **ambient,
    const char **annotation
);
