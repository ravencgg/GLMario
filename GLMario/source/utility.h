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



#define ArrayCount(array) sizeof(array) / sizeof(array[0])

#define NEW_ZERO(var, type) type* var = new type; \
                            memset(var, 0, sizeof(*var))

#define NEW_ARRAY_ZERO(var, type, count) type* var = new type[count];   \
                                   memset(var, 0, count * sizeof(*var))


struct MemoryArena
{
    size_t size;
    size_t used;
    uint8* base;
};


void AllocateMemoryArena(MemoryArena*, size_t);

void FreeMemoryArena(MemoryArena*);

#define PushStruct(type, arena, clear) (type*) PushSize(arena, sizeof(type), clear)

uint8* PushSize(MemoryArena* arena, bool clear);


/**********************************************
 *
 * High Performance Clock
 *
 ***************/


uint64 GetCycleCount();

uint64 GetCyclesPerSecond();

