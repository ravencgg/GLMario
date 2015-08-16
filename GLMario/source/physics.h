#pragma once

#include "types.h"
#include "mathops.h"
#include <vector>

#define Max_Static_Colliders 2048
#define Max_Dynamic_Colliders 2048

class Actor;
struct TStaticCollider;
struct TDynamicCollider;

struct RStaticCollider
{
    TStaticCollider* data;
    uint32 array_index;
};

struct RDynamicCollider
{
    TDynamicCollider* data;
    uint32 array_index;
};

struct CollisionInfo
{
    // NOTE(cgenova): only generating collision info for the dynamic colliders right now
    RDynamicCollider other;
};

// Should these get collision messages? They would need a parent pointer!
struct TStaticCollider
{
    bool32 active;
    Rectf rect;
};

struct TDynamicCollider
{
    bool32 active;
    Vector2 position;
    Vector2 velocity;

    std::vector<CollisionInfo> collisions;

    Actor* parent;
};

class Physics
{
    TStaticCollider* statics;
    TDynamicCollider* dynamics;

    std::vector<uint32> active_statics;
    std::vector<uint32> active_dynamics;

// TODO(cgenova): sorting!
    std::vector<uint32> inactive_statics;
    std::vector<uint32> inactive_dynamics;

    static Physics* s_physics;

public:

    Physics();
    ~Physics();

    RStaticCollider AddStaticCollider(TStaticCollider);
    RDynamicCollider AddDynamicCollider(TDynamicCollider);

    void RemoveDynamicCollider(RDynamicCollider);

    void Step(float);

    void DebugDraw();

	// Singleton style for physics? would it be better to use this class as a zone for optimization?
    static void Get();
};

struct Ray
{
	Vec2 v0;
	Vec2 v1;
};

inline
Ray make_ray(Vec2 a, Vec2 b)
{
	Ray result = { a, b };
	return result;
}

inline
Vec2 rect_center(Rectf r)
{
	Vec2 result = { r.x + r.w / 2.f, r.y + r.h / 2.f };
	return result;
}

struct CollisionData
{
	float distance;
	Vec2 point;
};

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
   result.y = pos.y - size.y / 2.f;
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

bool check_collision(PhysicsRect&, Vec2& velocity, PhysicsRect&, CollisionData&);
