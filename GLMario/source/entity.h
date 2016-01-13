#pragma once

#include "types.h"
#include "time.h"
#include "mathops.h"
#include "renderer.h"

enum EntityType
{
    EntityType_Null = 0,
    EntityType_Player,
    EntityType_Enemy,
    EntityType_Spawner,
    EntityType_Camera,

    EntityType_Count
};

// NOTE: could just use 32 bits for both, but don't need 4 billion simultaneous entity indexes
struct EntityID
{
    uint32 index      : 12;
    uint32 generation : 20;
};

enum EntityFlags
{
    EntityFlag_Enabled  = 0x1,
    EntityFlag_Removing = 0x2,

};

struct Transform
{
    Vec2 position;
    Vec2 scale;
    float rotation;
};


struct Entity
{
    EntityID id;
    EntityType type;
    uint32 flags;
    Transform transform;
    Vec2 size;

    Sprite sprite;

    bool delete_this_frame; // TODO: new thing for this

    union
    {
        struct
        {
            Vec2 velocity;
        }player;

        struct
        {
            Vec2 velocity;
            Timer despawn_timer;
            uint32 health;
        }enemy;

        struct 
        {
            uint32 num_spawned;
            float last_spawn_time;
            float time_between_spawns;
        }spawner;

        struct
        {
            EntityID follow_target_id;
            Camera camera;
        }camera;
    };
};

struct Scene;

// Run the entities update functions
void UpdateSceneEntities(GameState* game_state, Scene* scene);

// Run the entity draw functions
void DrawSceneEntities(Scene* scene, Renderer* renderer);

//
bool GetEntityWithID(Scene* scene, EntityID, Entity** out);

// Will be removed at the end of the frame
void RemoveEntity(Scene* scene, Entity* entity);

Entity* SpawnEntity(GameState* game_state, Scene* scene, EntityType type, Vec2 position);
