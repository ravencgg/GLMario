#include "utility.h"

#include <windows.h>
#include <assert.h>


/**********************************************
 *
 * File functions
 *
 ***************/

char* load_text_file(char* filename)
{
	char* result = nullptr;

	struct stat st;

	if (stat(filename, &st) == 0)
	{
		result = new char[st.st_size + 1]; // NOTE(chris): This does not clear to zero, and the + 1 is for the null terminator at the end of it.
		result[st.st_size] = '\0'; // This null terminator

		std::ifstream input(filename, std::ios::binary);

		if (input.is_open())
		{
			input.read(result, st.st_size);
			input.close();
		}
	}

	return result;
}


/**********************************************
 *
 * Memory functions
 *
 ***************/


void AllocateMemoryArena(MemoryArena* arena, size_t size)
{
    arena->size = size;
    arena->used = 0;
    arena->base = (uint8*) VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void FreeMemoryArena(MemoryArena* arena)
{
    VirtualFree(arena->base, 0, MEM_RELEASE);
}

uint8* PushSize(MemoryArena* arena, size_t size, bool clear)
{
    assert(arena->size - arena->used > size);

    if(arena->size - arena->used > size)
    {
        uint8* result = arena->base + arena->used;
        arena->used += size;

        if(clear)
        {
            memset(result, 0, size);
        }

        return result;
    }

    return nullptr;
}


/**********************************************
 *
 * Clock Functions
 *
 ***************/

uint64 GetCycleCount()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return (uint64)result.QuadPart;
}

uint64 GetCyclesPerSecond()
{
    LARGE_INTEGER result;
    QueryPerformanceFrequency(&result);
    return (uint64)result.QuadPart;
}




