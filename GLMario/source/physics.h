#pragma once

#include "types.h"
#include "mathops.h"
#include "renderer.h"
#include "console.h"
#include <vector>
#include <algorithm>

#define MAX_STATIC_COLLIDERS 2048
#define MAX_DYNAMIC_COLLIDERS 2048

#define COLLISION_EPSILON 0.0001f

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
    float distance;
    Vec2 point;
    Vec2 projection;
    Vec2 normal;

    // TEMPORARY:
    Rectf mSumOther;

    // RDynamicCollider other;
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
    Rectf rect;
    Vec2 position; // Allows for non-centered colliders 

    std::vector<CollisionInfo> collisions;

    Actor* parent;
};

struct Ray
{
    Vec2 v0;
    Vec2 v1;
};

void ColliderSetPosition(RDynamicCollider, Vec2 position);
Vec2 ColliderGetPosition(RDynamicCollider col);

Rectf CanonicalRect(TDynamicCollider*);

class Physics
{
    TStaticCollider* statics;
    TDynamicCollider* dynamics;

    std::vector<uint32> active_statics;
    std::vector<uint32> active_dynamics;

    std::vector<uint32> inactive_statics;
    std::vector<uint32> inactive_dynamics;


public:

    Physics();
    ~Physics();

    RStaticCollider AddStaticCollider(TStaticCollider);
    RStaticCollider AddStaticCollider(Rectf);
    RDynamicCollider AddDynamicCollider(TDynamicCollider);

    void RemoveDynamicCollider(RDynamicCollider);
    void StepDynamicColliders(float);
    void DebugDraw();

    // Returns true if hits
    bool RaycastStatics(Vec2, Vec2, CollisionInfo&, bool draw = false);

    // Returns final collider position 
    Vec2 StepCollider(RDynamicCollider collider, Vec2& velocity, float dt);
	

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
