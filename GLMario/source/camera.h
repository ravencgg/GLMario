#pragma once

#include "entity.h"
#include "mathops.h"
#include "input.h"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp" 


class Camera : public Entity 
{
public:
	Camera();
	virtual ~Camera() {}
	
	Vec2 viewport_size;

	glm::mat4x4 cached_projection_matrix;
	glm::mat4x4 cached_view_matrix;

	virtual void Tick(float dt) override;
	void update_matrices();

private:

	float far = 10.f;
	float near = 0.1f;
};
