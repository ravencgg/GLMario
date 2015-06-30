#pragma once

#include "game_object.h"
#include "mathops.h"
#include "input.h"

class Camera : public GameObject
{
public:

	Camera();
	
	Vector2 viewport_size;

	Mat4 cached_projection_matrix;
	Mat4 cached_view_matrix;

	virtual void update();
	virtual void paused_update();
	virtual void draw(IDrawer*);

	void update_matrices();

private:

	float far = 10.f;
	float near = 0.1f;
};