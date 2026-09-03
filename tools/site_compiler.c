/*
 * Floppy//144 Site Compiler
 *
 * Development-only JSONC -> generated C/X-macro data compiler.
 *
 * The generated file is intended to be included by the game at compile time.
 * The JSONC parser and this tool do NOT ship with Floppy144.exe.
 *
 * C99, standard library only.
 */

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F144_SITE_SIZE 100
#define F144_FIXED_ONE 16
#define F144_SCHEMA_VERSION 1

#define F144_MAX_STRING 128
#define F144_MAX_ROOMS 11
#define F144_MAX_REGIONS 128
#define F144_MAX_PLACEMENTS 2048
#define F144_MAX_SHARED 1024
#define F144_MAX_IDS 3072

#define F144_ROOM_OUTSIDE (-2)
#define F144_ROOM_INVALID (-1)

typedef enum TokenType
{
    TOK_EOF = 0,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COLON,
    TOK_COMMA,
    TOK_STRING,
    TOK_NUMBER,
    TOK_TRUE,
    TOK_FALSE,
    TOK_NULL,
    TOK_ERROR
}
TokenType;

typedef struct Token
{
    TokenType type;
    char text[F144_MAX_STRING];
    double number;
    int line;
    int column;
}
Token;

typedef struct Lexer
{
    const char *text;
    size_t length;
    size_t position;
    int line;
    int column;
    Token current;
}
Lexer;

typedef struct Region
{
    int room;
    int x;
    int y;
    int width;
    int height;
}
Region;

typedef struct Placement
{
    int room;
    bool shared;
    bool is_door;

    char id[F144_MAX_STRING];
    char type[F144_MAX_STRING];
    char variant[F144_MAX_STRING];
    char object_ref[F144_MAX_STRING];
    char from[F144_MAX_STRING];
    char to[F144_MAX_STRING];

    bool has_id;
    bool has_variant;
    bool has_object;
    bool has_from;
    bool has_to;

    bool has_x;
    bool has_y;
    bool has_centre_x;
    bool has_centre_y;
    bool has_width;
    bool has_height;
    bool has_rotation;

    double x;
    double y;
    double centre_x;
    double centre_y;
    double width;
    double height;
    double rotation;

    int rotation_normalized;

    /* Conservative whole-unit AABB used by the current Site runtime. */
    int bounds_x;
    int bounds_y;
    int bounds_width;
    int bounds_height;

    /* Original rotated geometry in 1/16 Site-unit fixed point. */
    int centre_x16;
    int centre_y16;
    int width16;
    int height16;
}
Placement;

typedef struct RoomInfo
{
    bool present;
    char id[F144_MAX_STRING];
    int first_region;
    int region_count;
    int geometry_count;
    int floor_count;
}
RoomInfo;

typedef struct SiteData
{
    int schema_version;
    bool has_schema_version;

    int site_width;
    int site_height;
    bool has_site_width;
    bool has_site_height;

    double spawn_x;
    double spawn_y;
    char spawn_room[F144_MAX_STRING];
    bool has_spawn_x;
    bool has_spawn_y;
    bool has_spawn_room;

    RoomInfo rooms[F144_MAX_ROOMS];

    Region regions[F144_MAX_REGIONS];
    int region_count;

    Placement placements[F144_MAX_PLACEMENTS];
    int placement_count;

    Placement shared[F144_MAX_SHARED];
    int shared_count;

    int warnings;
    int errors;
}
SiteData;

typedef struct NameMap
{
    const char *json_name;
    const char *c_name;
}
NameMap;

static const NameMap g_rooms[F144_MAX_ROOMS] =
{
    { "RECEPTION",        "FLOPPY144_ROOM_RECEPTION" },
    { "CORRIDOR",         "FLOPPY144_ROOM_CORRIDOR" },
    { "MAIN_OFFICE",      "FLOPPY144_ROOM_MAIN_OFFICE" },
    { "FACILITIES",       "FLOPPY144_ROOM_FACILITIES" },
    { "RECORDS_OFFICE",   "FLOPPY144_ROOM_RECORDS_OFFICE" },
    { "IT_SUPPORT",       "FLOPPY144_ROOM_IT_SUPPORT" },
    { "STAFF_ROOM",       "FLOPPY144_ROOM_STAFF_ROOM" },
    { "SECRETARY_OFFICE", "FLOPPY144_ROOM_SECRETARY_OFFICE" },
    { "DIRECTOR_OFFICE",  "FLOPPY144_ROOM_DIRECTOR_OFFICE" },
    { "SECURITY",         "FLOPPY144_ROOM_SECURITY" },
    { "SERVER_ROOM",      "FLOPPY144_ROOM_SERVER_ROOM" }
};

static const NameMap g_types[] =
{
    { "FLOOR_A",              "FLOPPY144_SITE_FLOOR_A" },
    { "FLOOR_B",              "FLOPPY144_SITE_FLOOR_B" },
    { "FLOOR_C",              "FLOPPY144_SITE_FLOOR_C" },
    { "FLOOR_D",              "FLOPPY144_SITE_FLOOR_D" },
    { "DOOR",                 "FLOPPY144_SITE_DOOR" },
    { "WINDOW",               "FLOPPY144_SITE_WINDOW" },
    { "STANDARD_DESK",        "FLOPPY144_SITE_STANDARD_DESK" },
    { "CHAIR",                "FLOPPY144_SITE_CHAIR" },
    { "NONSECURE_CABINET",    "FLOPPY144_SITE_NONSECURE_CABINET" },
    { "SECURE_CABINET_HALF",  "FLOPPY144_SITE_SECURE_CABINET_HALF" },
    { "SECURE_CABINET_FULL",  "FLOPPY144_SITE_SECURE_CABINET_FULL" },
    { "WALL_MOUNTED_ITEM",    "FLOPPY144_SITE_WALL_MOUNTED_ITEM" },
    { "BOOKCASE",             "FLOPPY144_SITE_BOOKCASE" },
    { "TERMINAL_DESK",        "FLOPPY144_SITE_TERMINAL_DESK" },
    { "PARTITION_WALL",       "FLOPPY144_SITE_PARTITION_WALL" },
    { "FRIDGE",               "FLOPPY144_SITE_FRIDGE" },
    { "WORKTOP",              "FLOPPY144_SITE_WORKTOP" },
    { "SINK",                 "FLOPPY144_SITE_SINK" },
    { "COFFEE_MAKER",         "FLOPPY144_SITE_COFFEE_MAKER" },
    { "SOFA",                 "FLOPPY144_SITE_SOFA" },
    { "SERVER",               "FLOPPY144_SITE_SERVER" },
    { "SHELVING_FULL",        "FLOPPY144_SITE_SHELVING_FULL" },
    { "TROLLEY",              "FLOPPY144_SITE_TROLLEY" },
    { "TABLE",                "FLOPPY144_SITE_TABLE" }
};

static void CopyString(char *destination, size_t capacity, const char *source)
{
    if(capacity == 0U)
    {
        return;
    }

    if(source == NULL)
    {
        destination[0] = '\0';
        return;
    }

    {
        size_t length = strlen(source);
        if(length >= capacity) length = capacity - 1U;
        memcpy(destination, source, length);
        destination[length] = '\0';
    }
}

static void SiteMessage(SiteData *site, bool error, const char *format, ...)
{
    va_list arguments;

    if(error)
    {
        ++site->errors;
        fputs("ERROR: ", stderr);
    }
    else
    {
        ++site->warnings;
        fputs("WARNING: ", stderr);
    }

    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);

    fputc('\n', stderr);
}

static bool IsNearlyInteger(double value)
{
    int64_t integer_value = (int64_t)value;
    double difference = value - (double)integer_value;

    if(difference < 0.0)
    {
        difference = -difference;
    }

    return difference < 0.000001;
}

static int RoundToInt(double value)
{
    return value >= 0.0
        ? (int)(value + 0.5)
        : (int)(value - 0.5);
}

