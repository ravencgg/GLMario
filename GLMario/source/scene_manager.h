#pragma once
#include <vector>

#include "entity.h"
#include "dynamic_array.h"
#include "player.h"
#include "input.h"
#include "camera.h"
#include "tilemap.h"
#include "physics.h"
#include "particles.h"

#include <algorithm>
#include <memory>

class SceneManager
{
public:
	SceneManager(Camera*);
	~SceneManager();

	void update_scene();

	Vec2 process_motion(Vec2&, Rectf, Vec2);

	Tilemap* get_tilemap() { return &tilemap; }
private:

	Renderer* renderer;
	Input* input;
	Camera* main_camera;
    Physics* physics;

	Tilemap tilemap;
	// Use resize() not clear() to "empty" vectors
	// std::vector<GameObject*> objects;
    std::vector<std::shared_ptr<Entity>> objects;
};
