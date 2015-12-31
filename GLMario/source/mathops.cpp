#include "mathops.h"
#include <float.h>

/*************************************************
*
*  2D Rotation
*
*************************************************/

// TODO: fixme
Vec2 RotatePoint(Rectf rect, float theta, Vec2 point)
{
    Vec2 result = { };
    if(theta == 0)
    {
        return point;
    }

    float left = rect.x;
    float right = rect.x + rect.w;
    float top = rect.y;
    float bot = rect.y + rect.h;

    Vec2 rotation_point = RectCenter(rect);

    float sin_t = sin(theta);
    float cos_t = cos(theta);

    float x_in = point.x;
    float y_in = point.y;

    float x = rotation_point.x;
    float y = rotation_point.y;

    float r00 = cos_t;
    float r01 = -sin_t;
    float r10 = sin_t;
    float r11 = cos_t;

// All of the multiplications in the equation
    float x_1 = r00 * x_in;
    float x_2 = r01 * y_in;
    float x_3 = r00 * x;
    float x_4 = r01 * y;

// All of the sums/differences in the equation
    float x_5 = x_1 + x_2;
    float x_6 = x_5 + x;
    float x_7 = x_6 - x_3;

    result.x = x_7 - x_4;

// All of the multiplications in the equation
    float y_1 = r10 * x_in;
    float y_2 = r11 * y_in;
    float y_3 = r10 * x;
    float y_4 = r11 * y;

// All of the sums/differences in the equation
    float y_5 = y_1 + y_2;
    float y_6 = y_5 + y;
    float y_7 = y_6 - y_3;
    result.y = y_7 - y_4;

    return result;
}

/*********************
 * Returns a Vec2_4 with the following logic
 *
 * Input            OutputIndex
 * ------------------------------
 * Top left         0
 * Bottom left      1
 * Bottom right     2
 * Top right        3
 *
 * So whichever corner was the Top left when coming into this function gets
 * rotated and output to the 0th element of the Vec2_4 etc.
 */
Vec2_4 RotatedRect(Rectf rect, float theta, Rectf* out_aabb)
{
    Vec2_4 result = { };

    float left = rect.x;
    float right = rect.x + rect.w;
    float bot = rect.bot;
    float top = rect.bot + rect.h;

    if(theta == 0)
    {
        result.e[0] = { left, top };
        result.e[1] = { left, bot };
        result.e[2] = { right, bot };
        result.e[3] = { right, top };

        return result;
    }

    Vec2 rotation_point = RectCenter(rect);

    float sin_t = sin(theta);
    float cos_t = cos(theta);

    __m128 x_in = _mm_setr_ps(left, left, right, right);
    __m128 y_in = _mm_setr_ps(top, bot, bot, top);

    __m128 x = _mm_set1_ps(rotation_point.x);
    __m128 y = _mm_set1_ps(rotation_point.y);

    __m128 r00 = _mm_set1_ps(cos_t);
    __m128 r01 = _mm_set1_ps(-sin_t);
    __m128 r10 = _mm_set1_ps(sin_t);
    __m128 r11 = _mm_set1_ps(cos_t);

// All of the multiplications in the equation
    __m128 x_1 = _mm_mul_ps(r00, x_in);
    __m128 x_2 = _mm_mul_ps(r01, y_in);
    __m128 x_3 = _mm_mul_ps(r00, x);
    __m128 x_4 = _mm_mul_ps(r01, y);

// All of the sums/differences in the equation
    __m128 x_5 = _mm_add_ps(x_1, x_2);
    __m128 x_6 = _mm_add_ps(x_5, x);
    __m128 x_7 = _mm_sub_ps(x_6, x_3);
    __m128 x_out = _mm_sub_ps(x_7, x_4);

// All of the multiplications in the equation
    __m128 y_1 = _mm_mul_ps(r10, x_in);
    __m128 y_2 = _mm_mul_ps(r11, y_in);
    __m128 y_3 = _mm_mul_ps(r10, x);
    __m128 y_4 = _mm_mul_ps(r11, y);

// All of the sums/differences in the equation
    __m128 y_5 = _mm_add_ps(y_1, y_2);
    __m128 y_6 = _mm_add_ps(y_5, y);
    __m128 y_7 = _mm_sub_ps(y_6, y_3);
    __m128 y_out = _mm_sub_ps(y_7, y_4);

    if(out_aabb)
    {
        float l, r, t, b;
        float xx = ((float *)&(x_out))[0];
        float yy = ((float *)&(y_out))[0];

        l = xx;
        r = xx;
        t = yy;
        b = yy;

        for(uint32 i = 0; i < 4; ++i)
        {
            xx = ((float *)&(x_out))[i];
            yy = ((float *)&(y_out))[i];

            if(xx < l)
            {
                l = xx;
            }
            if(xx > r)
            {
                r = xx;
            }

            if(yy < b)
            {
                b = yy;
            }
            if(yy > t)
            {
                t = yy;
            }

            result.e[i].x = xx;
            result.e[i].y = yy;
        }

        out_aabb->x = l;
        out_aabb->w = r-l;
        out_aabb->y = t;
        out_aabb->h = t-b;
    }
    else
    {
        for(uint32 i = 0; i < 4; ++i)
        {
            result.e[i].x = ((float *)&(x_out))[i];
            result.e[i].y = ((float *)&(y_out))[i];
        }
    }


    return result;
}

