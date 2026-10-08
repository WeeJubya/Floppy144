#include "floppy144_variation.h"

#include <stddef.h>

/* FNV-1a 32-bit folding; every operation is defined unsigned arithmetic. */
static uint32_t Floppy144VariationFold(uint32_t value, uint8_t byte)
{
    return (value ^ (uint32_t)byte) * 16777619U;
}

uint32_t Floppy144VariationValue(
    uint32_t recovery_seed, const char *feature, const char *item_id
)
{
    uint32_t value = 2166136261U;
    uint32_t index;
    const unsigned char *text;

    if(feature == NULL || feature[0] == '\0')
    {
        return 0U;
    }

    /* Fixed byte order, version/domain tag and NUL-separated namespaces. */
    for(index = 0U; index < 4U; ++index)
    {
        value = Floppy144VariationFold(
            value, (uint8_t)(recovery_seed >> (index * 8U))
        );
    }
    value = Floppy144VariationFold(value, 0xA5U);
    for(text = (const unsigned char *)feature; *text != 0U; ++text)
    {
        value = Floppy144VariationFold(value, *text);
    }
    value = Floppy144VariationFold(value, 0U);
    if(item_id != NULL)
    {
        for(text = (const unsigned char *)item_id; *text != 0U; ++text)
        {
            value = Floppy144VariationFold(value, *text);
        }
    }
    value = Floppy144VariationFold(value, 0U);

    /* Stateless 32-bit avalanche, not a global sequential PRNG. */
    value ^= value >> 16U;
    value *= 0x7feb352dU;
    value ^= value >> 15U;
    value *= 0x846ca68bU;
    value ^= value >> 16U;
    return value;
}

uint32_t Floppy144VariationRange(
    uint32_t recovery_seed, const char *feature, const char *item_id,
    uint32_t count
)
{
    if(count == 0U || feature == NULL || feature[0] == '\0')
    {
        return 0U;
    }
    /* Modulo bias is acceptable for small flavour-only option sets. */
    return Floppy144VariationValue(recovery_seed, feature, item_id) % count;
}

bool Floppy144VariationChance(
    uint32_t recovery_seed, const char *feature, const char *item_id,
    uint32_t numerator, uint32_t denominator
)
{
    if(
        denominator == 0U || numerator == 0U ||
        feature == NULL || feature[0] == '\0'
    )
    {
        return false;
    }
    if(numerator >= denominator)
    {
        return true;
    }
    return Floppy144VariationRange(
        recovery_seed, feature, item_id, denominator
    ) < numerator;
}