static bool ToFixed16(double value, int *result)
{
    double scaled = value * (double)F144_FIXED_ONE;
    int rounded = RoundToInt(scaled);
    double difference = scaled - (double)rounded;

    if(difference < 0.0)
    {
        difference = -difference;
    }

    if(difference > 0.000001)
    {
        return false;
    }

    *result = rounded;
    return true;
}

static int FloorDiv16(int value)
{
    if(value >= 0)
    {
        return value / F144_FIXED_ONE;
    }

    return -(((-value) + F144_FIXED_ONE - 1) / F144_FIXED_ONE);
}

static int CeilDiv16(int value)
{
    if(value >= 0)
    {
        return (value + F144_FIXED_ONE - 1) / F144_FIXED_ONE;
    }

    return -((-value) / F144_FIXED_ONE);
}

static int RoomIndex(const char *name)
{
    int index;

    if(strcmp(name, "OUTSIDE") == 0)
    {
        return F144_ROOM_OUTSIDE;
    }

    for(index = 0; index < F144_MAX_ROOMS; ++index)
    {
        if(strcmp(name, g_rooms[index].json_name) == 0)
        {
            return index;
        }
    }

    return F144_ROOM_INVALID;
}

static const char *RoomCSymbol(int room)
{
    if(room == F144_ROOM_OUTSIDE)
    {
        return "FLOPPY144_SITE_ROOM_OUTSIDE";
    }

    if(room >= 0 && room < F144_MAX_ROOMS)
    {
        return g_rooms[room].c_name;
    }

    return "FLOPPY144_ROOM_COUNT";
}

static const char *TypeCSymbol(const char *name)
{
    size_t index;

    for(index = 0U; index < sizeof(g_types) / sizeof(g_types[0]); ++index)
    {
        if(strcmp(name, g_types[index].json_name) == 0)
        {
            return g_types[index].c_name;
        }
    }

    return NULL;
}

static bool IsFloorType(const char *name)
{
    return
        strcmp(name, "FLOOR_A") == 0 ||
        strcmp(name, "FLOOR_B") == 0 ||
        strcmp(name, "FLOOR_C") == 0 ||
        strcmp(name, "FLOOR_D") == 0;
}

static void LexerAdvanceCharacter(Lexer *lexer)
{
    if(lexer->position >= lexer->length)
    {
        return;
    }

    if(lexer->text[lexer->position] == '\n')
    {
        ++lexer->line;
        lexer->column = 1;
    }
    else
    {
        ++lexer->column;
    }

    ++lexer->position;
}

static void LexerSkipWhitespaceAndComments(Lexer *lexer)
{
    for(;;)
    {
        while(
            lexer->position < lexer->length &&
            isspace((unsigned char)lexer->text[lexer->position])
        )
        {
            LexerAdvanceCharacter(lexer);
        }

        if(
            lexer->position + 1U < lexer->length &&
            lexer->text[lexer->position] == '/' &&
            lexer->text[lexer->position + 1U] == '/'
        )
        {
            LexerAdvanceCharacter(lexer);
            LexerAdvanceCharacter(lexer);

            while(
                lexer->position < lexer->length &&
                lexer->text[lexer->position] != '\n'
            )
            {
                LexerAdvanceCharacter(lexer);
            }

            continue;
        }

        if(
            lexer->position + 1U < lexer->length &&
            lexer->text[lexer->position] == '/' &&
            lexer->text[lexer->position + 1U] == '*'
        )
        {
            LexerAdvanceCharacter(lexer);
            LexerAdvanceCharacter(lexer);

            while(lexer->position + 1U < lexer->length)
            {
                if(
                    lexer->text[lexer->position] == '*' &&
                    lexer->text[lexer->position + 1U] == '/'
                )
                {
                    LexerAdvanceCharacter(lexer);
                    LexerAdvanceCharacter(lexer);
                    break;
                }

                LexerAdvanceCharacter(lexer);
            }

            continue;
        }

        break;
    }
}

static Token LexerReadToken(Lexer *lexer)
{
    Token token;
    char c;

    memset(&token, 0, sizeof(token));

    LexerSkipWhitespaceAndComments(lexer);

    token.line = lexer->line;
    token.column = lexer->column;

    if(lexer->position >= lexer->length)
    {
        token.type = TOK_EOF;
        return token;
    }

    c = lexer->text[lexer->position];

    switch(c)
    {
        case '{': token.type = TOK_LBRACE;   LexerAdvanceCharacter(lexer); return token;
        case '}': token.type = TOK_RBRACE;   LexerAdvanceCharacter(lexer); return token;
        case '[': token.type = TOK_LBRACKET; LexerAdvanceCharacter(lexer); return token;
        case ']': token.type = TOK_RBRACKET; LexerAdvanceCharacter(lexer); return token;
        case ':': token.type = TOK_COLON;    LexerAdvanceCharacter(lexer); return token;
        case ',': token.type = TOK_COMMA;    LexerAdvanceCharacter(lexer); return token;
        default: break;
    }

    if(c == '"')
    {
        size_t out = 0U;
        bool closed = false;

        LexerAdvanceCharacter(lexer);

        while(lexer->position < lexer->length)
        {
            c = lexer->text[lexer->position];

            if(c == '"')
            {
                LexerAdvanceCharacter(lexer);
                closed = true;
                break;
            }

            if(c == '\\')
            {
                char escaped;

                LexerAdvanceCharacter(lexer);

                if(lexer->position >= lexer->length)
                {
                    break;
                }

                escaped = lexer->text[lexer->position];

                switch(escaped)
                {
                    case '"': c = '"'; break;
                    case '\\': c = '\\'; break;
                    case '/': c = '/'; break;
                    case 'b': c = '\b'; break;
                    case 'f': c = '\f'; break;
                    case 'n': c = '\n'; break;
                    case 'r': c = '\r'; break;
                    case 't': c = '\t'; break;
                    default: c = '?'; break;
                }

                LexerAdvanceCharacter(lexer);
            }
            else
            {
                LexerAdvanceCharacter(lexer);
            }

            if(out + 1U < sizeof(token.text))
            {
                token.text[out++] = c;
            }
        }

        token.text[out] = '\0';
        token.type = closed ? TOK_STRING : TOK_ERROR;
        return token;
    }

    if(
        c == '-' ||
        c == '+' ||
        c == '.' ||
        isdigit((unsigned char)c)
    )
    {
        const char *start = lexer->text + lexer->position;
        char *end = NULL;

        errno = 0;
        token.number = strtod(start, &end);

        if(end == start || errno == ERANGE)
        {
            token.type = TOK_ERROR;
            return token;
        }

        while(lexer->text + lexer->position < end)
        {
            LexerAdvanceCharacter(lexer);
        }

        token.type = TOK_NUMBER;
        return token;
    }

    if(lexer->position + 4U <= lexer->length)
    {
        if(strncmp(lexer->text + lexer->position, "true", 4U) == 0)
        {
            int i;
            for(i = 0; i < 4; ++i) LexerAdvanceCharacter(lexer);
            token.type = TOK_TRUE;
            return token;
        }
        if(strncmp(lexer->text + lexer->position, "null", 4U) == 0)
        {
            int i;
            for(i = 0; i < 4; ++i) LexerAdvanceCharacter(lexer);
            token.type = TOK_NULL;
            return token;
        }
    }

    if(
        lexer->position + 5U <= lexer->length &&
        strncmp(lexer->text + lexer->position, "false", 5U) == 0
    )
    {
        int i;
        for(i = 0; i < 5; ++i) LexerAdvanceCharacter(lexer);
        token.type = TOK_FALSE;
        return token;
    }

    token.type = TOK_ERROR;
    LexerAdvanceCharacter(lexer);
    return token;
}

static void LexerNext(Lexer *lexer)
{
    lexer->current = LexerReadToken(lexer);
}

