#pragma once

#include <assert.h>
#include <string>
#include "types.h"

#define PI 3.1415926f
#define RAD_2_DEG 57.29577951308232f
#define DEG_2_RAD 0.0174532925f

inline int32 round_float_to_int32(float num)
{
	int32 result = (int32)(num + 0.5f);
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



#if 0
inline uint32 max(uint32 a, uint32 b)
{
	uint32 result = (a < b ? b : a);
	return result;
}

inline uint32 min(uint32 a, uint32 b)
{
	uint32 result = (a < b ? a : b);
	return result;
}

inline int32 max(int32 a, int32 b)
{
	int32 result = (a < b ? b : a);
	return result;
}

inline int32 min(int32 a, int32 b)
{
	int32 result = (a < b ? a : b);
	return result;
}

inline float max(float a, float b)
{
	float result = (a < b ? b : a);
	return result;
}

inline float min(float a, float b)
{
	float result = (a < b ? a : b);
	return result;
}
#endif

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

union Vec2
{
	struct { float x, y; };
	struct { float r, g; };
	float e[2];
};

union Vec3
{
	struct { float x, y, z; };
	struct { float r, g, b; };
	struct { Vec2 xy; float _ignoredv3; };
	float e[3];
};

union Vec4
{
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
	struct { Vec3 xyz; float _ignoredv4; };
	float e[4];
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

inline Vec2
hadamard(Vec2 A, Vec2 B)
{
    Vec2 result = {A.x*B.x, A.y*B.y};

    return result;
}

inline float
dot(Vec2 A, Vec2 B)
{
    float result = A.x*B.x + A.y*B.y;

    return result;
}

inline float
length_sq(Vec2 A)
{
    float result = dot(A, A);

    return result;
}

inline float
length(Vec2 A)
{
    float result = sqrt(length_sq(A));
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
    Vec2 result = A * (1.0f / length(A));
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
dot(Vec3 A, Vec3 B)
{
    float result = A.x*B.x + A.y*B.y + A.z*B.z;

    return result;
}

inline float
LengthSq(Vec3 A)
{
    float result = dot(A, A);

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
dot(Vec4 A, Vec4 B)
{
    float result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return result;
}

inline float
LengthSq(Vec4 A)
{
    float result = dot(A, A);

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

// inline
// Vec2 vec2(Vec2 rhs)
// {
// 	Vec2 result = {};
// 	lhs.x = rhs.x;
// 	lhx.y = rhs.y;
// 	return result;
// }

// inline
// Vec2 operator+(Vec2 lhs, Vec2 rhs)
// {
// 	Vec2 result = { lhs.x + rhs.x, lhs.y + rhs.y };
// 	return result;
// }

// inline
// Vec2 & operator+=(Vec2& lhs, Vec2 rhs)
// {
// 	lhs = lhs + rhs;
// 	return lhs;
// }

// inline
// Vec2 operator-(Vec2 l, Vec2 r)
// {
// 	Vec2 result = { l.x - r.x, l.y - r.y };
// 	return result;
// }

// inline
// Vec2 & operator-=(vector2 &rhs);


// vector2 operator*(const float &rhs);

// void operator*=(const float &rhs);

// // vector2 operator*(const float& rhs);

// float dot(const vector2& rhs) const;

// void normalize();

// static vector2 normalized(const vector2 &rhs);

// float get_length() const;

// std::string to_string();


class Vector2
{
public:
	union { float x; float u; };
	union { float y; float v; };

	Vector2(float = 0, float = 0);

	void operator=(const Vector2 &rhs);

	Vector2 operator+(const Vector2 &rhs) const;

	void operator+=(const Vector2 &rhs);

	Vector2 operator-(const Vector2 &rhs) const;

	void operator-=(const Vector2 &rhs);

	Vector2 operator*(const float &rhs);

	void operator*=(const float &rhs);

	// vector2 operator*(const float& rhs);

	float dot(const Vector2& rhs) const;

	void normalize();

	static Vector2 normalized(const Vector2 &rhs);

	float get_length() const;

	std::string to_string();
};

class Vector3
{
public:
	union { float x; float r; };
	union { float y; float g; };
	union { float z; float b; };
	// float x, y, z;

	Vector3(float = 0, float = 0, float = 0);
	Vector3(Vector2, float z = 0);

	Vector2 xy();

	void operator=(const Vector3 &rhs);

	Vector3 operator+(const Vector3 &rhs) const;

	void operator+=(const Vector3 &rhs);

	Vector3 operator-(const Vector3 &rhs) const;

	void operator-=(const Vector3 &rhs);

	Vector3 operator*(const float &rhs);

	void operator*=(const float &rhs);

	// Vector3 operator*(const float& rhs);

	float dot(const Vector3& rhs) const;

	Vector3 cross_product(const Vector3& rhs) const;

	void normalize();

	static Vector3 normalized(const Vector3 &rhs);

	float get_length() const;

	std::string to_string();
};

class Vector4
{
public:
	union { float x; float r; };
	union { float y; float g; };
	union { float z; float b; };
	union { float w; float a; };
	// float x, y, z, w;

	Vector4(float = 0, float = 0, float = 0, float = 0);
	Vector4(Vector3, float x);

	Vector3 xyz();

	void operator=(const Vector4 &rhs);

	Vector4 operator+(const Vector4 &rhs) const;

	void operator+=(const Vector4 &rhs);

	Vector4 operator-(const Vector4 &rhs) const;

	void operator-=(const Vector4 &rhs);

	Vector4 operator*(const float &rhs);

	void operator*=(const float &rhs);

	// Vector4 operator*(const float& rhs);

	float dot(const Vector4& rhs) const;

	void normalize();

	static Vector4 normalized(const Vector4 &rhs);

	float get_length() const;

	std::string to_string();
};

inline
Vec2 lerp(Vec2 a, Vec2 b, float t)
{
	Vec2 result = (b - a) * t + a;
	return result;
}

inline
Vec3 lerp(Vec3 a, Vec3 b, float t)
{
	Vec3 result = (b - a) * t + a;
	return result;
}

inline
Vec4 lerp(Vec4 a, Vec4 b, float t)
{
	Vec4 result = (b - a) * t + a;
	return result;
}

Vector2 lerp(Vector2&, Vector2&, float);

Vector3 lerp(Vector3&, Vector3&, float);

Vector4 lerp(Vector4&, Vector4&, float);

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

inline bool Contains(Rectf rect, Vec2 point)
{
	if (point.x < rect.x + rect.w && point.x > rect.x
		&& point.y > rect.y && point.y < rect.y + rect.h)
	{
		return true;
	}
	return false;
}

inline Vec2 rect_center(Rectf r)
{
	Vec2 result = { r.x + r.w / 2.f, r.y + r.h / 2.f };
	return result;
}

inline Rectf MinkowskiSum(Rectf base, Rectf expand_amount)
{
	Rectf result = { base.x - expand_amount.w / 2.f,
					base.y - expand_amount.h / 2.f,
					expand_amount.w + base.w,
					expand_amount.h + base.h };

    return result;
}

inline bool Intersects(Rectf lhs, Rectf rhs)
{
	Vec2 center = rect_center(lhs);
    Rectf mink_sum = MinkowskiSum(rhs, lhs);
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

class Mat4
{
public:
	Vector4 e[4];
};

Mat4 operator * (const Mat4& a, const Mat4& b);

Mat4 scale_matrix(Vector3 scale);
Mat4 translation_matrix(Vector3 position);
Mat4 rotation_matrix(Vector3 euler);
Mat4 z_rotation_matrix(float angle);
Mat4 orthographic_matrix(float w, float h, float n, float f, Vector2 center);
Mat4 perspective_matrix(float fov_x, float fov_y, float z_near, float z_far);
Mat4 view_matrix(Vector3 right, Vector3 up, Vector3 look, Vector3 position);
Mat4 identity_matrix();
Mat4 mvp_matrix(Vector3 scale, Vector3 position, Vector3 euler, Mat4& perspective, Mat4& view);
Mat4 ortho_mvp_matrix(Vector3 scale, Vector3 position, float angle, Mat4& perspective, Mat4& view);
