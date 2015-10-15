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
	tilemap.MakeWalledRoom(rect(-50, -20, 50, 50));
    tilemap.MakeWalledRoom(rect(-5, -20, 30, 3));
    tilemap.MakeWalledRoom(rect(-25, -10, 5, 10));
    tilemap.MakeWalledRoom(rect(20, 20, 10, 10));
    tilemap.MakeWalledRoom(rect(-5, -2, 10, 2));

    tilemap.AddTile(2, 2);
//    std::shared_ptr<ParticleSystem> ps = std::make_shared<ParticleSystem>(this);
    ParticleSystem* ps = new ParticleSystem(this);
    ps->initialize(1000, DrawLayer::FOREGROUND);
    ps->ped.spawn_rate = 100;
    ps->ped.spawn_size = vec2(20.f, 20.f);
    ps->ped.lifetime = FRange(1.5f, 10.f);
    //objects.push_back(std::move(ps));
    objects.Add(ps);

    AddEntity(new Spawner(this));
}

SceneManager::~SceneManager()
{
	if (physics)
	{
		delete physics;
		physics = nullptr;
	}
}

void SceneManager::AddEntity(Entity* p)
{
    //Entity* p = new Enemy(this);
    //p->parent_scene = this;
    //objects.push_back(std::move(p));
    //objects.back()->SetPosition(vec2(0.5f, 4.f));

    auto obj = objects.Add(p);
    (*obj.ptr)->SetPosition(vec2(0.5f, 4.f));
}

void SceneManager::update_scene()
{
	tilemap.draw();

    std::string tileString("Active Tiles: ");
    tileString.append(std::to_string(tilemap.tiles.size()));
    Console::get()->log_message(tileString);

	render_random_particles();

	if (input->on_down(SDLK_n))
	{
        // TODO: is created inside of a rect and throws an assert. Make spawning check for collisions
		//objects.push_back(std::make_shared<Enemy>(this));
		objects.Add(new Enemy(this));
	}
	if (input->on_down(SDLK_m))
	{
        if(input->is_down(SDLK_LSHIFT))
        {
//			std::shared_ptr<Enemy> p = std::make_shared<Enemy>(this);
            Entity* p = new Enemy(this);
			p->parent_scene = this;
			//objects.push_back(std::move(p));
			//objects.back()->SetPosition(vec2(0.5f, 4.f));
			auto obj = objects.Add(p);
			(*obj.ptr)->SetPosition(vec2(0.5f, 4.f));
        }
		else
		{
			//std::shared_ptr<Player> p = std::make_shared<Player>(this);
            Entity* p = new Player(this);
			p->parent_scene = this;
			//objects.push_back(std::move(p));
			//objects.back()->SetPosition(vec2(0.5f, 4.f));
			auto obj = objects.Add(p);
			(*obj.ptr)->SetPosition(vec2(0.5f, 4.f));
		}
	}

	Console::get()->log_message(std::string("Num objects: " + std::to_string(objects.Size())));
    Time* time = Time::get();

	bool deleting = objects.Size() > 10;

//	for (auto it = objects.begin(); it != objects.end(); ++it)
//	{
//		(*it)->Tick((float)time->delta_time);
//
//		if ((*it)->delete_this_frame)
//		{
//			std::swap(*it, objects.back());
//			objects.pop_back();
//			--it;
//		}
//	}

    Array<RArrayRef<Entity*>> to_delete;
    uint32 counter = 0;
    uint32 max = objects.Size();
    for (uint32 index = 0; counter < max; ++counter, ++index)
  	{
        RArrayRef<Entity*> obj = objects.GetRef(index);
        (*obj)->Tick((float)time->delta_time);
        
        // NOTE: Can I just do a --i and delete in place?
  		if ((*obj)->delete_this_frame)
  		{
            delete (*obj.ptr);
            objects.Remove(obj);
            --index;
  		}
  	}

    for(uint32 i = 0; i < to_delete.Size(); ++i)
    {
        objects.Remove(to_delete[i]);
    }

	static Vec2 start = { -3.5f, 4.0f };
    static Vec2 velocity = { 0, -1.f };

	if (input->is_down(SDLK_k))
	{
		velocity.y -= (float)time->delta_time;
	}
    if (input->is_down(SDLK_i))
    {
        velocity.y += (float)time->delta_time;
    }
    if (input->is_down(SDLK_j))
    {
        velocity.x -= (float)time->delta_time;
    }
	if (input->is_down(SDLK_l))
	{
		velocity.x += (float)time->delta_time;
	}

    float outDistance = 0;
    std::vector<SimpleVertex> line;

	Vec4 red = vec4(1, 0, 0, 1);
	Vec4 green = vec4(0, 1, 0, 1);
    Vec4 yellow = vec4(1, 1, 0, 1);
    Vec4 magenta = vec4(1, 0, 1, 1);
    Vec4 white = vec4(1, 1, 1, 1);

    CollisionInfo ci = {};
	if (physics->RaycastStatics(start, velocity, ci, false))
	{
		Console::get()->log_message(std::string("Raycast hit at distance: ") + std::to_string(ci.distance));
		line.push_back(SimpleVertex(start, red));
		Vec2 end = start + (Normalize(velocity) * ci.distance);
		line.push_back(SimpleVertex(end, red));
        line.push_back(SimpleVertex(end + ci.normal, white));

        float halfSize = 0.1f;
        float size = halfSize * 2.f;
        renderer->DrawRect(rectf(ci.point.x - halfSize, ci.point.y - halfSize, size, size), DrawLayer::UI, yellow);
        renderer->DrawRect(rectf(ci.point.x + ci.projection.x - halfSize, ci.point.y + ci.projection.y - halfSize, size, size), DrawLayer::UI, magenta);
        // renderer->DrawRect(rectf(ci.projection.x - halfSize, ci.projection.y - halfSize, size, size), DrawLayer::UI, magenta);
	}
	else
	{
		line.push_back(SimpleVertex(start, green));
		line.push_back(SimpleVertex(start + velocity, green));
	}

	renderer->DrawLine(line, DrawLayer::UI);

    //for (auto it = objects.begin(); it != objects.end(); ++it)
    //{
    //    (*it)->Draw();
    //}
    for (uint32 i = 0; i < objects.Size(); ++i)
    {
        auto o = objects.GetRef(i);
        (*o.ptr)->Draw();
        //(*it)->Draw();
    }

    static bool draw_colliders = true;

    if(input->on_down(SDLK_b)) draw_colliders = !draw_colliders;

    if(draw_colliders)
    {
        physics->DebugDraw();
    }
}

