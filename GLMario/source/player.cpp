#include "scene_manager.h"

#include "player.h"
#if 1

Player::Player()
	: ps(ParticleSystem(2000))
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
	ps.draw_layer = DrawLayer::POST_TILEMAP;
	ps.ptd.options = ParticleOptions::NONE;

	draw_call = {};
	draw_call.draw_type = DrawType::SINGLE_SPRITE;
	draw_call.image = ImageFiles::MARIO_IMAGE; 
	draw_call.shader = ShaderTypes::DEFAULT_SHADER;
	draw_call.options = DrawOptions::TEXTURE_RECT;
	draw_call.sd.tex_rect.top = 903;
	draw_call.sd.tex_rect.left = 17;
	draw_call.sd.tex_rect.width = 34;
	draw_call.sd.tex_rect.height = 34;	
	draw_call.sd.world_size = vec2(1.0f, 1.5f);
	draw_call.sd.world_position = transform.position;
	draw_call.sd.draw_angle = 0;
}

void Player::Tick(float dt)
{
	const float gravity = -9.8f;
	static uint32 count = 0;
	Input* input = Input::get();

	this->velocity.y += gravity * dt;
	if(input->is_down(SDLK_SPACE))
	{
		velocity.y = 5.f;
	}	

	if(input->is_down(SDLK_d))
	{
		velocity.x += 10.f * dt;
	}	
	else if(input->on_down(SDLK_a))
	{
		velocity.x += -10.f * dt;
		//transform.position.x -= 1.f;
	}
	else
	{
		velocity.x = 0;
	}

	velocity = this->active_scene->process_motion(transform.position,
													make_rect(transform.position, vec2(1.f, 1.5f)),
													velocity);

	std::string conOut("Player velocity: " + ::to_string(velocity));
	Console::get()->log_message(conOut);


	//std::string p_info("Player x: " + std::to_string(transform.position.x) + "\nPlayer y: " + std::to_string(transform.position.y));
	//Console::get()->log_message(p_info);
	// sprite.angle += 0.1f;
	//transform.position += velocity;

	//TODO(cgenova): get rid of this, but maybe add it in for real? use a separate particle system for bursts and have the particles subject to high amounts of gravity or something.
	static bool draw_player = true;
	if(input->on_down(SDLK_p))
	{
		draw_player = !draw_player;
		ps.create_particle_burst(500);
	}
	ps.update(this->transform.position);
	ps.render();
	//sprite.color_mod.w = 0.5f;
	if(draw_player)
	{
		Renderer* ren = Renderer::get();
		draw_call.sd.world_position = transform.position;
		ren->push_draw_call(draw_call, DrawLayer::PLAYER);
		// ren->draw_sprite(&sprite, transform.position);
	}
}

#endif
