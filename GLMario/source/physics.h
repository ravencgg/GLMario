#pragma once

#include "types.h"
#include "mathops.h"

namespace ColliderType
{
    enum Type { NONE, RECTANGLE, CIRCLE, COLLIDER_TYPE_COUNT };
}

struct PhysicsRect
{
    Rectf col_rect;
};

struct PhysicsCircle
{
    Vec2 position;
    float radius;
};

inline
Rectf make_rect(Vec2 pos, Vec2 size)
{
   Rectf result = {}; 
   result.x = pos.x - size.x / 2.f;
   result.y = pos.y + size.y / 2.f;
   result.w = size.x;
   result.h = size.y;
   return result;
}

inline
PhysicsCircle make_physics_circle(Vec2 pos, float size)
{
    PhysicsCircle result = {};
    result.position = pos;
	result.radius = size;
    return result;
}

class PhysicsSim
{
public:
    
private:

};
