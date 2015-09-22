#pragma once

#include "entity.h"
#include "mathops.h"
#include "input.h"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp" 
#include "types.h"


class Camera : public Entity 
{
public:
	Camera(SceneManager*);
	virtual ~Camera() {}
	
	Vec2 viewport_size;

	glm::mat4x4 cached_projection_matrix;
	glm::mat4x4 cached_view_matrix;

    Vec2 ScreenToWorldPoint(Point2 input);

	virtual void Tick(float dt) override;
	void update_matrices();

private:

	float far = 10.f;
	float near = 0.1f;
};
