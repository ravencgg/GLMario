#pragma once
#include <vector>

#include "game_object.h"
#include "dynamic_array.h"
#include "player.h"
#include "input.h"
#include "camera.h"
#include "tilemap.h"
#include "particles.h"
#include <algorithm>
#include <memory>

#include "entity.h"

#define MAX_ENTITIES 1000

struct EntityArray
{
	size_t memory_size;
	uint8* memory = nullptr;
	Entity* entities = nullptr;
};

class SceneManager
{
public:
	SceneManager(Camera*);
	~SceneManager();

	void update_scene();
	void allocate_entity_array(uint32);

	EntityArray ea;
	uint32 active_entities;
	uint32 last_assigned_id;

	Entity* add_entity(EntityType::Type, Vec2, uint32 pid = 0);
	bool32 delete_entity(uint32);
	Entity* find_entity(uint32);

	Tilemap* get_tilemap() { return &tilemap; }
private:

	Renderer* renderer;
	Input* input;
	Camera* main_camera;

	Tilemap tilemap;
	// Use resize() not clear() to "empty" vectors
	// std::vector<GameObject*> objects;
    std::vector<std::shared_ptr<GameObject>> objects;
};
