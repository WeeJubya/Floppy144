/*
 * F144 portable startup/developer configuration.
 *
 * Platform launchers populate this semantic structure. Game code may query it
 * without knowing where arguments/settings came from.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct F144CalendarDate
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
} F144CalendarDate;

typedef struct F144StartupConfig
{
    bool debug_enabled;

    bool recovery_seed_override_enabled;
    uint32_t recovery_seed_override;

    bool fixed_date_override_enabled;
    F144CalendarDate fixed_date_override;
} F144StartupConfig;

/*
 * Reset all configuration to normal player-facing defaults.
 */
void f144StartupConfigReset(
    F144StartupConfig *config
);

/*
 * Enable or disable the semantic developer/debug mode.
 */
void f144StartupConfigSetDebugEnabled(
    F144StartupConfig *config,
    bool enabled
);

/*
 * Report whether developer/debug facilities are enabled.
 */
bool f144StartupConfigDebugEnabled(
    const F144StartupConfig *config
);

/*
 * Install a deterministic recovery seed for developer/regression use.
 *
 * A seed of zero is rejected because normal game startup reserves zero as an
 * invalid/uninitialised seed.
 */
bool f144StartupConfigSetRecoverySeedOverride(
    F144StartupConfig *config,
    uint32_t seed
);

/*
 * Read the deterministic recovery seed override when one is present.
 */
bool f144StartupConfigRecoverySeedOverride(
    const F144StartupConfig *config,
    uint32_t *seed
);

/*
 * Install a validated fixed calendar date for developer/regression use.
 */
bool f144StartupConfigSetFixedDateOverride(
    F144StartupConfig *config,
    uint16_t year,
    uint8_t month,
    uint8_t day
);

/*
 * Read the fixed calendar date override when one is present.
 */
bool f144StartupConfigFixedDateOverride(
    const F144StartupConfig *config,
    F144CalendarDate *date
);
