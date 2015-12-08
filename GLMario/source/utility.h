#pragma once

/**********************************************
 *
 * Cross platform utility functions
 *
 ***************/

#include "types.h"

#include <fstream>
#include <sys/stat.h>

/**********************************************
 *
 * File
 *
 ***************/

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
 * Memory
 *
 ***************/

template <typename T>
T* ExpandArray(T* array, uint32 start_count, uint32 new_count)
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

void FreeMemoryArena(MemoryArena*);

uint8* PushSize(MemoryArena* arena, size_t size, bool clear = true);

// Clears the memory as well, since PushSize clears by default
#define PushStructs(arena, type, count) (type*) PushSize(arena, sizeof(type) * (count))
#define PushStruct(arena, type)         (type*) PushSize(arena, sizeof(type))


/**********************************************
 *
 * High Performance Clock
 *
 ***************/


uint64 GetCycleCount();

uint64 GetCyclesPerSecond();

