#pragma once

#include <stddef.h>

typedef struct StringView
{
    const char *data;
    size_t size;
}
StringView;

StringView cstr_sv
(
    const char *cstr
);

void sv_cstr
(
    StringView sv,
 char *buffer
);

const char *sv_find
(
    StringView pattern,
 StringView sv
);
