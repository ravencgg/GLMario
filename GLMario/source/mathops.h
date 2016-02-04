#pragma once

#include <assert.h>
#include <string>
#include "types.h"

#define PI 3.1415926f
#define TAU 6.2831853f
#define RAD_2_DEG 57.29577951308232f
#define DEG_2_RAD 0.0174532925f

inline int32 round_float_to_int32(float num)
{
    int32 result = num > 0 ? (int32)(num + 0.5f) : (int32)(num - 0.5f);
	return result;
}

inline float rad_to_deg(float f)
{
	float result = (f * 180.f) / PI;
	return result;
}

inline float deg_to_rad(float f)
{
	float result = (f * PI) / 180.f;
	return result;
}

#ifdef max
#undef max
#endif
#define max(a, b) ((a) > (b) ? (a) : (b))

#ifdef min
#undef min
#endif
#define min(a, b) ((a) < (b) ? (a) : (b))

inline float clamp01(float f)
{
	float result = min(max(f, 0.f), 1.f);
    return result;
}

inline float sign(float x)
{
   int result = (x > 0) - (x < 0);

   return (float)result;
}


struct Vec2_4
{
    Vec2 e[4];
};

struct Vec2_8
{
    Vec2 e[8];
};

inline Vec2 vec2(float x, float y)
{
	Vec2 result = { x, y };
	return result;
}

inline Vec3 vec3(float x, float y, float z)
{
	Vec3 result = { x, y, z };
	return result;
}

inline Vec3 vec3(Vec2 xy, float z)
{
	Vec3 result = { xy.x, xy.y, z };
	return result;
}

inline Vec4 vec4(float x, float y, float z, float w)
{
	Vec4 result = { x, y, z, w };
	return result;
}


inline Vec2
Perp(Vec2 A)
{
    Vec2 result = {-A.y, A.x};
    return result;
}

inline Vec2
operator*(float A, Vec2 B)
{
    Vec2 result;

    result.x = A*B.x;
    result.y = A*B.y;

    return result;
}

inline Vec2
operator*(Vec2 B, float A)
{
    Vec2 result = A*B;

    return result;
}

inline Vec2 &
operator*=(Vec2 &B, float A)
{
    B = A * B;

    return B;
}

inline Vec2
operator-(Vec2 A)
{
    Vec2 result;

    result.x = -A.x;
    result.y = -A.y;

    return result;
}

inline Vec2
operator+(Vec2 A, Vec2 B)
{
    Vec2 result;

    result.x = A.x + B.x;
    result.y = A.y + B.y;

    return result;
}

inline Vec2 &
operator+=(Vec2 &A, Vec2 B)
{
    A = A + B;

    return(A);
}

inline Vec2
operator-(Vec2 A, Vec2 B)
{
    Vec2 result;

    result.x = A.x - B.x;
    result.y = A.y - B.y;

    return result;
}

inline Vec2 &
operator-=(Vec2 &A, Vec2 B)
{
    A = A - B;

    return(A);
}

inline Vec2
hadamard(Vec2 A, Vec2 B)
{
    Vec2 result = {A.x*B.x, A.y*B.y};

    return result;
}

inline float
Dot(Vec2 A, Vec2 B)
{
    float result = A.x*B.x + A.y*B.y;

    return result;
}

inline float
LengthSq(Vec2 A)
{
    float result = Dot(A, A);

    return result;
}

inline float
Length(Vec2 A)
{
    float result = sqrt(LengthSq(A));
    return result;
}

inline Vec2
clamp01(Vec2 Value)
{
    Vec2 result;

    result.x = clamp01(Value.x);
    result.y = clamp01(Value.y);

    return result;
}

inline Vec2
Normalize(Vec2 A)
{
    Vec2 result = A * (1.0f / Length(A));
    return result;
}

inline
std::string to_string(Vec2 v)
{
	std::string x_str = std::to_string(v.x);
	std::string y_str = std::to_string(v.y);
	std::string result = "(" + x_str + ", " + y_str + ")";
	return result;
}

