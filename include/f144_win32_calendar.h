#pragma once

#include "f144_platform.h"

/* Native local calendar, separate from monotonic timing. */
bool f144Win32LocalCalendarDate(
    F144Platform *platform,
    F144CalendarDate *date
);
