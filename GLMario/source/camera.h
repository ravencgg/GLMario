#pragma once

#include "game_object.h"
#include "mathops.h"

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

	float near = 10.f;
	float far  = 0.1f;
};