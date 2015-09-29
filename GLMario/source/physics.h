#pragma once

#include "types.h"
#include "mathops.h"
#include "renderer.h"
#include "console.h"
#include <vector>
#include <algorithm>
#include "dynamic_array.h"

#define MAX_STATIC_COLLIDERS 2048
#define MAX_DYNAMIC_COLLIDERS 2048

#define COLLISION_EPSILON 0.001f

class Actor;
struct TStaticCollider;
struct TDynamicCollider;

struct CollisionInfo
{
    // NOTE(cgenova): only generating collision info for the dynamic colliders right now
    float distance;
    Vec2 point;
    Vec2 projection;
    Vec2 normal;

    // TEMPORARY:
    Rectf mSumOther;

    // RDynamicCollider other;
};

// Should these get collision messages? They would need a parent pointer!
struct StaticCollider
{
    bool32 active;
    Rectf rect;
};

struct DynamicCollider
{
    bool32 active;
    Rectf rect;
    Vec2 position; // Allows for non-centered colliders

// TODO:Array
    std::vector<CollisionInfo> collisions;

    Actor* parent;
};

struct Ray
{
    Vec2 v0;
    Vec2 v1;
};

//void ColliderSetPosition(RDynamicCollider, Vec2 position);
//Vec2 ColliderGetPosition(RDynamicCollider col);

Rectf CanonicalRect(TDynamicCollider*);

class Physics
{
    RArray<StaticCollider, MAX_STATIC_COLLIDERS> statics;
    RArray<DynamicCollider, MAX_DYNAMIC_COLLIDERS> dynamics;

public:

    Physics();
    ~Physics();

    RArrayRef<StaticCollider> AddStaticCollider(StaticCollider);
    RArrayRef<StaticCollider> AddStaticCollider(Rectf);
    RArrayRef<DynamicCollider> AddDynamicCollider(DynamicCollider);

    void RemoveDynamicCollider(RArrayRef<DynamicCollider>);
    void StepDynamicColliders(float);
    void DebugDraw();

    // Returns true if hits
    bool RaycastStatics(Vec2, Vec2, CollisionInfo&, bool draw = false);

    // Returns final collider position
    Vec2 StepCollider(RArrayRef<DynamicCollider> collider, Vec2& velocity, float dt);


#ifdef _DEBUG
	static std::vector<Rectf> minkowski_rects;
	static void AddMinkowskiDebugRect(Rectf);
#endif
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

bool CheckCollision(Rectf& m, Vec2 velocity, Rectf& other, CollisionInfo& out);