static bool ParseError(SiteData *site, const Lexer *lexer, const char *message)
{
    SiteMessage(
        site,
        true,
        "JSONC line %d, column %d: %s",
        lexer->current.line,
        lexer->current.column,
        message
    );

    return false;
}

static bool Expect(SiteData *site, Lexer *lexer, TokenType type, const char *message)
{
    if(lexer->current.type != type)
    {
        return ParseError(site, lexer, message);
    }

    LexerNext(lexer);
    return true;
}

static bool ParseStringValue(SiteData *site, Lexer *lexer, char *destination, size_t capacity)
{
    if(lexer->current.type != TOK_STRING)
    {
        return ParseError(site, lexer, "expected string value");
    }

    CopyString(destination, capacity, lexer->current.text);
    LexerNext(lexer);
    return true;
}

static bool ParseNumberValue(SiteData *site, Lexer *lexer, double *destination)
{
    if(lexer->current.type != TOK_NUMBER)
    {
        return ParseError(site, lexer, "expected numeric value");
    }

    *destination = lexer->current.number;
    LexerNext(lexer);
    return true;
}

static bool SkipValue(SiteData *site, Lexer *lexer);

static bool SkipObject(SiteData *site, Lexer *lexer)
{
    if(!Expect(site, lexer, TOK_LBRACE, "expected '{'"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACE)
    {
        if(lexer->current.type == TOK_EOF)
        {
            return ParseError(site, lexer, "unexpected end of file inside object");
        }

        if(lexer->current.type != TOK_STRING)
        {
            return ParseError(site, lexer, "expected object key");
        }

        LexerNext(lexer);

        if(!Expect(site, lexer, TOK_COLON, "expected ':' after object key"))
        {
            return false;
        }

        if(!SkipValue(site, lexer))
        {
            return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACE)
            {
                break;
            }
        }
        else if(lexer->current.type != TOK_RBRACE)
        {
            return ParseError(site, lexer, "expected ',' or '}'");
        }
    }

    return Expect(site, lexer, TOK_RBRACE, "expected '}'");
}

static bool SkipArray(SiteData *site, Lexer *lexer)
{
    if(!Expect(site, lexer, TOK_LBRACKET, "expected '['"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACKET)
    {
        if(lexer->current.type == TOK_EOF)
        {
            return ParseError(site, lexer, "unexpected end of file inside array");
        }

        if(!SkipValue(site, lexer))
        {
            return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACKET)
            {
                break;
            }
        }
        else if(lexer->current.type != TOK_RBRACKET)
        {
            return ParseError(site, lexer, "expected ',' or ']' in array");
        }
    }

    return Expect(site, lexer, TOK_RBRACKET, "expected ']'");
}

static bool SkipValue(SiteData *site, Lexer *lexer)
{
    switch(lexer->current.type)
    {
        case TOK_LBRACE:   return SkipObject(site, lexer);
        case TOK_LBRACKET: return SkipArray(site, lexer);
        case TOK_STRING:
        case TOK_NUMBER:
        case TOK_TRUE:
        case TOK_FALSE:
        case TOK_NULL:
            LexerNext(lexer);
            return true;
        default:
            return ParseError(site, lexer, "expected JSON value");
    }
}

static bool ParseRegionObject(SiteData *site, Lexer *lexer, Region *region)
{
    bool has_x = false;
    bool has_y = false;
    bool has_width = false;
    bool has_height = false;
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    if(!Expect(site, lexer, TOK_LBRACE, "expected region object"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACE)
    {
        char key[F144_MAX_STRING];

        if(lexer->current.type != TOK_STRING)
        {
            return ParseError(site, lexer, "expected region key");
        }

        CopyString(key, sizeof(key), lexer->current.text);
        LexerNext(lexer);

        if(!Expect(site, lexer, TOK_COLON, "expected ':' after region key"))
        {
            return false;
        }

        if(strcmp(key, "x") == 0)
        {
            if(!ParseNumberValue(site, lexer, &x)) return false;
            has_x = true;
        }
        else if(strcmp(key, "y") == 0)
        {
            if(!ParseNumberValue(site, lexer, &y)) return false;
            has_y = true;
        }
        else if(strcmp(key, "width") == 0)
        {
            if(!ParseNumberValue(site, lexer, &width)) return false;
            has_width = true;
        }
        else if(strcmp(key, "height") == 0)
        {
            if(!ParseNumberValue(site, lexer, &height)) return false;
            has_height = true;
        }
        else
        {
            if(!SkipValue(site, lexer)) return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACE) break;
        }
        else if(lexer->current.type != TOK_RBRACE)
        {
            return ParseError(site, lexer, "expected ',' or '}' in region");
        }
    }

    if(!Expect(site, lexer, TOK_RBRACE, "expected '}' after region"))
    {
        return false;
    }

    if(!has_x || !has_y || !has_width || !has_height)
    {
        SiteMessage(site, true, "room region is missing x, y, width or height");
        return true;
    }

    if(
        !IsNearlyInteger(x) ||
        !IsNearlyInteger(y) ||
        !IsNearlyInteger(width) ||
        !IsNearlyInteger(height)
    )
    {
        SiteMessage(site, true, "room regions must use whole Site units");
        return true;
    }

    region->x = RoundToInt(x);
    region->y = RoundToInt(y);
    region->width = RoundToInt(width);
    region->height = RoundToInt(height);

    return true;
}

static bool ParsePlacementObject(SiteData *site, Lexer *lexer, Placement *placement)
{
    memset(placement, 0, sizeof(*placement));
    placement->room = F144_ROOM_INVALID;

    if(!Expect(site, lexer, TOK_LBRACE, "expected geometry object"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACE)
    {
        char key[F144_MAX_STRING];

        if(lexer->current.type != TOK_STRING)
        {
            return ParseError(site, lexer, "expected geometry key");
        }

        CopyString(key, sizeof(key), lexer->current.text);
        LexerNext(lexer);

        if(!Expect(site, lexer, TOK_COLON, "expected ':' after geometry key"))
        {
            return false;
        }

        if(strcmp(key, "id") == 0)
        {
            if(!ParseStringValue(site, lexer, placement->id, sizeof(placement->id))) return false;
            placement->has_id = true;
        }
        else if(strcmp(key, "type") == 0)
        {
            if(!ParseStringValue(site, lexer, placement->type, sizeof(placement->type))) return false;
        }
        else if(strcmp(key, "variant") == 0)
        {
            if(!ParseStringValue(site, lexer, placement->variant, sizeof(placement->variant))) return false;
            placement->has_variant = true;
        }
        else if(strcmp(key, "object") == 0)
        {
            if(!ParseStringValue(site, lexer, placement->object_ref, sizeof(placement->object_ref))) return false;
            placement->has_object = true;
        }
        else if(strcmp(key, "from") == 0)
        {
            if(!ParseStringValue(site, lexer, placement->from, sizeof(placement->from))) return false;
            placement->has_from = true;
        }
        else if(strcmp(key, "to") == 0)
        {
            if(!ParseStringValue(site, lexer, placement->to, sizeof(placement->to))) return false;
            placement->has_to = true;
        }
        else if(strcmp(key, "x") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->x)) return false;
            placement->has_x = true;
        }
        else if(strcmp(key, "y") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->y)) return false;
            placement->has_y = true;
        }
        else if(strcmp(key, "centre_x") == 0 || strcmp(key, "center_x") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->centre_x)) return false;
            placement->has_centre_x = true;
        }
        else if(strcmp(key, "centre_y") == 0 || strcmp(key, "center_y") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->centre_y)) return false;
            placement->has_centre_y = true;
        }
        else if(strcmp(key, "width") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->width)) return false;
            placement->has_width = true;
        }
        else if(strcmp(key, "height") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->height)) return false;
            placement->has_height = true;
        }
        else if(strcmp(key, "rotation") == 0)
        {
            if(!ParseNumberValue(site, lexer, &placement->rotation)) return false;
            placement->has_rotation = true;
        }
        else
        {
            if(!SkipValue(site, lexer)) return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACE) break;
        }
        else if(lexer->current.type != TOK_RBRACE)
        {
            return ParseError(site, lexer, "expected ',' or '}' in geometry object");
        }
    }

    if(!Expect(site, lexer, TOK_RBRACE, "expected '}' after geometry object"))
    {
        return false;
    }

    return true;
}

