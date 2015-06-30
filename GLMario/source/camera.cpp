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

void Camera::update_matrices()
{
	cached_projection_matrix = orthographic_matrix(viewport_size.x, viewport_size.y, near, far, transform.position.xy());
	cached_view_matrix       = view_matrix(Vector3(1.f, 0, 0), Vector3(0, 1.f, 0), Vector3(0, 0, 1.f), Vector3(0, 0, 1.f)); 
	//cached_view_matrix = identity_matrix();
}