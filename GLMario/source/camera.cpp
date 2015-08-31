#include "camera.h"

Camera::Camera(SceneManager* sm)
	: Entity(sm)
{
	viewport_size.x = 16;
	viewport_size.y = 9;

	transform.position.x = 0;
	transform.position.y = 0; 
}

void Camera::Tick(float dt)
{
	static Input* input = Input::get();

	if(input->is_down(SDLK_LEFT))
	{
		transform.position.x -= 0.1f;
	}

	if(input->is_down(SDLK_RIGHT))
	{
		transform.position.x += 0.1f;
	}

	if(input->is_down(SDLK_UP))
	{
		// transform.position.y += 0.1f;
		viewport_size *= 1.1f;
	}

	if(input->is_down(SDLK_DOWN))
	{
		// transform.position.y -= 0.1f;
		viewport_size *= 0.9f;
	}
}
