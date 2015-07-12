#include "mathops.h"


/*************************************************
*
*  Vector2 Implementation
*
*************************************************/

Vector2::Vector2(float x, float y)
	:x(x),
	y(y){}

void Vector2::operator=(const Vector2 &rhs)
{
	this->x = rhs.x;
	this->y = rhs.y;
}

Vector2 Vector2::operator+(const Vector2 &rhs) const
{
	Vector2 result;
	result.x = this->x + rhs.x;
	result.y = this->y + rhs.y;
	return(result);
}

void Vector2::operator+=(const Vector2 &rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
}

Vector2 Vector2::operator-(const Vector2 &rhs) const
{
	Vector2 result;
	result.x = this->x - rhs.x;
	result.y = this->y - rhs.y;
	return result;
}

void Vector2::operator-=(const Vector2 &rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
}

// const Vector2 Vector2::operator*(const float &rhs)
// {
// 	Vector2 result;
// 	result.x = this->x * rhs;
// 	result.y = this->y * rhs;
// 	return result;
// }

void Vector2::operator*=(const float &rhs)
{
	this->x *= rhs;
	this->y *= rhs;
}

Vector2 Vector2::operator*(const float& rhs)
{
	Vector2 result(this->x * rhs, this->y * rhs);
	return result;
}

float Vector2::dot(const Vector2& rhs) const
{
	float result = this->x * rhs.x + this->y * rhs.y;
	return result;
}

void Vector2::normalize()
{
	float length = this->get_length();
	this->x /= length;
	this->y /= length;
}

// removed static
Vector2 Vector2::normalized(const Vector2 &rhs)
{
	Vector2 result;
	float length = rhs.get_length();
	result.x = rhs.x / length;
	result.y = rhs.y / length;
	return result;
}

float Vector2::get_length() const
{
	float result = sqrt(this->x * this->x + this->y * this->y);
	return result;
}

std::string Vector2::to_string()
{
	std::string x_str = std::to_string(this->x);
	std::string y_str = std::to_string(this->y);
	std::string result = "(" + x_str + ", " + y_str + ")";
	return result;
}


/*************************************************
 *
 *  Vector3 Implementation
 *
 *************************************************/


Vector3::Vector3(float x, float y, float z)
	: x(x),
	  y(y),
	  z(z){}

Vector3::Vector3(Vector2 xy, float z)
	: x(xy.x),
	  y(xy.y),
	  z(z){}

Vector2 Vector3::xy()
{
	return Vector2(this->x, this->y);
}

void Vector3::operator=(const Vector3 &rhs)
{
	this->x = rhs.x;
	this->y = rhs.y;
	this->z = rhs.z;
}

Vector3 Vector3::operator+(const Vector3 &rhs) const
{
	Vector3 result;
	result.x = this->x + rhs.x;
	result.y = this->y + rhs.y;
	result.z = this->z + rhs.z;
	return(result);
}

void Vector3::operator+=(const Vector3 &rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
}

Vector3 Vector3::operator-(const Vector3 &rhs) const
{
	Vector3 result;
	result.x = this->x - rhs.x;
	result.y = this->y - rhs.y;
	result.z = this->z - rhs.z;
	return result;
}

void Vector3::operator-=(const Vector3 &rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
}

// const Vector3 Vector3::operator*(const float &rhs)
// {
// 	Vector3 result;
// 	result.x = this->x * rhs;
// 	result.y = this->y * rhs;
// 	result.z = this->z * rhs;
// 	return result;
// }

void Vector3::operator*=(const float &rhs)
{
	this->x *= rhs;
	this->y *= rhs;
	this->z *= rhs;
}

Vector3 Vector3::operator*(const float& rhs)
{
	Vector3 result(this->x * rhs, this->y * rhs, this->z * rhs);
	return result;
}

float Vector3::dot(const Vector3& rhs) const
{
	float result = this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
	return result;
}

void Vector3::normalize()
{
	float length = this->get_length();
	this->x /= length;
	this->y /= length;
	this->z /= length;
}

