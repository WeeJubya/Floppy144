/*
 * F144 Win32 launch-configuration acquisition.
 */

#include "f144_win32_startup_config.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * Read one whitespace-delimited command-line token.
 *
 * The existing FLOPPY//144 developer switches contain no spaces, so preserving
 * the historical simple tokenisation is sufficient and keeps the parser tiny.
 */
static bool f144Win32StartupConfigNextToken(
    const char **cursor,
    const char **token_start,
    size_t *token_length
)
{
    const char *position;

    if(
        cursor == NULL ||
        *cursor == NULL ||
        token_start == NULL ||
        token_length == NULL
    )
    {
        return false;
    }

    position =
        *cursor;

    while(
        *position == ' ' ||
        *position == '\t'
    )
    {
        ++position;
    }

    if(*position == '\0')
    {
        *cursor =
            position;

        return false;
    }

    *token_start =
        position;

    while(
        *position != '\0' &&
        *position != ' ' &&
        *position != '\t'
    )
    {
        ++position;
    }

    *token_length =
        (size_t)(
            position -
            *token_start
        );

    *cursor =
        position;

    return true;
}

/*
 * Compare one parsed command-line token with an exact developer option.
 */
static bool f144Win32StartupConfigTokenEquals(
    const char *token_start,
    size_t token_length,
    const char *expected
)
{
    size_t expected_length;

    if(
        token_start == NULL ||
        expected == NULL
    )
    {
        return false;
    }

    expected_length =
        strlen(expected);

    return
        token_length == expected_length &&
        strncmp(
            token_start,
            expected,
            expected_length
        ) == 0;
}

/*
 * Copy a parsed token into a bounded local buffer for numeric/date parsing.
 */
static bool f144Win32StartupConfigCopyToken(
    const char *token_start,
    size_t token_length,
    char *buffer,
    size_t buffer_capacity
)
{
    if(
        token_start == NULL ||
        buffer == NULL ||
        buffer_capacity == 0U ||
        token_length == 0U ||
        token_length >= buffer_capacity
    )
    {
        return false;
    }

    memcpy(
        buffer,
        token_start,
        token_length
    );

    buffer[token_length] =
        '\0';

    return true;
}

/*
 * Parse a non-zero decimal uint32 recovery seed.
 */
static bool f144Win32StartupConfigParseSeed(
    const char *token_start,
    size_t token_length,
    uint32_t *seed
)
{
    char buffer[16];
    char *end;
    unsigned long value;

    if(
        seed == NULL ||
        !f144Win32StartupConfigCopyToken(
            token_start,
            token_length,
            buffer,
            sizeof(buffer)
        )
    )
    {
        return false;
    }

    errno =
        0;

    end =
        NULL;

    value =
        strtoul(
            buffer,
            &end,
            10
        );

    if(
        errno != 0 ||
        end == buffer ||
        *end != '\0' ||
        value == 0UL ||
        value > UINT32_MAX
    )
    {
        return false;
    }

    *seed =
        (uint32_t)value;

    return true;
}

/*
 * Convert exactly two decimal date digits into one byte value.
 */
static bool f144Win32StartupConfigParseTwoDigits(
    const char *text,
    uint8_t *value
)
{
    if(
        text == NULL ||
        value == NULL ||
        text[0] < '0' ||
        text[0] > '9' ||
        text[1] < '0' ||
        text[1] > '9'
    )
    {
        return false;
    }

    *value =
        (uint8_t)(
            (uint8_t)(text[0] - '0') * 10U +
            (uint8_t)(text[1] - '0')
        );

    return true;
}

/*
 * Parse a YYYY-MM-DD token and validate it through the portable config API.
 */
