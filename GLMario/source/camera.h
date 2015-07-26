#pragma once

#include "game_object.h"
#include "mathops.h"
#include "input.h"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp" 

class GameObject;

class Camera : public GameObject
{
public:
	Camera();
	virtual ~Camera() {}
	
	Vec2 viewport_size;

	glm::mat4x4 cached_projection_matrix;
	glm::mat4x4 cached_view_matrix;

	virtual void update_and_draw();
	void update_matrices();

private:

	float far = 10.f;
	float near = 0.1f;
};