static bool ParseRegionArray(
    SiteData *site,
    Lexer *lexer,
    int first_region,
    int *region_count
)
{
    int count = 0;

    if(!Expect(site, lexer, TOK_LBRACKET, "expected regions array"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACKET)
    {
        if(site->region_count >= F144_MAX_REGIONS)
        {
            SiteMessage(site, true, "too many room regions (limit %d)", F144_MAX_REGIONS);
            return false;
        }

        if(!ParseRegionObject(site, lexer, &site->regions[site->region_count]))
        {
            return false;
        }

        ++site->region_count;
        ++count;

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACKET) break;
        }
        else if(lexer->current.type != TOK_RBRACKET)
        {
            return ParseError(site, lexer, "expected ',' or ']' in regions array");
        }
    }

    if(!Expect(site, lexer, TOK_RBRACKET, "expected ']' after regions"))
    {
        return false;
    }

    (void)first_region;
    *region_count = count;
    return true;
}

static bool ParseGeometryArray(
    SiteData *site,
    Lexer *lexer,
    int first_geometry,
    int *geometry_count
)
{
    int count = 0;

    if(!Expect(site, lexer, TOK_LBRACKET, "expected geometry array"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACKET)
    {
        if(site->placement_count >= F144_MAX_PLACEMENTS)
        {
            SiteMessage(site, true, "too many room geometry entries (limit %d)", F144_MAX_PLACEMENTS);
            return false;
        }

        if(!ParsePlacementObject(site, lexer, &site->placements[site->placement_count]))
        {
            return false;
        }

        ++site->placement_count;
        ++count;

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACKET) break;
        }
        else if(lexer->current.type != TOK_RBRACKET)
        {
            return ParseError(site, lexer, "expected ',' or ']' in geometry array");
        }
    }

    if(!Expect(site, lexer, TOK_RBRACKET, "expected ']' after geometry"))
    {
        return false;
    }

    (void)first_geometry;
    *geometry_count = count;
    return true;
}

static bool ParseRoomObject(SiteData *site, Lexer *lexer)
{
    char room_id[F144_MAX_STRING] = "";
    bool has_id = false;
    int first_region = site->region_count;
    int first_geometry = site->placement_count;
    int region_count = 0;
    int geometry_count = 0;
    bool regions_seen = false;
    bool geometry_seen = false;
    int room_index;
    int index;

    if(!Expect(site, lexer, TOK_LBRACE, "expected room object"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACE)
    {
        char key[F144_MAX_STRING];

        if(lexer->current.type != TOK_STRING)
        {
            return ParseError(site, lexer, "expected room key");
        }

        CopyString(key, sizeof(key), lexer->current.text);
        LexerNext(lexer);

        if(!Expect(site, lexer, TOK_COLON, "expected ':' after room key"))
        {
            return false;
        }

        if(strcmp(key, "id") == 0)
        {
            if(!ParseStringValue(site, lexer, room_id, sizeof(room_id))) return false;
            has_id = true;
        }
        else if(strcmp(key, "regions") == 0)
        {
            if(regions_seen)
            {
                SiteMessage(site, true, "room contains duplicate 'regions' field");
            }
            if(!ParseRegionArray(site, lexer, first_region, &region_count)) return false;
            regions_seen = true;
        }
        else if(strcmp(key, "geometry") == 0)
        {
            if(geometry_seen)
            {
                SiteMessage(site, true, "room contains duplicate 'geometry' field");
            }
            if(!ParseGeometryArray(site, lexer, first_geometry, &geometry_count)) return false;
            geometry_seen = true;
        }
        else
        {
            if(!SkipValue(site, lexer)) return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACE) break;
        }
        else if(lexer->current.type != TOK_RBRACE)
        {
            return ParseError(site, lexer, "expected ',' or '}' in room object");
        }
    }

    if(!Expect(site, lexer, TOK_RBRACE, "expected '}' after room"))
    {
        return false;
    }

    if(!has_id)
    {
        SiteMessage(site, true, "room is missing required 'id'");
        return true;
    }

    room_index = RoomIndex(room_id);

    if(room_index < 0)
    {
        SiteMessage(site, true, "unknown room id '%s'", room_id);
        return true;
    }

    if(site->rooms[room_index].present)
    {
        SiteMessage(site, true, "room '%s' is defined more than once", room_id);
        return true;
    }

    site->rooms[room_index].present = true;
    CopyString(site->rooms[room_index].id, sizeof(site->rooms[room_index].id), room_id);
    site->rooms[room_index].first_region = first_region;
    site->rooms[room_index].region_count = region_count;
    site->rooms[room_index].geometry_count = geometry_count;

    for(index = first_region; index < first_region + region_count; ++index)
    {
        site->regions[index].room = room_index;
    }

    for(index = first_geometry; index < first_geometry + geometry_count; ++index)
    {
        site->placements[index].room = room_index;
        site->placements[index].shared = false;
    }

    return true;
}

static bool ParseRoomsArray(SiteData *site, Lexer *lexer)
{
    if(!Expect(site, lexer, TOK_LBRACKET, "expected rooms array"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACKET)
    {
        if(!ParseRoomObject(site, lexer))
        {
            return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACKET) break;
        }
        else if(lexer->current.type != TOK_RBRACKET)
        {
            return ParseError(site, lexer, "expected ',' or ']' in rooms array");
        }
    }

    return Expect(site, lexer, TOK_RBRACKET, "expected ']' after rooms");
}

static bool ParseSharedArray(SiteData *site, Lexer *lexer)
{
    if(!Expect(site, lexer, TOK_LBRACKET, "expected shared_geometry array"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACKET)
    {
        Placement *placement;

        if(site->shared_count >= F144_MAX_SHARED)
        {
            SiteMessage(site, true, "too many shared geometry entries (limit %d)", F144_MAX_SHARED);
            return false;
        }

        placement = &site->shared[site->shared_count];

        if(!ParsePlacementObject(site, lexer, placement))
        {
            return false;
        }

        placement->shared = true;
        placement->room = F144_ROOM_INVALID;
        ++site->shared_count;

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACKET) break;
        }
        else if(lexer->current.type != TOK_RBRACKET)
        {
            return ParseError(site, lexer, "expected ',' or ']' in shared_geometry array");
        }
    }

    return Expect(site, lexer, TOK_RBRACKET, "expected ']' after shared_geometry");
}

static bool ParseSpawnObject(SiteData *site, Lexer *lexer)
{
    if(!Expect(site, lexer, TOK_LBRACE, "expected spawn object"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACE)
    {
        char key[F144_MAX_STRING];

        if(lexer->current.type != TOK_STRING)
        {
            return ParseError(site, lexer, "expected spawn key");
        }

        CopyString(key, sizeof(key), lexer->current.text);
        LexerNext(lexer);

        if(!Expect(site, lexer, TOK_COLON, "expected ':' after spawn key")) return false;

        if(strcmp(key, "x") == 0)
        {
            if(!ParseNumberValue(site, lexer, &site->spawn_x)) return false;
            site->has_spawn_x = true;
        }
        else if(strcmp(key, "y") == 0)
        {
            if(!ParseNumberValue(site, lexer, &site->spawn_y)) return false;
            site->has_spawn_y = true;
        }
        else if(strcmp(key, "room") == 0)
        {
            if(!ParseStringValue(site, lexer, site->spawn_room, sizeof(site->spawn_room))) return false;
            site->has_spawn_room = true;
        }
        else
        {
            if(!SkipValue(site, lexer)) return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACE) break;
        }
        else if(lexer->current.type != TOK_RBRACE)
        {
            return ParseError(site, lexer, "expected ',' or '}' in spawn object");
        }
    }

    return Expect(site, lexer, TOK_RBRACE, "expected '}' after spawn");
}

