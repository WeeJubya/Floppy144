/*
 * F144 portable startup/developer configuration.
 */

#include "f144_startup_config.h"

#include <stddef.h>
#include <string.h>

/*
 * Determine whether a Gregorian year contains 29 February.
 */
static bool f144StartupConfigLeapYear(
    uint16_t year
)
{
    if((year % 400U) == 0U)
    {
        return true;
    }

    if((year % 100U) == 0U)
    {
        return false;
    }

    return (year % 4U) == 0U;
}

/*
 * Validate one Gregorian calendar date before exposing it to game systems.
 */
bool f144CalendarDateValid(
    uint16_t year,
    uint8_t month,
    uint8_t day
)
{
    static const uint8_t days_per_month[12] =
    {
        31U,
        28U,
        31U,
        30U,
        31U,
        30U,
        31U,
        31U,
        30U,
        31U,
        30U,
        31U
    };

    uint8_t maximum_day;

    if(
        year == 0U ||
        month == 0U ||
        month > 12U ||
        day == 0U
    )
    {
        return false;
    }

    maximum_day =
        days_per_month[month - 1U];

    if(
        month == 2U &&
        f144StartupConfigLeapYear(year)
    )
    {
        maximum_day =
            29U;
    }

    return day <= maximum_day;
}

/*
 * Reset all configuration to normal player-facing defaults.
 */
void f144StartupConfigReset(
    F144StartupConfig *config
)
{
    if(config == NULL)
    {
        return;
    }

    memset(
        config,
        0,
        sizeof(*config)
    );
}

/*
 * Enable or disable the semantic developer/debug mode.
 */
void f144StartupConfigSetDebugEnabled(
    F144StartupConfig *config,
    bool enabled
)
{
    if(config == NULL)
    {
        return;
    }

    config->debug_enabled =
        enabled;
}

/*
 * Report whether developer/debug facilities are enabled.
 */
bool f144StartupConfigDebugEnabled(
    const F144StartupConfig *config
)
{
    if(config == NULL)
    {
        return false;
    }

    return config->debug_enabled;
}

/*
 * Install a deterministic non-zero recovery seed.
 */
bool f144StartupConfigSetRecoverySeedOverride(
    F144StartupConfig *config,
    uint32_t seed
)
{
    if(
        config == NULL ||
        seed == 0U
    )
    {
        return false;
    }

    config->recovery_seed_override_enabled =
        true;

    config->recovery_seed_override =
        seed;

    return true;
}

/*
 * Read the configured deterministic recovery seed.
 */
bool f144StartupConfigRecoverySeedOverride(
    const F144StartupConfig *config,
    uint32_t *seed
)
{
    if(
        config == NULL ||
        seed == NULL ||
        !config->recovery_seed_override_enabled
    )
    {
        return false;
    }

    *seed =
        config->recovery_seed_override;

    return true;
}

/*
 * Install a validated fixed calendar date.
 */
bool f144StartupConfigSetFixedDateOverride(
    F144StartupConfig *config,
    uint16_t year,
    uint8_t month,
    uint8_t day
)
{
    if(
        config == NULL ||
        !f144CalendarDateValid(
            year,
            month,
            day
        )
    )
    {
        return false;
    }

    config->fixed_date_override_enabled =
        true;

    config->fixed_date_override.year =
        year;

    config->fixed_date_override.month =
        month;

    config->fixed_date_override.day =
        day;

    return true;
}

/*
 * Read the configured fixed calendar date.
 */
bool f144StartupConfigFixedDateOverride(
    const F144StartupConfig *config,
    F144CalendarDate *date
)
{
    if(
        config == NULL ||
        date == NULL ||
        !config->fixed_date_override_enabled
    )
    {
        return false;
    }

    *date =
        config->fixed_date_override;

    return true;
}
