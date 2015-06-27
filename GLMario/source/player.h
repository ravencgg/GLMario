#pragma once

#include "game_object.h"
#include "input.h"
#include "renderer.h"

class Player : public GameObject
{
public:

	Input* input;
	Sprite sprite;

	Player();

	virtual void update_and_draw(IDrawer*);
	virtual void paused_update_and_draw(IDrawer*); // Allows things to happen while the game is paused


private:
	Vector2 velocity;
};