//
// NOTE(casey): Vec3 operations
//

inline Vec3
operator*(float A, Vec3 B)
{
    Vec3 result;

    result.x = A*B.x;
    result.y = A*B.y;
    result.z = A*B.z;

    return result;
}

inline Vec3
operator*(Vec3 B, float A)
{
    Vec3 result = A*B;

    return result;
}

inline Vec3 &
operator*=(Vec3 &B, float A)
{
    B = A * B;

    return(B);
}

inline Vec3
operator-(Vec3 A)
{
    Vec3 result;

    result.x = -A.x;
    result.y = -A.y;
    result.z = -A.z;

    return result;
}

inline Vec3
operator+(Vec3 A, Vec3 B)
{
    Vec3 result;

    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;

    return result;
}

inline Vec3 &
operator+=(Vec3 &A, Vec3 B)
{
    A = A + B;

    return(A);
}

inline Vec3
operator-(Vec3 A, Vec3 B)
{
    Vec3 result;

    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;

    return result;
}

inline Vec3
Hadamard(Vec3 A, Vec3 B)
{
    Vec3 result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return result;
}

inline float
Dot(Vec3 A, Vec3 B)
{
    float result = A.x*B.x + A.y*B.y + A.z*B.z;

    return result;
}

inline float
LengthSq(Vec3 A)
{
    float result = Dot(A, A);

    return result;
}

inline float
Length(Vec3 A)
{
    float result = sqrt(LengthSq(A));
    return result;
}

inline Vec3
Normalize(Vec3 A)
{
    Vec3 result = A * (1.0f / Length(A));

    return result;
}

inline Vec3
Clamp01(Vec3 Value)
{
    Vec3 result;

    result.x = clamp01(Value.x);
    result.y = clamp01(Value.y);
    result.z = clamp01(Value.z);

    return result;
}

inline Vec3
Lerp(Vec3 A, float t, Vec3 B)
{
    Vec3 result = (1.0f - t)*A + t*B;

    return result;
}

//
// NOTE(casey): Vec4 operations
//

inline Vec4
operator*(float A, Vec4 B)
{
    Vec4 result;

    result.x = A*B.x;
    result.y = A*B.y;
    result.z = A*B.z;
    result.w = A*B.w;

    return result;
}

inline Vec4
operator*(Vec4 B, float A)
{
    Vec4 result = A*B;

    return result;
}

inline Vec4 &
operator*=(Vec4 &B, float A)
{
    B = A * B;

    return(B);
}

inline Vec4
operator-(Vec4 A)
{
    Vec4 result;

    result.x = -A.x;
    result.y = -A.y;
    result.z = -A.z;
    result.w = -A.w;

    return result;
}

inline Vec4
operator+(Vec4 A, Vec4 B)
{
    Vec4 result;

    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    result.w = A.w + B.w;

    return result;
}

inline Vec4 &
operator+=(Vec4 &A, Vec4 B)
{
    A = A + B;

    return(A);
}

inline Vec4
operator-(Vec4 A, Vec4 B)
{
    Vec4 result;

    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    result.w = A.w - B.w;

    return result;
}

inline Vec4
Hadamard(Vec4 A, Vec4 B)
{
    Vec4 result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};

    return result;
}

inline float
Dot(Vec4 A, Vec4 B)
{
    float result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return result;
}

inline float
LengthSq(Vec4 A)
{
    float result = Dot(A, A);

    return result;
}

inline float
Length(Vec4 A)
{
    float result = sqrt(LengthSq(A));
    return result;
}

inline Vec4
Clamp01(Vec4 Value)
{
    Vec4 result;

    result.x = clamp01(Value.x);
    result.y = clamp01(Value.y);
    result.z = clamp01(Value.z);
    result.w = clamp01(Value.w);

    return result;
}

inline Vec4
Lerp(Vec4 A, float t, Vec4 B)
{
    Vec4 result = (1.0f - t)*A + t*B;

    return result;
}

inline Vec2 Lerp(Vec2 a, Vec2 b, float t)
{
	Vec2 result = (b - a) * t + a;
	return result;
}