/*************************************************
*
*  Minkowski Mathematics
*
*************************************************/

// Aligned MinkowskiSum
Rectf MinkowskiSum(Rectf base, Rectf expand_amount)
{
	Rectf result = { base.x - expand_amount.w / 2.f,
					base.y - expand_amount.h / 2.f,
					expand_amount.w + base.w,
					expand_amount.h + base.h };

    return result;
}

Vec2_8 MinkowskiSum(Rectf a_in, float theta_a, Rectf b_in, float theta_b, Rectf* aabb)
{
    Vec2_8 result = { };
    const uint32 rect_sides = 4;
    Vec2_4 a = RotatedRect(a_in, theta_a);
    Vec2_4 b = RotatedRect(b_in, theta_b);

    Vec2 center = RectCenter(b_in);
    for(uint32 i = 0; i < 4; ++i)
    {
        b.e[i] -= center;
    }

    // Find the quadrant of the angle
    int32 int_angle = (int32) (theta_a * (360.f / TAU));
    int_angle = int_angle - (int_angle / 360) * 360;
    if(int_angle < 0)
    {
        int_angle += 360;
    }
    const int32 quadrant_map[] = { 0, 3, 2, 1, 0, 3, 2, 1, 0 };
    int32 left_quadrant = int_angle / 90;

#if 0
    //int32 int_angle_b = (int32) ((theta_b - (TAU / 8.f)) * (360.f / TAU));
    int32 int_angle_b = (int32) (theta_b * (360.f / TAU));
    const int32 quadrant_map_b[] = { 0, 3, 3, 2, 2, 1, 1, 0, 0, 3, 3, 2, 2, 1, 1, 0 };
    int_angle_b = int_angle_b - (int_angle_b / 360) * 360;
//    int_angle_b -= 45;
    if(int_angle_b < 0)
    {
        int_angle_b += 360;
    }
    int32 start_index = int_angle_b / 90; // Is the left most index
#endif

    const int32 quadrant_map_b[] = { 0, 3, 3, 2, 2, 1, 1, 0, 0, 3, 3, 2, 2, 1, 1, 0 };

    int left_index = 0;
    float left_amount = b.e[0].x;
    for(int i = 1; i < 4; ++i)
    {
        if(b.e[i].x < left_amount)
        {
            left_amount = b.e[i].x;
            left_index = i;
        }
    }

    Vec2 point_a1 = a.e[quadrant_map[left_quadrant]];
    Vec2 point_a2 = a.e[quadrant_map[left_quadrant + 1]];

    Vec2 left = b.e[left_index] + point_a1;
    int32 next_index = left_index - 1;
    if(next_index < 0) next_index = 3;

    Vec2 next_this_point = b.e[next_index] + point_a1 - left;
    Vec2 next_point = b.e[next_index] + point_a2 - left;

    float theta_this = atan2(next_this_point.y, next_this_point.x);
    float theta_next = atan2(next_point.y, next_point.x);

    int32 aabb_start_index = 0;
    if(theta_this < theta_next)
    {
        left_index++;
        aabb_start_index = 1;
    }

    int32 start_index = 4 - left_index;
    start_index *= 2;

    int32 in = 0;
    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    ++left_quadrant;

    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    ++left_quadrant;

    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    ++left_quadrant;

    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];
    result.e[in++] = a.e[quadrant_map[left_quadrant]] + b.e[quadrant_map_b[start_index++]];

    if(aabb)
    {
        aabb->left = result.e[aabb_start_index].x;
        aabb->bot  = result.e[aabb_start_index + 6].y;
        aabb->h    = result.e[aabb_start_index + 2].y - aabb->bot;
        aabb->w    = result.e[aabb_start_index + 4].x - aabb->left;
    }

    return result;
}

