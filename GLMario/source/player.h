#pragma once

#include "game_object.h"
#include "input.h"
#include "renderer.h"
#include "console.h"
#include "particles.h"

class Player : public GameObject
{
public:

	Vec2 velocity;
	
	Player();

	Input* input;
	Renderer* ren;
	Sprite sprite;
	DrawCall draw_call;

	ParticleSystem ps;

	static const uint32 max_attached_objects = 10;
	Transform* attached_objects[10];
	void attach_object(Transform*);
	void detach_object(Transform*);
	void update_attached_objects();

	virtual void update_and_draw();
	virtual void paused_update_and_draw(); // Allows things to happen while the game is paused

};
