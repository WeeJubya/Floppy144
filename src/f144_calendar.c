/* Portable game-facing wall-clock calendar query. No OS-specific calls. */
#include "f144_platform.h"

/*
 * Wall-clock calendar and monotonic animation time are deliberately separate.
 * Fixed dates are only honoured in developer mode, regardless of how a
 * configuration object was populated. Invalid native dates fail closed.
 */
bool f144PlatformCalendarDate(
    F144Platform *platform,
    const F144StartupConfig *config,
    F144CalendarDate *date
)
{
    F144CalendarDate result;

    if(date == NULL)
    {
        return false;
    }

    date->year = 0U;
    date->month = 0U;
    date->day = 0U;

    if(
        f144StartupConfigDebugEnabled(config) &&
        f144StartupConfigFixedDateOverride(config, &result) &&
        f144CalendarDateValid(result.year, result.month, result.day)
    )
    {
        *date = result;
        return true;
    }

    if(
        platform == NULL ||
        platform->api == NULL ||
        platform->api->local_calendar_date == NULL
    )
    {
        return false;
    }

    result.year = 0U;
    result.month = 0U;
    result.day = 0U;

    if(
        !platform->api->local_calendar_date(platform, &result) ||
        !f144CalendarDateValid(result.year, result.month, result.day)
    )
    {
        return false;
    }

    *date = result;
    return true;
}