//removed static
Vector3 Vector3::normalized(const Vector3 &rhs)
{
	Vector3 result;
	float length = rhs.get_length();
	result.x = rhs.x / length;
	result.y = rhs.y / length;
	result.z = rhs.z / length;
	return result;
}

float Vector3::get_length() const
{
	float result = sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	return result;
}

std::string Vector3::to_string()
{
	std::string x_str = std::to_string(this->x);
	std::string y_str = std::to_string(this->y);
	std::string z_str = std::to_string(this->z);
	std::string result = "(" + x_str + ", " + y_str + ", " + z_str + ")";
	return result;
}


/*************************************************
*
*  Vector4 Implementation
*
*************************************************/


Vector4::Vector4(float x, float y, float z, float w)
	: x(x),
	y(y),
	z(z),
	w(w){}

Vector4::Vector4(Vector3 xyz, float w = 0)
	: x(xyz.x),
	y(xyz.y),
	z(xyz.z),
	w(w){}

Vector3 Vector4::xyz()
{
	return Vector3(this->x, this->y, this->z);
}

void Vector4::operator=(const Vector4 &rhs)
{
	this->x = rhs.x;
	this->y = rhs.y;
	this->z = rhs.z;
	this->w = rhs.w;
}

Vector4 Vector4::operator+(const Vector4 &rhs) const
{
	Vector4 result;
	result.x = this->x + rhs.x;
	result.y = this->y + rhs.y;
	result.z = this->z + rhs.z;
	result.w = this->w + rhs.w;
	return(result);
}

void Vector4::operator+=(const Vector4 &rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	this->w += rhs.w;
}

Vector4 Vector4::operator-(const Vector4 &rhs) const
{
	Vector4 result;
	result.x = this->x - rhs.x;
	result.y = this->y - rhs.y;
	result.z = this->z - rhs.z;
	result.w = this->w - rhs.w;
	return result;
}

void Vector4::operator-=(const Vector4 &rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	this->w -= rhs.w;
}

// const Vector4 Vector4::operator*(const float &rhs)
// {
// 	Vector4 result;
// 	result.x = this->x * rhs;
// 	result.y = this->y * rhs;
// 	result.z = this->z * rhs;
// 	result.w = this->w * rhs;
// 	return result;
// }

void Vector4::operator*=(const float &rhs)
{
	this->x *= rhs;
	this->y *= rhs;
	this->z *= rhs;
	this->w *= rhs;
}

Vector4 Vector4::operator*(const float& rhs)
{
	Vector4 result(this->x * rhs, this->y * rhs, this->z * rhs, this->w * rhs);
	return result;
}

float Vector4::dot(const Vector4& rhs) const
{
	float result = this->x * rhs.x + this->y * rhs.y + this->z * rhs.z + this->w * rhs.w;
	return result;
}

Vector3 Vector3::cross_product(const Vector3 &rhs) const
{
	Vector3 result;

	result.x = this->y * rhs.z - this->z * rhs.y;
	result.y = this->z * rhs.x - this->x * rhs.z;
	result.z = this->x * rhs.y - this->y * rhs.x;

	result.x = -result.x;
	result.y = -result.y;
	result.z = -result.z;

	return result;
}

void Vector4::normalize()
{
	float length = this->get_length();
	this->x /= length;
	this->y /= length;
	this->z /= length;
	this->w /= length;
}

//removed static
Vector4 Vector4::normalized(const Vector4 &rhs)
{
	Vector4 result;
	float length = rhs.get_length();
	result.x = rhs.x / length;
	result.y = rhs.y / length;
	result.z = rhs.z / length;
	result.w = rhs.w / length;
	return result;
}

float Vector4::get_length() const
{
	float result = sqrt(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);
	return result;
}

std::string Vector4::to_string()
{
	std::string x_str = std::to_string(this->x);
	std::string y_str = std::to_string(this->y);
	std::string z_str = std::to_string(this->z);
	std::string w_str = std::to_string(this->w);
	std::string result = "(" + x_str + ", " + y_str + ", " + z_str + ", " + z_str + ")";
	return result;
}


