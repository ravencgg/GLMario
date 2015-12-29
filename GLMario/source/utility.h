/**********************************************
 *
 * Cross platform utility functions
 *
 ***************/

#pragma once


#include "types.h"
#include <assert.h>


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
#define PushStructs(arena, type, count) (type*) PushSize(arena, sizeof(type) * (count)) // @deprecate
#define PushStruct(arena, type)         (type*) PushSize(arena, sizeof(type))

struct AutoPopAllocation
{
    MemoryArena* arena;
    void* base;
    size_t size;

    AutoPopAllocation(MemoryArena* a) : arena(a), base(nullptr), size(0) { }

    ~AutoPopAllocation()
    {
        if(base && size)
        {
            PopAllocation(arena, base);
        }
    }
};

#define __PushArrayScopeHelper(dest, arena, type, count, line)                 \
        AutoPopAllocation apa_ ##line(arena);                                  \
        apa_ ##line.size = sizeof(type) * (count);                             \
        if(apa_ ##line.size) {                                                 \
            apa_ ##line.base = (void*) PushSize(arena, apa_ ##line.size);      \
            dest = (type*) apa_ ##line.base;                                   \
        } else {                                                               \
            dest = nullptr;                                                    \
        }

#define _PushArrayScopeHelper(dest, arena, type, count, line) __PushArrayScopeHelper(dest, arena, type, count, line)
#define PushArrayScoped(dest, arena, type, count) _PushArrayScopeHelper(dest, arena, type, count, __LINE__)
//#define PushStructScope assert(!"oopsies");



/**********************************************
 *
 * Scope End
 *
 ***************/

template <typename T>
struct ExitScope
{
    T lambda;
    ExitScope(T lambda): lambda(lambda){ }
    ~ExitScope(){ lambda();}
    ExitScope(const ExitScope&);
private:
    ExitScope& operator =(const ExitScope&);
};

struct ExitScopeHelp
{
    template<typename T>
    ExitScope<T> operator+(T t) { return t; }
};

#define _CONCAT(a, b) a##b     // Needed for this?

#define CONCAT(a, b) _CONCAT(a, b)

#define defer const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()


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
