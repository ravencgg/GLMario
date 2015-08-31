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
	Player(SceneManager* sm);

	DrawCall draw_call;
	ParticleSystem ps;

    void Tick(float) override;
    void Draw() override;
};
