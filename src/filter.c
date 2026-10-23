#include "filter.h"

#include <stdlib.h>

typedef struct
{
    uint8_t age;       // 0 is niewste 
    uint16_t value;
} FilterSample;

static FilterSample *samples = NULL;
static uint8_t size = 0;
static bool filled = false;      // start op vals zodat eerste waarde alles opvult

static int CompareValue(const void *a, const void *b)
{
    uint16_t x = ((const FilterSample *)a)->value;
    uint16_t y = ((const FilterSample *)b)->value;

    return (x > y) - (x < y);
}

static int CompareAge(const void *a, const void *b)
{
    uint8_t x = ((const FilterSample *)a)->age;
    uint8_t y = ((const FilterSample *)b)->age;

    return (x > y) - (x < y);
}

// quicksort om te sorteren en pakt middeslte waarde
static uint16_t Median(void)
{
    qsort(samples, size, sizeof(FilterSample), CompareValue);
    return samples[size / 2].value;
}

static bool IsValidSize(uint8_t new_size)
{
    return new_size >= FILTER_MIN_SIZE &&
           new_size <= FILTER_MAX_SIZE &&
           (new_size % 2) == 1;
}

bool InitFilter(uint8_t new_size)
{
    FreeFilter();

    if (!IsValidSize(new_size))
    {
        return false;
    }

    samples = malloc(new_size * sizeof(FilterSample));
    if (samples == NULL)
    {
        return false;
    }

    size = new_size;
    filled = false;
    return true;
}

uint16_t FilterDistance(uint16_t distance)
{
    if (samples == NULL)
    {
        return distance;
    }

    if (!filled)
    {
        // eerste waarde vult alles
        for (uint8_t i = 0; i < size; i++)
        {
            samples[i].age = i;
            samples[i].value = distance;
        }
        filled = true;
        return distance;
    }

    // oudste waarde vinden
    uint8_t oldest = 0;
    for (uint8_t i = 1; i < size; i++)
    {
        if (samples[i].age > samples[oldest].age)
        {
            oldest = i;
        }
    }

    // age + 1 
    for (uint8_t i = 0; i < size; i++)
    {
        samples[i].age++;
    }

    // oudste waarde wordt overschreven
    samples[oldest].age = 0;
    samples[oldest].value = distance;

    return Median();
}

bool SetFilterSize(uint8_t new_size)
{
    if (!IsValidSize(new_size))
    {
        return false;
    }

    if (samples == NULL)
    {
        return InitFilter(new_size);
    }

    if (new_size == size)
    {
        return true;
    }

    if (new_size < size)
    {
        // alleen nieuwste houden
        qsort(samples, size, sizeof(FilterSample), CompareAge);

        FilterSample *smaller = realloc(samples, new_size * sizeof(FilterSample));
        if (smaller == NULL)
        {
            return false;                
        }
        samples = smaller;
        size = new_size;
        return true;
    }

    uint16_t fill_value = filled ? Median() : 0;

    FilterSample *bigger = realloc(samples, new_size * sizeof(FilterSample));
    if (bigger == NULL)
    {
        return false;                    
    }

    if (filled)
    {
        for (uint8_t i = size; i < new_size; i++)
        {
            bigger[i].age = i;           
            bigger[i].value = fill_value;
        }
    }

    samples = bigger;
    size = new_size;
    return true;
}

uint8_t GetFilterSize(void)
{
    return size;
}

uint8_t NextFilterSize(uint8_t current, int8_t step)
{
    int16_t next = (int16_t)current + 2 * step;

    if (next < FILTER_MIN_SIZE)
    {
        return FILTER_MIN_SIZE;
    }
    if (next > FILTER_MAX_SIZE)
    {
        return FILTER_MAX_SIZE;
    }
    return (uint8_t)next;
}

void ResetFilter(void)
{
    filled = false;
}

void FreeFilter(void)
{
    free(samples);
    samples = NULL;
    size = 0;
    filled = false;
}