static bool ParseSiteObject(SiteData *site, Lexer *lexer)
{
    if(!Expect(site, lexer, TOK_LBRACE, "expected site object"))
    {
        return false;
    }

    while(lexer->current.type != TOK_RBRACE)
    {
        char key[F144_MAX_STRING];
        double number;

        if(lexer->current.type != TOK_STRING)
        {
            return ParseError(site, lexer, "expected site key");
        }

        CopyString(key, sizeof(key), lexer->current.text);
        LexerNext(lexer);

        if(!Expect(site, lexer, TOK_COLON, "expected ':' after site key")) return false;

        if(strcmp(key, "width") == 0)
        {
            if(!ParseNumberValue(site, lexer, &number)) return false;
            site->site_width = RoundToInt(number);
            site->has_site_width = true;
            if(!IsNearlyInteger(number)) SiteMessage(site, true, "site width must be an integer");
        }
        else if(strcmp(key, "height") == 0)
        {
            if(!ParseNumberValue(site, lexer, &number)) return false;
            site->site_height = RoundToInt(number);
            site->has_site_height = true;
            if(!IsNearlyInteger(number)) SiteMessage(site, true, "site height must be an integer");
        }
        else if(strcmp(key, "spawn") == 0)
        {
            if(!ParseSpawnObject(site, lexer)) return false;
        }
        else
        {
            if(!SkipValue(site, lexer)) return false;
        }

        if(lexer->current.type == TOK_COMMA)
        {
            LexerNext(lexer);
            if(lexer->current.type == TOK_RBRACE) break;
        }
        else if(lexer->current.type != TOK_RBRACE)
        {
            return ParseError(site, lexer, "expected ',' or '}' in site object");
        }
    }

    return Expect(site, lexer, TOK_RBRACE, "expected '}' after site");
}

static bool ParseDocument(SiteData *site, const char *text, size_t length)
{
    Lexer lexer;

    memset(&lexer, 0, sizeof(lexer));
    lexer.text = text;
    lexer.length = length;
    lexer.line = 1;
    lexer.column = 1;

    LexerNext(&lexer);

    if(!Expect(site, &lexer, TOK_LBRACE, "expected top-level object"))
    {
        return false;
    }

    while(lexer.current.type != TOK_RBRACE)
    {
        char key[F144_MAX_STRING];

        if(lexer.current.type == TOK_EOF)
        {
            return ParseError(site, &lexer, "unexpected end of file");
        }

        if(lexer.current.type != TOK_STRING)
        {
            return ParseError(site, &lexer, "expected top-level key");
        }

        CopyString(key, sizeof(key), lexer.current.text);
        LexerNext(&lexer);

        if(!Expect(site, &lexer, TOK_COLON, "expected ':' after top-level key")) return false;

        if(strcmp(key, "schema_version") == 0)
        {
            double version;
            if(!ParseNumberValue(site, &lexer, &version)) return false;
            site->schema_version = RoundToInt(version);
            site->has_schema_version = true;
            if(!IsNearlyInteger(version)) SiteMessage(site, true, "schema_version must be an integer");
        }
        else if(strcmp(key, "site") == 0)
        {
            if(!ParseSiteObject(site, &lexer)) return false;
        }
        else if(strcmp(key, "rooms") == 0)
        {
            if(!ParseRoomsArray(site, &lexer)) return false;
        }
        else if(strcmp(key, "shared_geometry") == 0)
        {
            if(!ParseSharedArray(site, &lexer)) return false;
        }
        else
        {
            if(!SkipValue(site, &lexer)) return false;
        }

        if(lexer.current.type == TOK_COMMA)
        {
            LexerNext(&lexer);
            if(lexer.current.type == TOK_RBRACE) break;
        }
        else if(lexer.current.type != TOK_RBRACE)
        {
            return ParseError(site, &lexer, "expected ',' or '}' at top level");
        }
    }

    if(!Expect(site, &lexer, TOK_RBRACE, "expected final '}'"))
    {
        return false;
    }

    if(lexer.current.type != TOK_EOF)
    {
        return ParseError(site, &lexer, "unexpected content after top-level object");
    }

    return true;
}

static bool PointInRegion(const Region *region, int x, int y)
{
    return
        x >= region->x &&
        y >= region->y &&
        x < region->x + region->width &&
        y < region->y + region->height;
}

static bool CellInRoomRegions(const SiteData *site, int room, int x, int y)
{
    int index;

    for(index = 0; index < site->region_count; ++index)
    {
        const Region *region = &site->regions[index];

        if(region->room == room && PointInRegion(region, x, y))
        {
            return true;
        }
    }

    return false;
}

static bool NormalizePlacement(SiteData *site, Placement *placement, const char *context)
{
    bool top_left_form = placement->has_x || placement->has_y;
    bool centre_form = placement->has_centre_x || placement->has_centre_y;
    int rotation = 0;

    if(placement->type[0] == '\0')
    {
        SiteMessage(site, true, "%s is missing required 'type'", context);
        return false;
    }

    if(TypeCSymbol(placement->type) == NULL)
    {
        SiteMessage(site, true, "%s uses unknown type '%s'", context, placement->type);
    }

    placement->is_door = strcmp(placement->type, "DOOR") == 0;

    if(!placement->has_width || !placement->has_height)
    {
        SiteMessage(site, true, "%s is missing width or height", context);
        return false;
    }

    if(placement->width <= 0.0 || placement->height <= 0.0)
    {
        SiteMessage(site, true, "%s has non-positive width or height", context);
        return false;
    }

    if(!IsNearlyInteger(placement->width) || !IsNearlyInteger(placement->height))
    {
        SiteMessage(site, true, "%s width and height must use whole Site units", context);
        return false;
    }

    if(top_left_form && centre_form)
    {
        SiteMessage(site, true, "%s mixes x/y with centre_x/centre_y", context);
        return false;
    }

    if(!top_left_form && !centre_form)
    {
        SiteMessage(site, true, "%s needs either x/y or centre_x/centre_y", context);
        return false;
    }

    if(top_left_form)
    {
        if(!placement->has_x || !placement->has_y)
        {
            SiteMessage(site, true, "%s needs both x and y", context);
            return false;
        }

        if(!IsNearlyInteger(placement->x) || !IsNearlyInteger(placement->y))
        {
            SiteMessage(site, true, "%s x/y coordinates must use whole Site units", context);
            return false;
        }

        if(placement->has_rotation && !IsNearlyInteger(placement->rotation))
        {
            SiteMessage(site, true, "%s rotation must be a whole number of degrees", context);
        }

        rotation = placement->has_rotation ? RoundToInt(placement->rotation) : 0;

        if(rotation % 360 != 0)
        {
            SiteMessage(
                site,
                true,
                "%s uses rotation with x/y form; rotated geometry must use centre_x/centre_y",
                context
            );
        }

        placement->rotation_normalized = 0;
        placement->bounds_x = RoundToInt(placement->x);
        placement->bounds_y = RoundToInt(placement->y);
        placement->bounds_width = RoundToInt(placement->width);
        placement->bounds_height = RoundToInt(placement->height);
        placement->centre_x16 =
            placement->bounds_x * F144_FIXED_ONE +
            placement->bounds_width * (F144_FIXED_ONE / 2);
        placement->centre_y16 =
            placement->bounds_y * F144_FIXED_ONE +
            placement->bounds_height * (F144_FIXED_ONE / 2);
        placement->width16 = placement->bounds_width * F144_FIXED_ONE;
        placement->height16 = placement->bounds_height * F144_FIXED_ONE;
    }
    else
    {
        int half_x16;
        int half_y16;
        int x0_16;
        int x1_16;
        int y0_16;
        int y1_16;

        if(!placement->has_centre_x || !placement->has_centre_y)
        {
            SiteMessage(site, true, "%s needs both centre_x and centre_y", context);
            return false;
        }

        if(!placement->has_rotation)
        {
            SiteMessage(site, true, "%s uses centre coordinates but has no rotation", context);
            return false;
        }

        if(!IsNearlyInteger(placement->rotation))
        {
            SiteMessage(site, true, "%s rotation must be a whole number of degrees", context);
            return false;
        }

        rotation = RoundToInt(placement->rotation);

        if(rotation % 45 != 0)
        {
            SiteMessage(site, true, "%s rotation must be a multiple of 45 degrees", context);
            return false;
        }

        rotation %= 360;
        if(rotation < 0) rotation += 360;
        placement->rotation_normalized = rotation;

        if(!ToFixed16(placement->centre_x, &placement->centre_x16) ||
           !ToFixed16(placement->centre_y, &placement->centre_y16))
        {
            SiteMessage(site, true, "%s centre coordinates must be multiples of 1/16 Site unit", context);
            return false;
        }

        placement->width16 = RoundToInt(placement->width) * F144_FIXED_ONE;
        placement->height16 = RoundToInt(placement->height) * F144_FIXED_ONE;

        if(rotation == 0 || rotation == 180)
        {
            half_x16 = placement->width16 / 2;
            half_y16 = placement->height16 / 2;
        }
        else if(rotation == 90 || rotation == 270)
        {
            half_x16 = placement->height16 / 2;
            half_y16 = placement->width16 / 2;
        }
        else
        {
            /*
             * For 45-degree diagonals:
             * half AABB extent = (width + height) * sqrt(2) / 4.
             * 182/512 is a deliberately conservative integer approximation.
             */
            int combined = placement->width16 + placement->height16;
            half_x16 = (combined * 182 + 511) / 512;
            half_y16 = half_x16;
        }

        x0_16 = placement->centre_x16 - half_x16;
        x1_16 = placement->centre_x16 + half_x16;
        y0_16 = placement->centre_y16 - half_y16;
        y1_16 = placement->centre_y16 + half_y16;

        placement->bounds_x = FloorDiv16(x0_16);
        placement->bounds_y = FloorDiv16(y0_16);
        placement->bounds_width = CeilDiv16(x1_16) - placement->bounds_x;
        placement->bounds_height = CeilDiv16(y1_16) - placement->bounds_y;
    }

    return true;
}