static bool f144Win32StartupConfigParseDate(
    const char *token_start,
    size_t token_length,
    F144StartupConfig *candidate_config
)
{
    char buffer[11];
    uint16_t year;
    uint8_t month;
    uint8_t day;

    if(
        candidate_config == NULL ||
        token_length != 10U ||
        !f144Win32StartupConfigCopyToken(
            token_start,
            token_length,
            buffer,
            sizeof(buffer)
        )
    )
    {
        return false;
    }

    if(
        buffer[4] != '-' ||
        buffer[7] != '-' ||
        buffer[0] < '0' ||
        buffer[0] > '9' ||
        buffer[1] < '0' ||
        buffer[1] > '9' ||
        buffer[2] < '0' ||
        buffer[2] > '9' ||
        buffer[3] < '0' ||
        buffer[3] > '9'
    )
    {
        return false;
    }

    year =
        (uint16_t)(
            (uint16_t)(buffer[0] - '0') * 1000U +
            (uint16_t)(buffer[1] - '0') * 100U +
            (uint16_t)(buffer[2] - '0') * 10U +
            (uint16_t)(buffer[3] - '0')
        );

    if(
        !f144Win32StartupConfigParseTwoDigits(
            &buffer[5],
            &month
        ) ||
        !f144Win32StartupConfigParseTwoDigits(
            &buffer[8],
            &day
        )
    )
    {
        return false;
    }

    return f144StartupConfigSetFixedDateOverride(
        candidate_config,
        year,
        month,
        day
    );
}

/*
 * Convert WinMain's raw command-line tail into portable developer config.
 */
bool f144Win32StartupConfigFromCommandLine(
    const char *command_line,
    F144StartupConfig *config
)
{
    const char *cursor;
    bool debug_enabled;
    bool hathaway_enabled = false;
    bool seed_seen;
    bool date_seen;
    uint32_t parsed_seed;
    F144StartupConfig parsed_overrides;

    if(config == NULL)
    {
        return false;
    }

    f144StartupConfigReset(
        config
    );

    f144StartupConfigReset(
        &parsed_overrides
    );

    cursor =
        command_line != NULL
            ? command_line
            : "";

    debug_enabled =
        false;

    seed_seen =
        false;

    date_seen =
        false;

    parsed_seed =
        0U;

    for(;;)
    {
        const char *token_start;
        size_t token_length;

        if(
            !f144Win32StartupConfigNextToken(
                &cursor,
                &token_start,
                &token_length
            )
        )
        {
            break;
        }

        if(
            f144Win32StartupConfigTokenEquals(
                token_start,
                token_length,
                "-GDR-CinderEllie"
            )
        )
        {
            debug_enabled =
                true;

            continue;
        }

        if(
            f144Win32StartupConfigTokenEquals(
                token_start, token_length, "-GDR-Hathaway"
            )
        )
        {
            hathaway_enabled = true;
            continue;
        }

        if(
            f144Win32StartupConfigTokenEquals(
                token_start,
                token_length,
                "-seed"
            )
        )
        {
            const char *value_start;
            size_t value_length;

            if(
                f144Win32StartupConfigNextToken(
                    &cursor,
                    &value_start,
                    &value_length
                ) &&
                f144Win32StartupConfigParseSeed(
                    value_start,
                    value_length,
                    &parsed_seed
                )
            )
            {
                seed_seen =
                    true;
            }

            continue;
        }

        if(
            f144Win32StartupConfigTokenEquals(
                token_start,
                token_length,
                "-date"
            )
        )
        {
            const char *value_start;
            size_t value_length;

            if(
                f144Win32StartupConfigNextToken(
                    &cursor,
                    &value_start,
                    &value_length
                ) &&
                f144Win32StartupConfigParseDate(
                    value_start,
                    value_length,
                    &parsed_overrides
                )
            )
            {
                date_seen =
                    true;
            }

            continue;
        }
    }

    f144StartupConfigSetDebugEnabled(
        config,
        debug_enabled && !hathaway_enabled
    );
    f144StartupConfigSetVisualInspectionEnabled(
        config, hathaway_enabled
    );

    /*
     * Deterministic overrides are developer facilities, so ordinary release
     * launches cannot activate them accidentally without explicitly enabling
     * -GDR-CinderEllie as well. Hathaway always overrides debug.
     */
    if(debug_enabled && !hathaway_enabled)
    {
        if(seed_seen)
        {
            (void)f144StartupConfigSetRecoverySeedOverride(
                config,
                parsed_seed
            );
        }

        if(date_seen)
        {
            F144CalendarDate date;

            if(
                f144StartupConfigFixedDateOverride(
                    &parsed_overrides,
                    &date
                )
            )
            {
                (void)f144StartupConfigSetFixedDateOverride(
                    config,
                    date.year,
                    date.month,
                    date.day
                );
            }
        }
    }

    return true;
}
