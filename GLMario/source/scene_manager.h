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
    Renderer* renderer;
    Input* input;
    Camera* main_camera;
    Physics* physics;

    Tilemap tilemap;
    // Use resize() not clear() to "empty" vectors
    // std::vector<GameObject*> objects;
    std::vector<std::shared_ptr<Entity>> objects;

	SceneManager();
	~SceneManager();

	void update_scene();
	void SetMainCamera(Camera*);

    // Debug only
	void render_random_particles();
	
	Tilemap* GetTilemap() { return &tilemap; }
    Physics* GetPhysics() { return physics; }
};
