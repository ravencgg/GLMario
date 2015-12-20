#pragma once

#include "types.h"
#include "containers.h"
#include "console.h"
#include <vector>
#include <algorithm>


#define MAX_STATIC_COLLIDERS 2048
#define MAX_DYNAMIC_COLLIDERS 2048

#define COLLISION_EPSILON 0.001f

class Actor;
class Renderer;
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
//    uint32 collider_id;
    bool32 active;
    Rectf rect;
    Rectf aabb;
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

// NOTE: rotated rects using 2d rotation matrix?


// TODO: Max depth
// TODO: handle overflow
#define MAX_LEAF_SIZE 8
#define QUADTREE_CHILDREN 4
struct PhysicsNode
{
    uint16 depth;
    uint8 contained_colliders;
    bool is_parent;
    Rectf aabb;

    union
    {
        StaticCollider* colliders[MAX_LEAF_SIZE]; // Colliders are NOT contiguous in memory
        PhysicsNode* child_nodes;                    // Children  are contiguous in memory
    };
};

void AddCollider(PhysicsNode*, MemoryArena*, StaticCollider* collider);

// @untested! We currently don't allow the removal of static colliders so this hasn't been used
uint32 RemoveCollider(PhysicsNode*, StaticCollider*); // Returns the number of copies that were found and removed

StaticCollider** GetPotentialColliders(PhysicsNode* node, Rectf aabb, StaticCollider** collision_list, uint32* list_size);
void DrawBoundingBoxes(PhysicsNode*, Renderer*);
bool Contains(PhysicsNode*, Rectf rect);
bool Contains(PhysicsNode*, Vec2 point);
bool IsLeaf(PhysicsNode*);

class Physics
{
    RArray<StaticCollider, MAX_STATIC_COLLIDERS> statics;
    RArray<DynamicCollider, MAX_DYNAMIC_COLLIDERS> dynamics;

public:

    PhysicsNode quadtree;
    MemoryArena quadtree_memory;

    Physics();
    ~Physics();

    RArrayRef<StaticCollider> AddStaticCollider(StaticCollider);
    RArrayRef<StaticCollider> AddStaticCollider(Rectf);
    RArrayRef<DynamicCollider> AddDynamicCollider(DynamicCollider);

    void DestroyCollider(RArrayRef<DynamicCollider>);
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

namespace ColliderType
{
    enum Type { NONE, RECTANGLE, CIRCLE, COLLIDER_TYPE_COUNT };
}

struct PhysicsRect
{
    Rectf col_rect;
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

bool CheckCollision(const Rectf& m, Vec2 velocity, Rectf& other, CollisionInfo& out);





