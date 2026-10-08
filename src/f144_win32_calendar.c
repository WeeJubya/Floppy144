/*
 * Native Win32 civil calendar adapter. No game content or override policy.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_win32_calendar.h"

bool f144Win32LocalCalendarDate(
    F144Platform *platform,
    F144CalendarDate *date
)
{
    SYSTEMTIME local_time;
    (void)platform;

    if(date == NULL)
    {
        return false;
    }

    GetLocalTime(&local_time);

    date->year = (uint16_t)local_time.wYear;
    date->month = (uint8_t)local_time.wMonth;
    date->day = (uint8_t)local_time.wDay;
    return true;
}
