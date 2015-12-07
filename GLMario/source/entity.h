#pragma once

#include "input.h"
#include "mathops.h"
#include "types.h"
#include "physics.h"
#include "containers.h"

struct GameState;
class SceneManager;

enum SceneType
{
    SceneType_Menu,
    SceneType_Cutscene,
    SceneType_Level,
};

enum EntityType
{
    EntityType_Null = 0,
    EntityType_Player,
    EntityType_Enemy,
    EntityType_Spawner,
    EntityType_Tile,

    EntityType_Count
};

enum EntityFlags
{
    EntityFlag_Enabled  = 0x1;
    EntityFlag_Removing = 0x2;

}

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
    uint32 health;
};
struct EntitySpawner
{
    float last_spawn_time;
    float time_between_spawns;
};

struct GameEntity
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
    };
};

void UpdateGameEntities(GameEntity* entities, uint32 num_entities, float dt);

void DrawGameEntities(GameEntity* entities, uint32 num_entities);

struct GameObject // particle system, parallax background
{

};


class Entity
{
public:
    Transform transform;
    SceneManager* parent_scene;
    EntityType entity_type;
    LIST_LINK(Entity) entity_group;
    bool delete_this_frame = false;

    Entity(SceneManager* sm) { parent_scene = sm; }
    virtual ~Entity() {}

    virtual void Tick(GameState*) {}
    virtual void Draw() {}
    virtual void SetPosition(Vec2 pos) { transform.position = pos; }
    virtual void Translate(Vec2 offset) { transform.position += offset; }
};

class Actor : public Entity
{
public:

	DrawCall draw_call;
    RArrayRef<DynamicCollider> collider;

    Actor(SceneManager* sm) : Entity(sm) { };
    virtual ~Actor() { }
    void SetPosition(Vec2 pos) { collider->position = pos; }
};


class Enemy : public Actor
{
public:
	Vec2 velocity;

	Enemy(SceneManager* sm);
    virtual ~Enemy() {}

    virtual void Tick(GameState*) override;
    virtual void Draw() override;
};

class Spawner : public Entity
{
public:

    float time_between_spawns;
    float last_spawn_time;

    void SpawnEnemy();

	Spawner(SceneManager* sm);
    virtual ~Spawner();

    virtual void Tick(GameState*) override;
    virtual void Draw() override;
};
