#pragma once

#include "input.h"
#include "mathops.h"
#include "types.h"
#include "physics.h"

class SceneManager;

struct Transform
{
    Vec2 position;
    Vec2 scale;
    float rotation;
};

class Entity
{
public:
    Transform transform;
    bool delete_this_frame = false;
    SceneManager* parent_scene;

    Entity(SceneManager* sm) { parent_scene = sm; }
    virtual ~Entity() {}
    
    virtual void Tick(float) {}
    virtual void Draw() {}
    virtual void SetPosition(Vec2 pos) { transform.position = pos; }
};

class Actor : public Entity
{
public:
    Actor(SceneManager* sm) : Entity(sm) {}
    //virtual ~Actor() {if(collider.data} Physics::);

    void SetPosition(Vec2 pos) override { ::SetPosition(collider, pos); }

    RDynamicCollider collider;
};


class Enemy : public Actor 
{
public:
    Enemy(SceneManager* sm) : Actor(sm) {}
    virtual ~Enemy() {}

    virtual void Tick(float) override
    {
        
    }
};

//namespace EntityType
//{
//    enum Type { UNKNOWN, PLAYER, TILE, PARTICLE_SYSTEM, TILEMAP, ENTITY_TYPE_COUNT };
//}
//
//struct EntityTransform
//{
//    uint32 parent_id;
//    Vec2 position;
//};
//
//struct EntityDrawer
//{
//    uint32 parent_id;
//    DrawCall draw_call;
//    EntityTransform* transform;
//};
//
//struct EntityPhysics
//{
//    bool32 gravity;
//
//    uint32 parent_id;
//    ColliderType::Type col_type;
//
//    union
//    {
//        PhysicsRect pr;
//        PhysicsCircle pc;
//    };
//};
//
//struct PlayerEntity
//{
//	Vec2 velocity;
//    float acceleration;
//    float max_hor_vel;
//    Vec2 max_ver_vel;
//    DrawCall draw_call;
//};
//
//struct TileEntity
//{
//    bool32 dangerous;
//    DrawCall draw_call;
//};
//
//struct ParticleEntity
//{
//    ParticleSystem* ps;
//};
//
//struct Entity
//{
//    EntityType::Type type;
//    uint32 eid;
//    uint32 parent_eid;
//    Vec2 position;
//    float rotation;
//
//    union
//    {
//        PlayerEntity pe;
//        TileEntity te;
//        ParticleEntity pse;
//    };
//};
//
//
//void entity_update(Entity&, float);
//
//void draw_entity(EntityDrawer);
//
//void simulate_physics(EntityPhysics);
