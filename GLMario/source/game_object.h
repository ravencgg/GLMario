#pragma once

#include "mathops.h"

// TODO(chris): remove!
#include <iostream> 

class IDrawer;

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

	//TODO(cgenova): scrap all of this virtual shit
	GameObject() {};
	virtual ~GameObject() { std::cout << "In GO destructor" << std::endl; };

	virtual void update_and_draw() { std::cout << "In base update() " << std::endl; }
	virtual void paused_update_and_draw() {};
};
