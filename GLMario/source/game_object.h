#pragma once

#include "types.h"
#include "mathops.h"
#include <iostream>

struct Transform
{
	Vec2 position;
	Vec2 scale;
	float rotation;
};

class GameObject
{
	
public:
	Transform transform;
	bool delete_this_frame = false;

	GameObject() {};
	GameObject(Transform t) : transform(t) {}

	virtual ~GameObject() {};
	virtual void update_and_draw() = 0; // { Console::get()->log_message(std::string("In base update() ")); }
};

#if 1
class Enemy : public GameObject
{
public:
	Enemy() : GameObject() {}
	Enemy(Transform t) : GameObject(t) {}
	virtual ~Enemy() {}

	virtual void update_and_draw() override
	{
		//Console::get()->log_message(std::string("Updating enemy object"));
	}
};

#endif
