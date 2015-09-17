#include "scene_manager.h"

void SceneManager::SetMainCamera(Camera* camera)
{
	this->main_camera = camera;
}

void SceneManager::render_random_particles()
{
#if 1	
	static Time* time = Time::get();
	static ParticleSystem ps1(this, 10000);
	ps1.draw_layer = DrawLayer::PRE_TILEMAP;
	//ps1.draw_layer = DrawLayer::POST_TILEMAP;
	static ParticleEmissionData data[2];
	static uint32 active_data = 0;
	
	static ParticleSystem ps2(this, 7500);
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

SceneManager::SceneManager()
	:renderer(Renderer::get()),
	 physics(new Physics()),
	 tilemap(physics),
	 input(Input::get())
{
	//tilemap.fill_checkerboard();
	tilemap.MakeWalledRoom(rect(0, 0, 10, 10));

    tilemap.AddTile(2, 2);

	//Entity* e = add_entity(EntityType::PLAYER, vec2(5.f, 5.f));
	//e->pe.draw_call.draw_type = DrawType::SINGLE_SPRITE;
	//e->pe.draw_call.image = ImageFiles::MARIO_IMAGE;
	//e->pe.draw_call.shader = ShaderTypes::DEFAULT_SHADER;
	//e->pe.draw_call.options = DrawOptions::TEXTURE_RECT;
	//e->pe.draw_call.sd.tex_rect.top = 903;
	//e->pe.draw_call.sd.tex_rect.left = 17;
	//e->pe.draw_call.sd.tex_rect.width = 34;
	//e->pe.draw_call.sd.tex_rect.height = 34;
	//e->pe.draw_call.sd.world_size = vec2(1.0f, 1.5f);
	//e->pe.draw_call.sd.draw_angle = 0;


    /*std::shared_ptr<GameObject> p = std::make_shared<Player>();
	objects.push_back(p);
	objects.push_back(std::make_shared<Enemy>());
*/
     std::shared_ptr<ParticleSystem> ps = std::make_shared<ParticleSystem>(this);
	 ps->initialize(1000, DrawLayer::FOREGROUND);
	 ps->ped.spawn_rate = 100;
	 ps->ped.spawn_size = vec2(20.f, 20.f);
	 ps->ped.lifetime = FRange(1.5f, 10.f);
	 objects.push_back(std::move(ps));
}

SceneManager::~SceneManager()
{
	if (physics)
	{
		delete physics;
		physics = nullptr;
	}
}

// Returns the post-move velocity
// Put this into the physics system instead of having it here.
Vec2 SceneManager::process_motion(Vec2& position, Rectf object, Vec2 velocity)
{
	//Vec2 result = velocity;
	//int32 check_size = 50;// max(2, (int32)(length(velocity) * 2.f));

	//PhysicsRect pObj = {};
	//pObj.col_rect = object;

	//Point2 start = { (int32)position.x - check_size, (int32)position.y - check_size };
	//Point2 end   = { (int32)position.x + check_size, (int32)position.y + check_size };

	//TileBlock tiles = tilemap.get_tile_block(start, end);
	//CollisionData cData = {};

	//bool collided = false;

	//for (uint32 i = 0; i < tiles.tile_info.size(); ++i)
	//{
	//	if (check_collision(pObj, velocity, tiles.tile_info[i], cData))
	//	{
	//		collided = true;
	//		break;
	//	}
	//}
	//

	//if (!collided)
	//{
	//	position += velocity * (float)Time::get()->delta_time;
	//}

	Vec2 result = {};
	return result;
}


void SceneManager::update_scene()
{
	tilemap.draw();
	render_random_particles();

	if (input->on_down(SDLK_n))
	{
		objects.push_back(std::make_shared<Enemy>(this));
	}
	if (input->on_down(SDLK_m))
	{
		std::shared_ptr<Player> p = std::make_shared<Player>(this);
		p->parent_scene = this;
		objects.push_back(std::move(p));
        objects.back()->SetPosition(vec2(5.1f, 5.2f));
	}

	Console::get()->log_message(std::string("Num objects: " + std::to_string(objects.size())));
    Time* time = Time::get();

	bool deleting = objects.size() > 10;

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		(*it)->Tick((float)time->delta_time);

		if ((*it)->delete_this_frame)
		{
			std::swap(*it, objects.back());
			objects.pop_back();
			--it;
		}

		if (deleting)
		{
			Entity* go = dynamic_cast<Entity*>(it->get());
			Enemy* enemy = dynamic_cast<Enemy*>(&(*it->get()));
			if (enemy && rand() % 2 == 0)
			{
				std::swap(*it, objects.back());
				objects.pop_back();
				--it;
			}
		}
	}

	static Vec2 start = { -3.f, 0 };
    static Vec2 velocity = { -1.f, 0 };

	//static Vec2 start = { 0, 0 };
 //   static Vec2 velocity = { 1, 1 };

	if (input->is_down(SDLK_v))
	{
		velocity.y -= (float)time->delta_time;
	}
	if (input->is_down(SDLK_g))
	{
		velocity.x += (float)time->delta_time;
	}
	if (input->is_down(SDLK_y))
	{
		velocity.y += (float)time->delta_time;
	}


    float outDistance = 0;
    std::vector<SimpleVertex> line;

	Vec4 red = vec4(1, 0, 0, 1);
	Vec4 green = vec4(0, 1, 0, 1);

	if (physics->RaycastStatics(start, velocity, outDistance, false))
	{
		Console::get()->log_message(std::string("Raycast hit at distance: ") + std::to_string(outDistance));
		line.push_back(SimpleVertex(start, red));
		Vec2 end = start + (Normalize(velocity) * outDistance);
		line.push_back(SimpleVertex(end, red));
	}
	else
	{
		line.push_back(SimpleVertex(start, green));
		line.push_back(SimpleVertex(start + velocity, green));
	}


	renderer->DrawLine(line, DrawLayer::UI);
	
    physics->StepDynamicColliders((float)time->delta_time);

    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        (*it)->Draw();
    }

    static bool draw_colliders = true;

    if(input->on_down(SDLK_b)) draw_colliders = !draw_colliders;

    if(draw_colliders)
    {
        physics->DebugDraw();
    }
}