static bool PlacementInsideSite(const Placement *placement, int width, int height)
{
    return
        placement->bounds_x >= 0 &&
        placement->bounds_y >= 0 &&
        placement->bounds_width > 0 &&
        placement->bounds_height > 0 &&
        placement->bounds_x + placement->bounds_width <= width &&
        placement->bounds_y + placement->bounds_height <= height;
}

static bool PlacementCoveredByRoomRegions(const SiteData *site, const Placement *placement)
{
    int x;
    int y;

    for(y = placement->bounds_y; y < placement->bounds_y + placement->bounds_height; ++y)
    {
        for(x = placement->bounds_x; x < placement->bounds_x + placement->bounds_width; ++x)
        {
            if(!CellInRoomRegions(site, placement->room, x, y))
            {
                return false;
            }
        }
    }

    return true;
}

static bool PlacementContainsCell(const Placement *placement, int x, int y)
{
    return
        x >= placement->bounds_x &&
        y >= placement->bounds_y &&
        x < placement->bounds_x + placement->bounds_width &&
        y < placement->bounds_y + placement->bounds_height;
}

static bool SpawnInsideFloor(const SiteData *site, int room, int x16, int y16)
{
    int x = x16 / F144_FIXED_ONE;
    int y = y16 / F144_FIXED_ONE;
    int index;

    for(index = 0; index < site->placement_count; ++index)
    {
        const Placement *placement = &site->placements[index];

        if(
            placement->room == room &&
            IsFloorType(placement->type) &&
            PlacementContainsCell(placement, x, y)
        )
        {
            return true;
        }
    }

    return false;
}

static void ValidateRegions(SiteData *site)
{
    int index;

    for(index = 0; index < site->region_count; ++index)
    {
        Region *region = &site->regions[index];

        if(region->width <= 0 || region->height <= 0)
        {
            SiteMessage(site, true, "%s region has non-positive size", g_rooms[region->room].json_name);
        }

        if(
            region->x < 0 ||
            region->y < 0 ||
            region->x + region->width > site->site_width ||
            region->y + region->height > site->site_height
        )
        {
            SiteMessage(
                site,
                true,
                "%s region (%d,%d %dx%d) leaves the %dx%d Site",
                g_rooms[region->room].json_name,
                region->x,
                region->y,
                region->width,
                region->height,
                site->site_width,
                site->site_height
            );
        }
    }
}

static void ValidatePlacementList(SiteData *site, Placement *placements, int count, bool shared)
{
    int index;

    for(index = 0; index < count; ++index)
    {
        Placement *placement = &placements[index];
        char context[256];

        if(placement->has_id)
        {
            snprintf(context, sizeof(context), "placement '%s'", placement->id);
        }
        else if(shared)
        {
            snprintf(context, sizeof(context), "shared geometry #%d", index + 1);
        }
        else if(placement->room >= 0 && placement->room < F144_MAX_ROOMS)
        {
            snprintf(
                context,
                sizeof(context),
                "%s geometry #%d",
                g_rooms[placement->room].json_name,
                index + 1
            );
        }
        else
        {
            snprintf(context, sizeof(context), "geometry #%d", index + 1);
        }

        NormalizePlacement(site, placement, context);

        if(!PlacementInsideSite(placement, site->site_width, site->site_height))
        {
            SiteMessage(
                site,
                true,
                "%s bounds (%d,%d %dx%d) leave the Site",
                context,
                placement->bounds_x,
                placement->bounds_y,
                placement->bounds_width,
                placement->bounds_height
            );
        }

        if(!shared)
        {
            if(placement->room < 0 || placement->room >= F144_MAX_ROOMS)
            {
                SiteMessage(site, true, "%s has no valid owning room", context);
            }
            else if(!PlacementCoveredByRoomRegions(site, placement))
            {
                if(placement->rotation_normalized != 0)
                {
                    SiteMessage(
                        site,
                        false,
                        "%s conservative rotated bounds cross outside %s region",
                        context,
                        g_rooms[placement->room].json_name
                    );
                }
                else
                {
                    SiteMessage(
                        site,
                        true,
                        "%s lies outside the declared %s region",
                        context,
                        g_rooms[placement->room].json_name
                    );
                }
            }

            if(placement->is_door)
            {
                SiteMessage(
                    site,
                    true,
                    "%s is a DOOR inside room geometry; doors belong in shared_geometry with from/to",
                    context
                );
            }
        }
        else if(placement->is_door)
        {
            int from_room;
            int to_room;

            if(!placement->has_id)
            {
                SiteMessage(site, true, "%s DOOR requires an id", context);
            }

            if(!placement->has_from || !placement->has_to)
            {
                SiteMessage(site, true, "%s DOOR requires from and to", context);
            }
            else
            {
                from_room = RoomIndex(placement->from);
                to_room = RoomIndex(placement->to);

                if(from_room == F144_ROOM_INVALID)
                {
                    SiteMessage(site, true, "%s has unknown door endpoint '%s'", context, placement->from);
                }

                if(to_room == F144_ROOM_INVALID)
                {
                    SiteMessage(site, true, "%s has unknown door endpoint '%s'", context, placement->to);
                }

                if(from_room == F144_ROOM_OUTSIDE && to_room == F144_ROOM_OUTSIDE)
                {
                    SiteMessage(site, true, "%s cannot connect OUTSIDE to OUTSIDE", context);
                }
            }
        }
        else if(placement->has_from || placement->has_to)
        {
            SiteMessage(site, false, "%s has from/to fields but is not a DOOR; they will be ignored", context);
        }
    }
}