Vector2 lerp(Vector2& a, Vector2& b, float t)
{
	Vector2 result = (b - a) * t + a;  
	return result;
}

Vector3 lerp(Vector3& a, Vector3& b, float t)
{
	Vector3 result = (b - a) * t + a;  
	return result;
}

Vector4 lerp(Vector4& a, Vector4& b, float t)
{
	Vector4 result = (b - a) * t + a;  
	return result;
}


/*************************************************
*
*  Mat4 Implementation
*
*************************************************/


Mat4 operator * (const Mat4& a, const Mat4& b) {
	
	// Extract columns from the second matrix;
	Vector4 c[4];
	c[0] = Vector4(b.e[0].x, b.e[1].x, b.e[2].x, b.e[3].x);
	c[1] = Vector4(b.e[0].y, b.e[1].y, b.e[2].y, b.e[3].y);
	c[2] = Vector4(b.e[0].z, b.e[1].z, b.e[2].z, b.e[3].z);
	c[3] = Vector4(b.e[0].w, b.e[1].w, b.e[2].w, b.e[3].w);

	Mat4 result;
	result.e[0] = Vector4(a.e[0].dot(c[0]), a.e[0].dot(c[1]), a.e[0].dot(c[2]), a.e[0].dot(c[3]));
	result.e[1] = Vector4(a.e[1].dot(c[0]), a.e[1].dot(c[1]), a.e[1].dot(c[2]), a.e[1].dot(c[3]));
	result.e[2] = Vector4(a.e[2].dot(c[0]), a.e[2].dot(c[1]), a.e[2].dot(c[2]), a.e[2].dot(c[3]));
	result.e[3] = Vector4(a.e[3].dot(c[0]), a.e[3].dot(c[1]), a.e[3].dot(c[2]), a.e[3].dot(c[3]));
	
	return result;
}

Mat4 translation_matrix(Vector3 t)
{
	Mat4 result;

	result.e[0] = Vector4(1.f, 0, 0, t.x);
	result.e[1] = Vector4(0, 1.f, 0, t.y);
	result.e[2] = Vector4(0, 0, 1.f, t.z);
	result.e[3] = Vector4(0, 0, 0, 1.f);

	return result;
}

Mat4 scale_matrix(Vector3 scale)
{
	Mat4 result;

	result.e[0] = Vector4(scale.x, 0, 0, 0);
	result.e[1] = Vector4(0, scale.y, 0, 0);
	result.e[2] = Vector4(0, 0, scale.z, 0);
	result.e[3] = Vector4(0, 0, 0, 1.f);

	return result;
}

Mat4 rotation_matrix(Vector3 euler)
{

	Mat4 x_rot;
	x_rot.e[0] = Vector4(1.f, 0, 0, 0);
	x_rot.e[1] = Vector4(0, cos(euler.x), -sin(euler.x), 0);
	x_rot.e[2] = Vector4(0, sin(euler.x), cos(euler.x), 0);
	x_rot.e[3] = Vector4(0, 0, 0, 1.f);

	Mat4 y_rot;
	y_rot.e[0] = Vector4(cos(euler.y), 0, sin(euler.y), 0);
	y_rot.e[1] = Vector4(0, 1.f, 0, 0);
	y_rot.e[2] = Vector4(-sin(euler.y), 0, cos(euler.y), 0);
	y_rot.e[3] = Vector4(0, 0, 0, 1.f);

	Mat4 z_rot;
	z_rot.e[0] = Vector4(cos(euler.z), -sin(euler.z), 0, 0);
	z_rot.e[1] = Vector4(sin(euler.z), cos(euler.z), 0, 0);
	z_rot.e[2] = Vector4(0, 0, 1.f, 0);
	z_rot.e[3] = Vector4(0, 0, 0, 1.f);

	Mat4 result = x_rot * y_rot * z_rot;

	return result;
}

