#pragma once

#include <assert.h>
#include <math.h>
#include <string>
#include "types.h"

#define PI 3.1415926f
#define RAD_2_DEG 57.29577951308232f
#define DEG_2_RAD 0.0174532925f

inline int32 round_float_to_int32(float num)
{
	int32 result = (int32)(num + 0.5f);
	return(result);
}

inline float rad_to_deg(float f)
{
	float result = (f * 180.f) / PI;
	return result;
}

inline float deg_to_rad(float f)
{
	float result = (f * PI) / 180.f;
}

class Vector2
{
public:
	float x;
	float y;

	Vector2(float = 0, float = 0);

	void operator=(const Vector2 &rhs);

	const Vector2 operator+(const Vector2 &rhs) const;

	void operator+=(const Vector2 &rhs);

	const Vector2 operator-(const Vector2 &rhs) const;

	void operator-=(const Vector2 &rhs);

	const Vector2 operator*(const float &rhs);

	void operator*=(const float &rhs);

	float dot(const Vector2& rhs) const;

	void normalize();

	static Vector2 normalized(const Vector2 &rhs);

	float get_length() const;

	std::string to_string();
};

class Vector3
{
public:
	float x, y, z;

	Vector3(float = 0, float = 0, float = 0);
	Vector3(Vector2, float x);
	
	Vector2 xy();

	void operator=(const Vector3 &rhs);

	const Vector3 operator+(const Vector3 &rhs) const;

	void operator+=(const Vector3 &rhs);

	const Vector3 operator-(const Vector3 &rhs) const;

	void operator-=(const Vector3 &rhs);

	const Vector3 operator*(const float &rhs);

	void operator*=(const float &rhs);

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
	float x, y, z, w;

	Vector4(float = 0, float = 0, float = 0, float = 0);
	Vector4(Vector3, float x);

	Vector3 xyz();

	void operator=(const Vector4 &rhs);

	const Vector4 operator+(const Vector4 &rhs) const;

	void operator+=(const Vector4 &rhs);

	const Vector4 operator-(const Vector4 &rhs) const;

	void operator-=(const Vector4 &rhs);

	const Vector4 operator*(const float &rhs);

	void operator*=(const float &rhs);

	float dot(const Vector4& rhs) const;

	void normalize();

	static Vector4 normalized(const Vector4 &rhs);

	float get_length() const;

	std::string to_string();
};

class Mat4
{
public:
	Vector4 e[4];
};

Mat4 operator * (Mat4& a, Mat4& b);

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