static void ValidateUniqueIdsAndObjects(SiteData *site)
{
    const Placement *all[F144_MAX_IDS];
    int count = 0;
    int i;
    int j;

    for(i = 0; i < site->placement_count && count < F144_MAX_IDS; ++i)
    {
        all[count++] = &site->placements[i];
    }

    for(i = 0; i < site->shared_count && count < F144_MAX_IDS; ++i)
    {
        all[count++] = &site->shared[i];
    }

    for(i = 0; i < count; ++i)
    {
        if(all[i]->has_id)
        {
            for(j = i + 1; j < count; ++j)
            {
                if(all[j]->has_id && strcmp(all[i]->id, all[j]->id) == 0)
                {
                    SiteMessage(site, true, "duplicate placement id '%s'", all[i]->id);
                    break;
                }
            }
        }

        if(all[i]->has_object)
        {
            for(j = i + 1; j < count; ++j)
            {
                if(all[j]->has_object && strcmp(all[i]->object_ref, all[j]->object_ref) == 0)
                {
                    SiteMessage(
                        site,
                        false,
                        "object reference '%s' is used by more than one placement",
                        all[i]->object_ref
                    );
                    break;
                }
            }
        }
    }
}

static void ValidateFloorOwnership(SiteData *site)
{
    int owner[F144_SITE_SIZE][F144_SITE_SIZE];
    int x;
    int y;
    int index;
    int room;

    for(y = 0; y < F144_SITE_SIZE; ++y)
    {
        for(x = 0; x < F144_SITE_SIZE; ++x)
        {
            owner[y][x] = F144_ROOM_INVALID;
        }
    }

    for(index = 0; index < site->placement_count; ++index)
    {
        Placement *placement = &site->placements[index];

        if(!IsFloorType(placement->type))
        {
            continue;
        }

        if(placement->rotation_normalized != 0)
        {
            SiteMessage(site, true, "floor geometry cannot be rotated");
            continue;
        }

        if(placement->room >= 0 && placement->room < F144_MAX_ROOMS)
        {
            ++site->rooms[placement->room].floor_count;
        }

        for(y = placement->bounds_y; y < placement->bounds_y + placement->bounds_height; ++y)
        {
            for(x = placement->bounds_x; x < placement->bounds_x + placement->bounds_width; ++x)
            {
                if(x < 0 || y < 0 || x >= F144_SITE_SIZE || y >= F144_SITE_SIZE)
                {
                    continue;
                }

                if(owner[y][x] != F144_ROOM_INVALID)
                {
                    SiteMessage(
                        site,
                        true,
                        "floor cell (%d,%d) is claimed by both %s and %s",
                        x,
                        y,
                        g_rooms[owner[y][x]].json_name,
                        g_rooms[placement->room].json_name
                    );
                }
                else
                {
                    owner[y][x] = placement->room;
                }
            }
        }
    }

    for(room = 0; room < F144_MAX_ROOMS; ++room)
    {
        if(site->rooms[room].floor_count == 0)
        {
            SiteMessage(site, true, "room '%s' has no FLOOR_* geometry", g_rooms[room].json_name);
        }
    }
}

static bool ValidateSite(SiteData *site)
{
    int room;
    int spawn_room;
    int spawn_x16 = 0;
    int spawn_y16 = 0;

    if(!site->has_schema_version)
    {
        SiteMessage(site, true, "missing schema_version");
    }
    else if(site->schema_version != F144_SCHEMA_VERSION)
    {
        SiteMessage(site, true, "unsupported schema_version %d (expected %d)", site->schema_version, F144_SCHEMA_VERSION);
    }

    if(!site->has_site_width || !site->has_site_height)
    {
        SiteMessage(site, true, "site.width and site.height are required");
    }
    else if(site->site_width != F144_SITE_SIZE || site->site_height != F144_SITE_SIZE)
    {
        SiteMessage(site, true, "Floppy//144 Site must be exactly 100 x 100 units");
    }

    for(room = 0; room < F144_MAX_ROOMS; ++room)
    {
        if(!site->rooms[room].present)
        {
            SiteMessage(site, true, "canonical room '%s' is missing", g_rooms[room].json_name);
        }

        if(site->rooms[room].present && site->rooms[room].region_count == 0)
        {
            SiteMessage(site, true, "room '%s' has no regions", g_rooms[room].json_name);
        }
    }

    ValidateRegions(site);
    ValidatePlacementList(site, site->placements, site->placement_count, false);
    ValidatePlacementList(site, site->shared, site->shared_count, true);
    ValidateUniqueIdsAndObjects(site);
    ValidateFloorOwnership(site);

    if(!site->has_spawn_x || !site->has_spawn_y || !site->has_spawn_room)
    {
        SiteMessage(site, true, "site.spawn requires x, y and room");
    }
    else
    {
        spawn_room = RoomIndex(site->spawn_room);

        if(spawn_room < 0)
        {
            SiteMessage(site, true, "spawn uses unknown room '%s'", site->spawn_room);
        }

        if(!ToFixed16(site->spawn_x, &spawn_x16) || !ToFixed16(site->spawn_y, &spawn_y16))
        {
            SiteMessage(site, true, "spawn coordinates must be multiples of 1/16 Site unit");
        }
        else if(
            spawn_x16 < 0 ||
            spawn_y16 < 0 ||
            spawn_x16 >= site->site_width * F144_FIXED_ONE ||
            spawn_y16 >= site->site_height * F144_FIXED_ONE
        )
        {
            SiteMessage(site, true, "spawn position lies outside the Site");
        }
        else if(spawn_room >= 0 && !SpawnInsideFloor(site, spawn_room, spawn_x16, spawn_y16))
        {
            SiteMessage(
                site,
                true,
                "spawn position (%.4g, %.4g) is not on %s FLOOR_* geometry",
                site->spawn_x,
                site->spawn_y,
                g_rooms[spawn_room].json_name
            );
        }
    }

    return site->errors == 0;
}

static void WritePlacementComment(FILE *output, const Placement *placement)
{
    if(!placement->has_id && !placement->has_variant && !placement->has_object)
    {
        return;
    }

    fputs("/*", output);

    if(placement->has_id)
    {
        fprintf(output, " id=%s", placement->id);
    }

    if(placement->has_variant)
    {
        fprintf(output, " variant=%s", placement->variant);
    }

    if(placement->has_object)
    {
        fprintf(output, " object=%s", placement->object_ref);
    }

    fputs(" */\n", output);
}

