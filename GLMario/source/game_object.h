#pragma once

#include "mathops.h"

// TODO(chris): remove!
#include <iostream> 

class IDrawer;

struct Transform
{
	Vector3 position;
	Vector3 scale;
	Vector3 rotation;
};

class GameObject
{
	
public:
	Transform transform;
	bool delete_this_frame = false;

	GameObject() {};
	virtual ~GameObject() { std::cout << "In GO destructor" << std::endl; };

	virtual void update_and_draw(IDrawer*) { std::cout << "In base update() " << std::endl; }
	virtual void paused_update_and_draw(IDrawer*) {};
};