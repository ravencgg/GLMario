#include "scene_manager.h"


SceneManager::SceneManager(Camera* cam)
	:renderer(Renderer::get()),
	 main_camera(cam),
	 tilemap(16, 9),
	 input(Input::get())
{
	allocate_entity_array(MAX_ENTITIES);

	//tilemap.fill_checkerboard();
	tilemap.fill_walled_room();

	Entity* e = add_entity(EntityType::PLAYER, vec2(5.f, 5.f));
	e->pe.draw_call.draw_type = DrawType::SINGLE_SPRITE;
	e->pe.draw_call.image = ImageFiles::MARIO_IMAGE;
	e->pe.draw_call.shader = ShaderTypes::DEFAULT_SHADER;
	e->pe.draw_call.options = DrawOptions::TEXTURE_RECT;
	e->pe.draw_call.sd.tex_rect.top = 903;
	e->pe.draw_call.sd.tex_rect.left = 17;
	e->pe.draw_call.sd.tex_rect.width = 34;
	e->pe.draw_call.sd.tex_rect.height = 34;
	e->pe.draw_call.sd.world_size = vec2(1.0f, 1.5f);
	e->pe.draw_call.sd.draw_angle = 0;


    /*std::shared_ptr<GameObject> p = std::make_shared<Player>();
	objects.push_back(p);
	objects.push_back(std::make_shared<Enemy>());
*/
    std::shared_ptr<ParticleSystem> ps = std::make_shared<ParticleSystem>();
	ps->initialize(100000, DrawLayer::FOREGROUND);
	ps->ped.spawn_rate = 100000;
	ps->ped.lifetime = FRange(1.5f, 10.f);
	objects.push_back(std::move(ps));
}

SceneManager::~SceneManager()
{
	if(ea.memory) delete[] ea.memory;
}

void SceneManager::allocate_entity_array(uint32 max_entities)
{
	ea.memory_size = sizeof(Entity) * max_entities;
	ea.memory = new uint8[ea.memory_size];
	ea.entities = (Entity*)ea.memory;	
}

// Just create the basic entity in here, then the creator can modify with the returned pointer
Entity* SceneManager::add_entity(EntityType::Type t, Vec2 pos, uint32 pid)
{
	bool32 valid = false;
	Entity* result = nullptr;
	Entity new_entity = {};

	//NOTE(cgenova); no entity has id = 0
	new_entity.eid = ++last_assigned_id;
	new_entity.type = t;
	new_entity.parent_eid = pid;
	new_entity.position = pos;

	switch(t)
	{
		case EntityType::PLAYER:
		{
			valid = true;
			new_entity.pe.acceleration = 2.0f;
			new_entity.pe.max_hor_vel = 3.0f;
			new_entity.pe.max_ver_vel = vec2(5.0f, 10.0f);
		} break;
		case EntityType::TILE:
		{
			valid = true;


		}break;
		case EntityType::PARTICLE_SYSTEM:
		{
			valid = true;


		}break;
		case EntityType::TILEMAP:
		default:
		{
			assert(0);
		}break;
	}

	if(valid)
	{
		result = ea.entities + active_entities++;
		*result = new_entity;
	}
	return result; 
}

Entity* SceneManager::find_entity(uint32 eid)
{
	Entity* result = nullptr;
	for(uint32 i = 0; i < active_entities; ++i)
	{
		if(eid == ea.entities[i].eid)	
		{
			result = ea.entities + i;
			break;
		}
	}	
	return result;
}

bool32 SceneManager::delete_entity(uint32 eid)
{
	bool32 result = false;

	for(uint32 i = 0; i < active_entities; ++i)
	{
		if(eid == ea.entities[i].eid)	
		{
			ea.entities[i] = ea.entities[active_entities - 1];
			ea.entities[active_entities - 1].type = EntityType::UNKNOWN;	
			--active_entities;
			result = true;
			break;
		}
	}

	return result;
}

void render_random_particles()
{
#if 0	
	static Time* time = Time::get();
	static ParticleSystem ps1(10000);
	ps1.draw_layer = DrawLayer::PRE_TILEMAP;
	static ParticleEmissionData data[2];
	static uint32 active_data = 0;
	
	static ParticleSystem ps2(7500);
	ps2.draw_layer = DrawLayer::PRE_TILEMAP;
	static bool setup = false;
	if(!setup)
	{
		setup = true;

		ps1.ped.spawn_position.x = -2.f;
		ps1.ped.spawn_rate = 1000;
		ps1.ped.spawn_size.x = 1.f;
		ps1.ped.spawn_size.y = 1.f;
		ps1.ped.lifetime.min = 9.0f;
		ps1.ped.lifetime.max = 10.f;
		ps1.ptd.gravity = vec2(-10.0f, 1.0f);
		
		ps2.ptd.gravity = vec2(1.0f, 0.3f);
		ps2.ped.spawn_size.x = 1.f;
		ps2.ped.spawn_size.y = 1.f;
		ps2.ped.spawn_rate = 1000;
		ps2.ped.spawn_position.x = 2.f;
		ps2.ped.start_color = vec4(0, 1.f, 0, 1.f);
		ps2.ped.end_color   = vec4(0.2f, 0.8f, 0.2f, 0.4f);

		data[0] = ps2.ped;
		data[1] = ps2.ped;
		data[0].start_color = vec4(1, 0, 0, 1);
		data[0].end_color   = vec4(0, 1, 0, 0.5f);
		data[1].start_color = vec4(0, 1, 0, 1);
		data[1].end_color   = vec4(1, 1, 0, 0);
	}

	ps2.ped.start_color = vec4((float)sin(time->current_time), (float)cos(time->current_time), 1, 1.f);
	ps2.ped.end_color = vec4(0.2f, (float)sin(time->current_time), (float)cos(time->current_time), 0.4f);

	static Timer timer(1.0f);
	if (timer.is_finished())
	{
		timer.reset();
		active_data ^= 1;
		ps2.ped = data[active_data];
	}

	ps1.update();
	ps1.render();
	ps2.update();
	ps2.render();
#endif
}

void SceneManager::update_scene()
{
	tilemap.draw();
	//render_random_particles();
	
	for(uint32 i = 0; i < active_entities; ++i)
	{
		entity_update(ea.entities[i], (float)Time::get()->delta_time);
	}

	if (input->on_down(SDLK_n))
	{
		objects.push_back(std::make_shared<Enemy>());
	}
	if (input->on_down(SDLK_m))
	{
		objects.push_back(std::make_shared<Player>());
	}

	Console::get()->log_message(std::string("Num objects: " + std::to_string(objects.size())));

	bool deleting = objects.size() > 10;

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->update_and_draw();

		if ((*it)->delete_this_frame)
		{
			std::swap(*it, objects.back());
			objects.pop_back();
			--it;
		}

		if (deleting)
		{
			GameObject* go = dynamic_cast<GameObject*>(it->get());
			Enemy* enemy = dynamic_cast<Enemy*>(&(*it->get()));
			if (enemy && rand() % 2 == 0)
			{
				std::swap(*it, objects.back());
				objects.pop_back();
				--it;
			}
		}
	}
}