static bool WriteGeneratedFile(const SiteData *site, const char *input_path, const char *output_path)
{
    FILE *output = fopen(output_path, "wb");
    int room;
    int index;
    int spawn_room;
    int spawn_x16;
    int spawn_y16;
    int running_region = 0;

    if(output == NULL)
    {
        fprintf(stderr, "ERROR: cannot create output '%s'\n", output_path);
        return false;
    }

    ToFixed16(site->spawn_x, &spawn_x16);
    ToFixed16(site->spawn_y, &spawn_y16);
    spawn_room = RoomIndex(site->spawn_room);

    fprintf(
        output,
        "/*\n"
        " * AUTO-GENERATED FILE. DO NOT EDIT.\n"
        " *\n"
        " * Source: %s\n"
        " * Generator: tools/site_compiler.c\n"
        " * Schema: %d\n"
        " *\n"
        " * The JSONC file is the single authored Site source of truth.\n"
        " */\n\n",
        input_path,
        site->schema_version
    );

    fprintf(
        output,
        "SITE_META(%dU, %dU, %d, %d, %s)\n\n",
        site->site_width,
        site->site_height,
        spawn_x16,
        spawn_y16,
        RoomCSymbol(spawn_room)
    );

    fputs("/* Room view/camera regions. */\n", output);

    for(room = 0; room < F144_MAX_ROOMS; ++room)
    {
        int room_region_count = 0;

        fprintf(output, "\n/* %s */\n", g_rooms[room].json_name);

        for(index = 0; index < site->region_count; ++index)
        {
            const Region *region = &site->regions[index];

            if(region->room != room)
            {
                continue;
            }

            fprintf(
                output,
                "SITE_ROOM_REGION(%s, %dU, %dU, %dU, %dU)\n",
                RoomCSymbol(room),
                region->x,
                region->y,
                region->width,
                region->height
            );

            ++room_region_count;
        }

        fprintf(
            output,
            "SITE_ROOM_DEF(%s, %dU, %dU)\n",
            RoomCSymbol(room),
            running_region,
            room_region_count
        );

        running_region += room_region_count;
    }

    fputs("\n/* Room-owned geometry. FLOOR_* entries are authoritative room ownership. */\n", output);

    for(room = 0; room < F144_MAX_ROOMS; ++room)
    {
        fprintf(output, "\n/* %s */\n", g_rooms[room].json_name);

        for(index = 0; index < site->placement_count; ++index)
        {
            const Placement *placement = &site->placements[index];
            const char *type_symbol;

            if(placement->room != room)
            {
                continue;
            }

            type_symbol = TypeCSymbol(placement->type);
            if(type_symbol == NULL) type_symbol = "FLOPPY144_SITE_ELEMENT_COUNT";

            WritePlacementComment(output, placement);

            if(placement->rotation_normalized == 0)
            {
                fprintf(
                    output,
                    "SITE_GEOMETRY(%s, %s, %dU, %dU, %dU, %dU)\n",
                    RoomCSymbol(room),
                    type_symbol,
                    placement->bounds_x,
                    placement->bounds_y,
                    placement->bounds_width,
                    placement->bounds_height
                );
            }
            else
            {
                fprintf(
                    output,
                    "SITE_ROTATED_GEOMETRY(%s, %s, %dU, %dU, %dU, %dU, %d, %d, %d, %d, %d)\n",
                    RoomCSymbol(room),
                    type_symbol,
                    placement->bounds_x,
                    placement->bounds_y,
                    placement->bounds_width,
                    placement->bounds_height,
                    placement->centre_x16,
                    placement->centre_y16,
                    placement->width16,
                    placement->height16,
                    placement->rotation_normalized
                );
            }
        }
    }

    fputs("\n/* Shared geometry and door topology. */\n", output);

    for(index = 0; index < site->shared_count; ++index)
    {
        const Placement *placement = &site->shared[index];
        const char *type_symbol = TypeCSymbol(placement->type);

        if(type_symbol == NULL) type_symbol = "FLOPPY144_SITE_ELEMENT_COUNT";

        WritePlacementComment(output, placement);

        if(placement->is_door)
        {
            int from_room = RoomIndex(placement->from);
            int to_room = RoomIndex(placement->to);

            fprintf(
                output,
                "SITE_DOOR(%s, %s, %dU, %dU, %dU, %dU)\n",
                RoomCSymbol(from_room),
                RoomCSymbol(to_room),
                placement->bounds_x,
                placement->bounds_y,
                placement->bounds_width,
                placement->bounds_height
            );
        }
        else if(placement->rotation_normalized == 0)
        {
            fprintf(
                output,
                "SITE_SHARED(%s, %dU, %dU, %dU, %dU)\n",
                type_symbol,
                placement->bounds_x,
                placement->bounds_y,
                placement->bounds_width,
                placement->bounds_height
            );
        }
        else
        {
            fprintf(
                output,
                "SITE_ROTATED_SHARED(%s, %dU, %dU, %dU, %dU, %d, %d, %d, %d, %d)\n",
                type_symbol,
                placement->bounds_x,
                placement->bounds_y,
                placement->bounds_width,
                placement->bounds_height,
                placement->centre_x16,
                placement->centre_y16,
                placement->width16,
                placement->height16,
                placement->rotation_normalized
            );
        }
    }

    fclose(output);
    return true;
}

static char *ReadEntireFile(const char *path, size_t *length)
{
    FILE *file = fopen(path, "rb");
    long size;
    char *data;
    size_t read_count;

    if(file == NULL)
    {
        return NULL;
    }

    if(fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return NULL;
    }

    size = ftell(file);
    if(size < 0)
    {
        fclose(file);
        return NULL;
    }

    if(fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return NULL;
    }

    data = (char *)malloc((size_t)size + 1U);
    if(data == NULL)
    {
        fclose(file);
        return NULL;
    }

    read_count = fread(data, 1U, (size_t)size, file);
    fclose(file);

    if(read_count != (size_t)size)
    {
        free(data);
        return NULL;
    }

    data[read_count] = '\0';
    *length = read_count;
    return data;
}

static void PrintAudit(const SiteData *site, const char *input_path, const char *output_path, bool generated)
{
    int rooms = 0;
    int doors = 0;
    int rotated = 0;
    int object_refs = 0;
    int floors = 0;
    int room;
    int index;

    for(room = 0; room < F144_MAX_ROOMS; ++room)
    {
        if(site->rooms[room].present) ++rooms;
    }

    for(index = 0; index < site->placement_count; ++index)
    {
        if(IsFloorType(site->placements[index].type)) ++floors;
        if(site->placements[index].rotation_normalized != 0) ++rotated;
        if(site->placements[index].has_object) ++object_refs;
    }

    for(index = 0; index < site->shared_count; ++index)
    {
        if(site->shared[index].is_door) ++doors;
        if(site->shared[index].rotation_normalized != 0) ++rotated;
        if(site->shared[index].has_object) ++object_refs;
    }

    printf("\n=== FLOPPY//144 SITE COMPILER ===\n");
    printf("Input:             %s\n", input_path);
    printf("Schema:            %d\n", site->schema_version);
    printf("Site:              %d x %d\n", site->site_width, site->site_height);
    printf("Rooms:             %d / %d\n", rooms, F144_MAX_ROOMS);
    printf("Room regions:      %d\n", site->region_count);
    printf("Room geometry:     %d\n", site->placement_count);
    printf("Floor rectangles:  %d\n", floors);
    printf("Shared geometry:   %d\n", site->shared_count);
    printf("Doors:             %d\n", doors);
    printf("Rotated items:     %d\n", rotated);
    printf("Object hooks:      %d\n", object_refs);
    printf("Warnings:          %d\n", site->warnings);
    printf("Errors:            %d\n", site->errors);

    if(generated)
    {
        printf("Generated:         %s\n", output_path);
        printf("RESULT:            PASS\n");
    }
    else
    {
        printf("Generated:         (none)\n");
        printf("RESULT:            FAIL\n");
    }

    printf("==================================\n");
}

int main(int argc, char **argv)
{
    const char *input_path = "site_layout.jsonc";
    const char *output_path = "game/src/floppy144_site_generated.def";
    char *text;
    size_t length = 0U;
    SiteData *site;
    bool parsed;
    bool valid;
    bool generated = false;

    if(argc > 3)
    {
        fprintf(stderr, "Usage: site_compiler [input.jsonc] [output.def]\n");
        return 2;
    }

    if(argc >= 2)
    {
        input_path = argv[1];
    }

    if(argc >= 3)
    {
        output_path = argv[2];
    }

    site = (SiteData *)calloc(1U, sizeof(*site));
    if(site == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n");
        return 2;
    }

    text = ReadEntireFile(input_path, &length);

    if(text == NULL)
    {
        fprintf(stderr, "ERROR: cannot read input '%s'\n", input_path);
        free(site);
        return 2;
    }

    parsed = ParseDocument(site, text, length);
    free(text);

    if(!parsed)
    {
        PrintAudit(site, input_path, output_path, false);
        free(site);
        return 1;
    }

    valid = ValidateSite(site);

    if(valid)
    {
        generated = WriteGeneratedFile(site, input_path, output_path);
        if(!generated)
        {
            ++site->errors;
        }
    }

    PrintAudit(site, input_path, output_path, generated);

    free(site);
    return generated ? 0 : 1;
}
