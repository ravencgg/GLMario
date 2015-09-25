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
    virtual void Translate(Vec2 offset) { transform.position += offset; }
};

class Actor : public Entity
{
public:
	DrawCall draw_call;
    RDynamicCollider collider;


    Actor(SceneManager* sm) : Entity(sm) {}
    // TODO(cgenova): remove collider from active list in destructor;
    //virtual ~Actor() {if(collider.data} Physics::);

    void SetPosition(Vec2 pos) override { ColliderSetPosition(collider, pos); }
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
