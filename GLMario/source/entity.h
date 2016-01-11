#pragma once

#include "types.h"
#include "time.h"
#include "mathops.h"
#include "renderer.h"

struct GameState;

enum SceneType
{
    SceneType_Menu,
    SceneType_Cutscene,
    SceneType_Level,
};

enum EntityType : uint32
{
    EntityType_Null = 0,
    EntityType_Player,
    EntityType_Enemy,
    EntityType_Spawner,
    EntityType_Camera,
    EntityType_Tile,

    EntityType_Count
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

struct EntityPlayer
{
    Vec2 velocity;
};
struct EntityEnemy
{
    Vec2 velocity;
    Timer despawn_timer;
    uint32 health;
};
struct EntitySpawner
{
    uint32 num_spawned;
    float last_spawn_time;
    float time_between_spawns;
};
struct EntityCamera
{
    uint32 follow_target_id;
    Camera camera;
};

struct Entity
{
    uint32 flags;
    uint32 id;
    EntityType type;
    Transform transform;
    Sprite sprite;

    bool delete_this_frame; // TODO: new thing for this

    union
    {
        EntityPlayer  player;
        EntityEnemy   enemy;
        EntitySpawner spawner;
        EntityCamera  camera;
    };
};

struct Scene;

// Run the entities update functions
void UpdateSceneEntities(GameState* game_state, Scene* scene);

// Run the entity draw functions
void DrawSceneEntities(Scene* scene, Renderer* renderer);

// Create the vtable at run-time
void BuildEntityVTable(Scene* scene);

//
bool FindEntityWithID(Scene* scene, uint32 id, Entity** out);

// Will be removed at the end of the frame
void RemoveEntity(Scene* scene, Entity* entity);

Entity* SpawnEntity(GameState* game_state, Scene* scene, EntityType type, Vec2 position);

typedef void (*DrawFunc)(Entity*, Renderer*);
#define EntityDrawFunc(name) static void name(Entity* entity, Renderer* renderer)

typedef void (*UpdateFunc)(GameState* game_state, Scene* scene, float dt, Entity*);
#define EntityUpdateFunc(name) static void name(GameState* game_state, Scene* scene, float dt, Entity* entity)

typedef void (*SpawnFunc)(GameState* game_state, Scene* scene, Entity* entity, Vec2 position);
#define EntitySpawnFunc(name) static void name(GameState* game_state, Scene* scene, Entity* entity, Vec2 position)

struct EntityVtable
{
    UpdateFunc update;
    DrawFunc   draw;
    SpawnFunc  spawn;
};

