#include "scene_manager.h"

#include "player.h"
#if 1

Player::Player(SceneManager* sm)
	: Actor(sm)
    , ps(ParticleSystem(sm, 2000))
{
	ps.ped.spawn_position = vec2(0.f, 0.f);
	ps.ped.spawn_size = vec2(1.f, 1.5f);
	ps.ped.start_size = FRange(1.f, 4.f);
	ps.ped.lifetime = FRange(0.5f, 2.0f);
	ps.ped.min_start_speed = vec2(-1.91f, -1.91f);
	ps.ped.max_start_speed = vec2(1.91f, 1.91f);
	ps.ped.start_color = vec4(0.8f, 0, 1, 1.f);
	ps.ped.end_color = vec4(0.8f, 0.8f, 0.0f, 0.1f);
	ps.ped.spawn_rate = 100;
	ps.ptd.gravity = vec2(0, -0.8f);
	// ps.ptd.options |= ParticleOptions::LOCAL_SIM;
	ps.draw_layer = DrawLayer_PostTilemap;
	ps.ptd.options = ParticleOptions::NONE;

    Vec2 size = vec2(1.0f, 1.5f);

	draw_call = {};
	draw_call.draw_type = DrawType::SINGLE_SPRITE;
	draw_call.image = ImageFiles::MARIO_IMAGE;
	draw_call.shader = Shader_Default;
	draw_call.options = DrawOptions::TEXTURE_RECT;
    draw_call.sd.tex_rect = { 17, 903, 34, 34 };
	draw_call.sd.world_size = size;
	draw_call.sd.world_position = transform.position;
	draw_call.sd.draw_angle = 0;

    DynamicCollider col;
    col.active = true;
    col.position = transform.position;
    col.rect = { -size.x / 2.f,
                 -size.y / 2.f,
                 size.x,
                 size.y };
    col.parent = this;

    collider = parent_scene->physics->AddDynamicCollider(col);
    collider->position = transform.position;
}

Player::~Player()
{
    parent_scene->physics->DestroyCollider(collider);
}

void Player::Tick(GameState* game_state)
{
	const float gravity = -20.f;
	static uint32 count = 0;
    float dt = FrameTime(game_state);

	if(KeyIsDown(SDLK_SPACE))
	{
		velocity.y = 10.f;
	}
    else
    {
        velocity.y += gravity * dt;
        velocity.y = max(velocity.y, -100.0f);
    }

	if(KeyIsDown(SDLK_d))
	{
		velocity.x += 50.f * dt;
        velocity.x = min(velocity.x, 5.0f);
	}
	else if(KeyIsDown(SDLK_a))
	{
		velocity.x -= 50.f * dt;
        velocity.x = max(velocity.x, -5.0f);
	}
	else
	{
		velocity.x = 0;
	}

	if(KeyIsDown(SDLK_1))
    {
        delete_this_frame = true;
    }

	DebugPrintf("Player collider.data->velocity: (%.2f, %.2f)", velocity.x, velocity.y);

    DebugPrintf("Player position: (%.2f, %.2f)", transform.position.x, transform.position.y);

    Vec2 old_velocity = velocity;

    transform.position = parent_scene->physics->StepCollider(collider, velocity, FrameTime(game_state));

    const uint8 line_width = 3;
    Renderer::get()->DrawLine(transform.position,  transform.position + old_velocity, vec4(0, 1, 1, 1), line_width);

    DebugPrintf("Player collider.data->velocity: (%.2f, %.2f)", velocity.x, velocity.y);

    ps.update(game_state, this->transform.position);
	//std::string p_info("Player x: " + std::to_string(transform.position.x) + "\nPlayer y: " + std::to_string(transform.position.y));
	//Console::get()->log_message(p_info);
	//draw_call.sd.draw_angle += 0.1f;

    if(KeyFrameDown(SDLK_p))
    {
        ps.create_particle_burst(500);
    }
}

void Player::Draw()
{
    ps.render();

    draw_call.sd.world_position = transform.position;
    Renderer::get()->push_draw_call(draw_call, DrawLayer_Player);
}

#endif
