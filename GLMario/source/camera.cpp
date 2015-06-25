#include "camera.h"



Camera::Camera()
{
	transform.position.z = -1.0f;
}

void Camera::update()
{

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
	// cached_view_matrix       = view_matrix(Vector3(1.f, 0, 0), Vector3(0, 1.f, 0), 
								// Vector3(0, 0, 1.f), Vector3(0, 0, 1.f)); 
	cached_view_matrix = identity_matrix();
}