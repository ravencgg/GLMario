#pragma once

#include "input.h"
#include "mathops.h"
#include "types.h"
#include "physics.h"
#include "containers.h"

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

    virtual void Tick(float) override;
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

    virtual void Tick(float) override;
    virtual void Draw() override;
};