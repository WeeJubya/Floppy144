#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Stateless Stage 4E variation. Pass the existing RunState.recovery_seed.
 * Use stable nonempty ASCII feature namespaces and optional item IDs.
 * NULL item_id and "" are equivalent. Invalid feature returns zero/false.
 */
uint32_t Floppy144VariationValue(
    uint32_t recovery_seed, const char *feature, const char *item_id
);
uint32_t Floppy144VariationRange(
    uint32_t recovery_seed, const char *feature, const char *item_id,
    uint32_t count
);
bool Floppy144VariationChance(
    uint32_t recovery_seed, const char *feature, const char *item_id,
    uint32_t numerator, uint32_t denominator
);
