#pragma once

#include "types.h"


/**********************************************
 *
 * Cross platform utility functions
 *
 ***************/

#include <fstream>
#include <sys/stat.h>

/**********************************************
 *
 * Memory
 *
 ***************/

template <typename T>
T* ExpandArray(T* array, size_t start_count, size_t new_count)
{
    T* result = nullptr;
    if(array)
    {
        result = new T[new_count];
        memcpy(result, array, sizeof(T) * start_count);
        delete[] array;
    }

    assert(result);
    return result;
}

struct MemoryArena
{
    size_t size;
    size_t used;
    uint8* base;
};

#define ArrayCount(array) sizeof(array) / sizeof(array[0])

#define NEW_ZERO(var, type) type* var = new type; \
                            memset(var, 0, sizeof(*var))

#define NEW_ARRAY_ZERO(var, type, count) type* var = new type[count];   \
                                   memset(var, 0, count * sizeof(*var))

void AllocateMemoryArena(MemoryArena*, size_t);

MemoryArena CreateSubArena(MemoryArena* base, size_t size);

// Resets the arena without clearing any of the data since it will be cleared on allocation
void ResetArena(MemoryArena*);

void FreeMemoryArena(MemoryArena*);

uint8* PushSize(MemoryArena* arena, size_t size, bool clear = true);

// Since this is a stack allocation, this will free all allocations made after the supplied pointer's position
void PopAllocation(MemoryArena* arena, void*);

// Clears the memory as well, since PushSize clears by default
#define PushArray(arena, type, count)   (type*) PushSize(arena, sizeof(type) * (count))
#define PushStructs(arena, type, count) (type*) PushSize(arena, sizeof(type) * (count))
#define PushStruct(arena, type)         (type*) PushSize(arena, sizeof(type))



/**********************************************
 *
 * File
 *
 ***************/

uint8* LoadDataFile(MemoryArena*, uint32* out_size, char* filename);

char* load_text_file(char*);

/**********************************************
 *
 * String
 *
 ***************/

#ifdef _MSC_VER
//#define vsnprintf _vsnprintf
#define snprintf  _snprintf
#endif

/**********************************************
 *
 * High Performance Clock
 *
 ***************/


uint64 GetCycleCount();

uint64 GetCyclesPerSecond();


/**********************************************
 *
 * Random Number Generation
 *
 ***************/

float random_float(float x_min, float x_max);
