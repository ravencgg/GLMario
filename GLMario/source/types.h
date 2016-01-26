#pragma once

#include <stdint.h>

typedef float real32;
typedef double real64;
typedef float r32;
typedef double r64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t bool8;
typedef int16_t bool16;
typedef int32_t bool32;

typedef int8_t b8;
typedef int16_t b6;
typedef int32_t b32;

#define NOT_IMPLEMENTED assert(!"Not implemented!");

#define KILOBYTES(kb) (1024ull * kb)
#define MEGABYTES(mb) (1024ull * KILOBYTES(mb))
#define GIGABYTES(gb) (1024ull * MEGABYTES(gb))

#define MIN(a, b) ( (a) < (b) ? (a) : (b) )
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )

#define Maximum(a, b) ( (a) > (b) ? (a) : (b) )
#define Minimum(a, b) ( (a) < (b) ? (a) : (b) )

#define NO_MATRICES 1

#define InvalidDefaultCase default:  \
                            assert(!"Invalid default case!");

#define OPTIONAL_ASSIGN(var, value) if(var) *(var) = (value)

#define DeclareOpaqueHandle(type)  \
struct type;                       \
typedef struct type* type##Handle


struct FRange
{
	float min_range;
	float max_range;

	FRange() : min_range(0), max_range(0) {}
	FRange(float min, float max) : min_range(min), max_range(max) {}
};

struct Rect
{
	union
	{
		struct
		{
			int32 x, y, w, h;
		};
		struct
		{
			int32 left, bot, width, height;
		};
		int32 E[4];
	};

    int32 Left()  { return x; }
    int32 Right() { return x + w; }
    int32 Bot()   { return y; }
    int32 Top()   { return y + h; }
};

struct Rectf
{
	union
	{
		struct
		{
			float x, y, w, h;
		};
		struct
		{
			float left, bot, width, height;
		};
		float E[4];
	};

    float Left()  { return x; }
    float Right() { return x + w; }
    float Bot()   { return y; }
    float Top()   { return y + h; }
};

inline Rect rect(int32 x, int32 y, int32 w, int32 h)
{
	Rect result = {};
	result.x = x;
	result.y = y;
	result.w = w;
	result.h = h;
	return result;
}

inline Rectf rectf(float x, float y, float w, float h)
{
	Rectf result = {};
	result.x = x;
	result.y = y;
	result.w = w;
	result.h = h;
	return result;
}

struct Vec2i
{
    int32 x, y;
};

inline Vec2i operator- (const Vec2i& lhs, const Vec2i& rhs)
{
    Vec2i result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;
}

struct Timer
{
	float start_time;
	float duration;
};
