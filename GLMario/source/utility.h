/**********************************************
 *
 * Cross platform utility functions
 *
 ***************/


#pragma once

#include "types.h"

#include <fstream>
#include <sys/stat.h>

/**********************************************
 *
 * File functions
 *
 ***************/

char* load_text_file(char*);



/**********************************************
 *
 * Memory functions
 *
 ***************/

#define ArrayCount(array) sizeof(array) / sizeof(array[0])

#define NEW_ZERO(var, type) type* var = new type; \
                            memset(var, 0, sizeof(*var))

#define NEW_ARRAY_ZERO(var, type, count) type* var = new type[count];   \
                                   memset(var, 0, count * sizeof(*var))


struct MemoryArena
{
    uint32 size;
    uint32 used;
    uint8* base;
};


void AllocateMemoryArena(MemoryArena*, size_t);

void FreeMemoryArena(MemoryArena*);

#define PushStruct(type, arena, clear) (type*) PushSize(arena, sizeof(type), clear)

uint8* PushSize(MemoryArena* arena, bool clear);

