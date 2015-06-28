#pragma once

#include "game_object.h"
#include "dynamic_array.h"
#include "player.h"
#include "input.h"
#include "camera.h"
#include "tilemap.h"


class SceneManager
{
public:
	SceneManager(IDrawer*, Camera*);
	~SceneManager();

	void update_scene();
	void add_object(GameObject* object);		
	void destroy_object(GameObject* object);

private:

	IDrawer* renderer;
	Input* input;
	Camera* main_camera;

	Tilemap tilemap;

#define USE_LINKED_LIST

#ifdef USE_LINKED_LIST
	struct GameObjectNode
	{
		GameObject* data = nullptr;
		GameObjectNode* next = nullptr;
	};
	GameObjectNode* first_game_object = nullptr;
	GameObjectNode* last_game_object = nullptr;
#else
	// TODO(chris): need a way of storing data as well, not just the pointers to the interface
	DynamicArray<GameObject*> game_objects;
#endif
};