Mat4 z_rotation_matrix(float angle)
{
	Mat4 result;

	result.e[0] = Vector4(cos(angle), -sin(angle), 0, 0);
	result.e[1] = Vector4(sin(angle), cos(angle), 0, 0);
	result.e[2] = Vector4(0, 0, 1.f, 0);
	result.e[3] = Vector4(0, 0, 0, 1.f);

	return result;
}

Mat4 orthographic_matrix(float w, float h, float n, float f, Vector2 center)
{
	float r = center.x + (w / 2);
	float l = center.x - (w / 2);

	float t = center.y + (h / 2);
	float b = center.y - (h / 2);

	float nr = n;
	float far = f;

	Mat4 result;

	result.e[0] = Vector4((2.f * nr) / (r - l), 0, (r + l) / (r - l), 0);
	result.e[1] = Vector4(0, (2.f * nr) / (t - b), (t + b) / (t - b), 0);
	result.e[2] = Vector4(0, 0, (-far - nr) / (far - nr), (-2.f * far * nr) / (far - nr));
	result.e[3] = Vector4(0, 0, -1.f, 0);

	return result;
}

// TODO(chris): add in an aspect ratio version of this.
Mat4 perspective_matrix(float fov_x, float fov_y, float z_near, float z_far)
{
	Mat4 result = {};
	// float aspect = 16.f/9.f;
	fov_x *= DEG_2_RAD;
	fov_y *= DEG_2_RAD;

	result.e[0] = Vector4(atanf(fov_x / 2.f), 0, 0, 0);
	result.e[1] = Vector4(0, atanf(fov_y / 2.f), 0, 0);
	result.e[2] = Vector4(0, 0, - (z_far + z_near) / (z_far - z_near), - (2 * z_near * z_far) / (z_far - z_near));
	result.e[3] = Vector4(0, 0, -1.f, 0);

	// float tanHalfFovy = tan(fov_y / 2);

	// //// tmat4x4<T, defaultp> Result(static_cast<T>(0));
	// result.e[0].x = 1 / (aspect * tanHalfFovy);
	// result.e[1].y = 1 / (tanHalfFovy);
	// result.e[2].z = - (z_far + z_near) / (z_far - z_near);
	// result.e[2].w = - 1;
	// result.e[3].y = - ((2) * z_far * z_near) / (z_far - z_near);
	// //return Result;

	//TODO(chris): e[0] is not atanf, it is ar*tan;
	//result.e[0] = Vector4(1.f / atanf(fov_x / 2.f), 0, 0, 0);
	//result.e[1] = Vector4(0, 1 / tanf(fov_y / 2.f), 0, 0);
	//result.e[2] = Vector4(0, 0, (-z_far - z_near) / (z_near - z_far), (2 * z_near * z_far) / (z_near - z_far));
	//result.e[3] = Vector4(0, 0, 1.f, 0);

	return result;
}

Mat4 view_matrix(Vector3 right, Vector3 up, Vector3 look, Vector3 position)
{
	Mat4 result;

	result.e[0] = Vector4(right.x, right.y, right.z, 0);
	result.e[1] = Vector4(up.x, up.y, up.z, 0);
	result.e[2] = Vector4(look.x, look.y, look.z, 0);
	result.e[3] = Vector4(position.x, position.y, position.z, 1.f);

	return result;
}

Mat4 identity_matrix()
{
	Mat4 result;

	result.e[0] = Vector4(1, 0, 0, 0);
	result.e[1] = Vector4(0, 1, 0, 0);
	result.e[2] = Vector4(0, 0, 1, 0);
	result.e[3] = Vector4(0, 0, 0, 1);

	return result;
}

Mat4 mvp_matrix(Vector3 scale, Vector3 position, Vector3 euler, Mat4& perspective, Mat4& view)
{
	Mat4 result = perspective * view * translation_matrix(position) * scale_matrix(scale) * rotation_matrix(euler);
	return result;
}

Mat4 ortho_mvp_matrix(Vector3 scale, Vector3 position, float angle, Mat4& perspective, Mat4& view)
{
	Mat4 result = perspective * view * translation_matrix(position) * scale_matrix(scale) * z_rotation_matrix(angle);
	return result;
}