inline Vec2 lerp(Vec2 a, Vec2 b, float t)
{
	Vec2 result = (b - a) * t + a;
	return result;
}

inline Vec3 lerp(Vec3 a, Vec3 b, float t)
{
	Vec3 result = (b - a) * t + a;
	return result;
}

inline Vec4 lerp(Vec4 a, Vec4 b, float t)
{
	Vec4 result = (b - a) * t + a;
	return result;
}

inline float MaxSigned(float in, float maxValue)
{
	float result = sign(in) * max(abs(in), abs(maxValue));
	return result;
}

inline float MinSigned(float in, float minValue)
{
	float result = sign(in) * min(abs(in), abs(minValue));
	return result;
}

/*************************************************
*
*  Rectf Creation Utilities
*
*************************************************/

inline Rectf
RectFromDimCenter(Vec2 pos, Vec2 dim)
{
    Rectf result;
    result.x = pos.x - (dim.x * 0.5f);
    result.y = pos.y - (dim.y * 0.5f);
    result.w = dim.x;
    result.h = dim.y;
    return result;
}

inline Rectf RectFromDimCorner(Vec2 pos, Vec2 dim)
{
    Rectf result;
    result.x = pos.x;
    result.y = pos.y;
    result.w = dim.x;
    result.h = dim.y;
    return result;
}

/*************************************************
*
*  Minkowski Mathematics
*
*************************************************/

Rectf MinkowskiSum(Rectf base, Rectf expand_amount);

Vec2_8 MinkowskiSum(Rectf a_in, float theta_a, Rectf b_in, float theta_b, Rectf* aabb);

/*************************************************
*
*  Other utility
*
*************************************************/

Vec2 RotatePoint(Vec2 in_point, float theta, Vec2 rotate_point);

Vec2_4 RotatedRect(Rectf rect, float theta, Rectf* out_aabb = nullptr);

inline bool Contains(Rectf rect, Vec2 point)
{
	if (point.x < rect.x + rect.w && point.x > rect.x
		&& point.y > rect.bot && point.y < rect.bot + rect.h)
	{
		return true;
	}
	return false;
}

inline bool Contains(Rect rect, Vec2i point)
{
	if (point.x < rect.x + rect.w && point.x > rect.x
		&& point.y > rect.bot && point.y < rect.y + rect.h)
	{
		return true;
	}
	return false;
}


inline Vec2 RectCenter(Rectf r)
{
	Vec2 result = { r.left + r.w / 2.f, r.bot + r.h / 2.f };
	return result;
}

// TODO: @test -> untested code
inline bool Contains(Rectf rect, float theta, Vec2 point)
{
    // Rotate the point backwards, and check against the unrotated rect
    Vec2 rot_point = RotatePoint(point, -theta, RectCenter(rect));
    return Contains(rect, rot_point);
}

inline bool Intersects(Rectf lhs, Rectf rhs)
{
	Vec2 center = RectCenter(rhs);
    Rectf mink_sum = MinkowskiSum(lhs, rhs);
    bool result = Contains(mink_sum, center);
    return result;
}

inline __m128 lerp(__m128& a, __m128& b, __m128& t)
{
	__m128 result = _mm_sub_ps(b, a);
	result = _mm_mul_ps(result, t);
	result = _mm_add_ps(result, a);
	return result;
}

/*************************************************
*
*  Colors
*
*************************************************/

Vec4 MakeColor(Vec3 color, float alpha);

extern Vec3 cg_white;
extern Vec3 cg_black;

extern Vec3 cg_gray1;
extern Vec3 cg_gray2;
extern Vec3 cg_gray3;
extern Vec3 cg_gray4;

extern Vec3 cg_yellow;
extern Vec3 cg_blue;
extern Vec3 cg_cyan;
extern Vec3 cg_green;
extern Vec3 cg_orange;
extern Vec3 cg_purple;

extern Vec3 cg_dark_purple;
extern Vec3 cg_dark_green;

extern Vec3 cg_light_green;
extern Vec3 cg_light_blue;

