#include "camera.h"

#include "scene_manager.h"

Camera::Camera(SceneManager* sm)
	: Entity(sm)
{
	viewport_size.x = 16;
	viewport_size.y = 9;

	transform.position.x = 0;
	transform.position.y = 0;
}

void Camera::SetFollowTarget(RArrayRef<Entity*> new_target)
{
    if(parent_scene->ValidRef(new_target))
    {
        follow_target = new_target;
    }
}

void Camera::Tick(GameState* game_state)
{
    if (parent_scene->ValidRef(follow_target))
    {
        transform.position = (*follow_target)->transform.position;
    }

	if(KeyIsDown(SDLK_LEFT))
	{
		transform.position.x -= 0.1f;
	}

	if(KeyIsDown(SDLK_RIGHT))
	{
		transform.position.x += 0.1f;
	}

	if(KeyIsDown(SDLK_UP))
	{
		// transform.position.y += 0.1f;
		viewport_size *= 1.1f;
	}

	if(KeyIsDown(SDLK_DOWN))
	{
		// transform.position.y -= 0.1f;
		viewport_size *= 0.9f;
	}
}

