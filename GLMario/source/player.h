#pragma once

#include "game_object.h"
#include "input.h"
#include "renderer.h"

class Player : public GameObject
{
public:

	Player();

	Input* input;
	Sprite sprite;

	static const uint32 max_attached_objects = 10;
	Transform* attached_objects[10];
	void attach_object(Transform*);
	void detach_object(Transform*);
	void update_attached_objects();

	virtual void update_and_draw(IDrawer*);
	virtual void paused_update_and_draw(IDrawer*); // Allows things to happen while the game is paused

private:
	Vector2 velocity;
};