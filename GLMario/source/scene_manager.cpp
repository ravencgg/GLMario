#include "scene_manager.h"


SceneManager::SceneManager(IDrawer* ren, Camera* cam)
	:renderer(ren),
	 main_camera(cam),
	 tilemap(16, 9)
{
	input = (Input::get());
#if defined(USE_LINKED_LIST)
	Player* player = new Player();
//	player->attach_object(&cam->transform);
	add_object(player);
#else
	game_objects.add(new Player());
#endif

	tilemap.fill_checkerboard();
}

SceneManager::~SceneManager()
{
#if defined(USE_LINKED_LIST)
	GameObjectNode* it = first_game_object; 
	GameObjectNode* temp;

	while(it)
	{
		temp = it;
		it = it->next;
		delete temp;
	}

#else

	for (uint32 i = 0; i < game_objects.size(); ++i)
	{
		delete game_objects.get(i);
	}
#endif
}

void SceneManager::update_scene()
{

	//static bool drawn = false;

	//if (!drawn)
	//{
	//	drawn = true;
	//}
	//tilemap.draw();

#ifdef USE_LINKED_LIST

	if(input->on_down(SDLK_g))
	{
		add_object(new Player());
	}
	else if(input->on_down(SDLK_h))
	{
		if (last_game_object)
		{
			destroy_object(last_game_object->data);
		}
	}


	GameObjectNode* it = first_game_object;
	GameObjectNode* prev = nullptr;
	while(it)
	{
		// TODO(chris): change to an update_and_draw(renderer) function instead?
		it->data->update_and_draw(renderer);

		if(it->data->delete_this_frame)
		{
			if(prev)
			{
				prev->next = it->next;

				if(!prev->next)
				{
					last_game_object = prev;
				}
				delete it;
				it = prev->next;
			}
			else
			{
				first_game_object = it->next;
				if(!first_game_object) last_game_object = nullptr;
				delete it;
				it = first_game_object;
			}
		}
		else
		{
			prev = it;
			it = it->next;
		}

	}

#else

	for (uint32 i = 0; i < game_objects.size(); ++i)
	{
		game_objects[i]->update_and_draw(renderer);

		if (game_objects[i]->delete_this_frame)
		{
			destroy_object(game_objects[i]);
		}
	}

#endif
	static graphics::ParticleSystem ps1(1000000);
	ps1.ped.spawn_position.x = -2.f;
	ps1.ped.spawn_rate = 200000;
	ps1.ped.lifetime.min = 1.0f;
	ps1.ped.lifetime.max = 10.f;
	// ps1.update();
	// ps1.render();

	static graphics::ParticleSystem ps2(1000000);
	static bool setup = false;
	if(!setup)
	{
		ps1.ptd.gravity = Vector2(0.0f, 1.0f);
		ps2.ptd.gravity = Vector2(-10.0f, -10.0f);

		ps2.ped.spawn_rate = 200000;
		setup = true;
		ps2.ped.spawn_position.x = 2.f;
		ps2.ped.start_color = Vector4(0, 1.f, 0, 1.f);
		ps2.ped.end_color   = Vector4(0.2f, 0.8f, 0.2f, 0.4f);
	}
	// ps2.update();
	// ps2.render();
}

void SceneManager::add_object(GameObject* object)
{

#ifdef USE_LINKED_LIST
	GameObjectNode* to_add = new GameObjectNode();
	to_add->data = object;

	if(!first_game_object)
	{
		first_game_object = to_add;
		last_game_object = to_add;	
	}
	else
	{
		last_game_object->next = to_add;
		last_game_object = to_add;
	}

#else

	game_objects.add(object);

#endif
}

void SceneManager::destroy_object(GameObject* object)
{
#ifdef USE_LINKED_LIST

	if(first_game_object->data == object)
	{
		GameObjectNode* temp = first_game_object;
		first_game_object = first_game_object->next;	
		if (!first_game_object) last_game_object = nullptr;
		delete temp;
	}
	else
	{

		GameObjectNode* it = first_game_object->next; 
		GameObjectNode* prev = first_game_object;
		while(it)
		{
			if(it->data == object)
			{
				prev->next = it->next;
				if(!prev->next) last_game_object = prev;
				delete it;
				break;
			}

			prev = it;
			it = it->next;
		}
	}
	
#else
	uint32 loc = 0;
	bool found = false;
	for (uint32 i = 0; i < game_objects.size(); ++i)
	{
		if (object == game_objects.get(i))
		{
			found = true;
			loc = i;
			break;
		}
	}

	assert(found);
	delete game_objects.get(loc);
	game_objects.remove(loc);
#endif
}