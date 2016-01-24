#include "utility.h"

#include <Windows.h>
#include <assert.h>
#include <random>

#include "types.h"

/**********************************************
 *
 * File functions
 *
 ***************/

// Returns a pointer to the data
// Doesn't work on files > 4 gigs
uint8* LoadDataFile(MemoryArena* arena, uint32* out_size, char* filename)
{
    uint8* result = 0;
    HANDLE file = 0;

    file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    LARGE_INTEGER file_size = {};

    if(file != INVALID_HANDLE_VALUE)
    {
        GetFileSizeEx(file, &file_size);
        *out_size = file_size.LowPart;

        DWORD bytes_read = 0;
        result = PushSize(arena, *out_size, false);
        ReadFile(file, result, *out_size, &bytes_read, 0);

        CloseHandle(file);
        assert(file_size.HighPart == 0);
        return result;
    }

    *out_size = 0;
    return nullptr;
};

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
    // TODO: Platform layer call
    arena->base = (uint8*) VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

MemoryArena CreateSubArena(MemoryArena* super, size_t size)
{
    MemoryArena result;
    result.size = size;
    result.used = 0;
    result.base = PushSize(super, size, false);

    return result;
}

void FreeMemoryArena(MemoryArena* arena)
{
    VirtualFree(arena->base, 0, MEM_RELEASE);
}

uint8* PushSize(MemoryArena* arena, size_t size, bool clear, uint32 byte_alignment)
{
    if(size == 0)
    {
        return nullptr;
    }
    assert((byte_alignment == 0 || !(byte_alignment & (byte_alignment - 1))));

    uint8* result = arena->base + arena->used;
    uint32 align = byte_alignment - 1;
    uint32 result_offset = byte_alignment ? (byte_alignment - ((size_t)result & align) & align) : 0;

    assert(arena->size - arena->used >= size + result_offset);
    if (arena->size - arena->used >= size + result_offset)
    {
        result += result_offset;
        arena->used += size + result_offset;
    }
    else
    {
        result = nullptr;
    }


    if(clear)
    {
        memset(result, 0, size);
    }

    return result;
}

// NOTE: Could fill the memory with obvious garbage in these resetting functions
// to track deallocation problems
void PopAllocation(MemoryArena* arena, void* position)
{
    assert(position >= arena->base && position < (arena->base + arena->size));
    assert(position < arena->base + arena->used);
    arena->used = (size_t)position - (size_t)arena->base;
    assert(arena->used >= 0 && arena->used < arena->size);
}

void ClearArena(MemoryArena* arena)
{
    arena->used = 0;
}

void ResetArena(MemoryArena* arena)
{
    arena->used = 0;
}


/**********************************************
 *
 * String
 *
 ***************/

size_t StrPrintf(char* dest, size_t len, char* format, ...)
{
    if(len == 0) return 0;

    va_list args;
    va_start(args, format);
    // NOTE: return value does not count the terminating null
    // NOTE: Windows only, linux OSes can use vsnprintf with a NULL buffer to find the required size
    int32 input_size = _vscprintf(format, args) + 1; // Add 1 for the '\n'
    va_end(args);

    size_t buffer_size = Minimum(input_size, len);

    va_start(args, format);
    size_t written = _vsnprintf_s(dest, buffer_size, buffer_size, format, args);
    va_end(args);

    return written;
}


size_t StrSkipWhitespace(char** string, uint32 tab_size)
{
    assert(string);
    char* c = *string;

    uint32 skipped = 0;
    while(*c)
    {
        if(*c == ' ')
        {
            ++skipped;
            ++c;
        }
        else if(*c == '\t')
        {
            skipped += tab_size;
            ++c;
        }
        else
        {
            break;
        }
    }

    *string = c;
    return skipped;
}

size_t StrCountTrailingWhitespace(char* string, uint32 max_check, uint32* num_tabs)
{
    assert(string);
    char* c = string;

    uint32 tab_counter = 0;
    while(*c && --max_check)
    {
        if(*c == ' ')
        {
            --c;
        }
        else if(*c == '\t')
        {
            tab_counter++;
            --c;
        }
        else
        {
            break;
        }
    }

    OPTIONAL_ASSIGN(num_tabs, tab_counter);
    return size_t(string - c);
}



// Points at the last character of a token
char* StrGetToken(char* string, size_t* token_size, char* whitespace)
{
    if(!string) return nullptr;
    char* token_end = string;

    size_t num_whitespace_chars = strlen(whitespace);
    if(!string || !num_whitespace_chars) return nullptr;

    while(*token_end)
    {
        for(uint32 i = 0; i < num_whitespace_chars; ++i)
        {
            if(*token_end == whitespace[i])
            {
                goto done;
            }
        }
        ++token_end;
    }

done:
    OPTIONAL_ASSIGN(token_size, size_t(token_end - string));
    return token_end;
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


/**********************************************
 *
 * Random Number Generation
 *
 ***************/

float random_float(float x_min, float x_max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(0, 1);
    float result = dist(mt);
    result *= (x_max - x_min); // range;
    result += x_min;
    return result;
}
