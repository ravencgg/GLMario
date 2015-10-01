#pragma once

#include "entity.h"
#include "input.h"
#include "renderer.h"
#include "console.h"
#include "particles.h"
#include "physics.h"

class SceneManager;

class Player : public Actor
{
public:

    Vec2 velocity;

	ParticleSystem ps;

    Player(SceneManager* sm);
    virtual ~Player() override;

    void Tick(float) override;
    void Draw() override;
};
