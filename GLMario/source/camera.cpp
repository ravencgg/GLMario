#include "camera.h"

Camera::Camera()
{
	viewport_size.x = 16;
	viewport_size.y = 9;

	transform.position.x = 0; //viewport_size.x / 2;
	transform.position.y = 0; //viewport_size.y / 2;
	transform.position.z = -1.0f;
}

void Camera::update()
{
	static Input* input = Input::get_instance();

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

void Camera::paused_update()
{

}

void Camera::draw(IDrawer*)
{
	
}
