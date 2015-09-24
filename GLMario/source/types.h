#pragma once

#include "SDL.h"
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


// TODO: create a utility header to have things like this;
#define ArrayCount(array) sizeof(array) / sizeof(array[0])


struct Dimension
{
	int32 width, height;
};

struct Point2
{
	int32 x, y;
};

struct Point3
{
	int32 x, y, z;
};

struct FRange
{
	float min;
	float max;

	FRange() : min(0), max(0) {}
	FRange(float min, float max) : min(min), max(max) {}
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
			int32 left, top, width, height;
		};
		int32 E[4];
	};
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
			float left, top, width, height;
		};
		float E[4];
	};	
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
