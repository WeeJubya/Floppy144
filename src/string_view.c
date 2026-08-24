#include "string_view.h"

#include <string.h>

StringView cstr_sv
(
    const char *cstr
){
    StringView result = {0};

    if(!cstr)
    {
        return result;
    }

    result.data = cstr;
    result.size = strlen(cstr);

    return result;
}

void sv_cstr
(
    StringView sv,
    char *buffer
){
    if(!buffer)
    {
        return;
    }

    if(!sv.data)
    {
        buffer[0] = '\0';
        return;
    }

    memcpy(buffer, sv.data, sv.size);
    buffer[sv.size] = '\0';
}

const char *sv_find
(
    StringView pattern,
    StringView sv
){
    size_t i;

    if(!pattern.data ||
       !sv.data ||
       pattern.size == 0 ||
       pattern.size > sv.size)
    {
        return 0;
    }

    for(i = 0; i <= sv.size - pattern.size; ++i)
    {
        if(memcmp(sv.data + i, pattern.data, pattern.size) == 0)
        {
            return sv.data + i;
        }
    }

    return 0;
}
