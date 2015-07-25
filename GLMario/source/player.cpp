#include "player.h"

Player::Player()
	: input(Input::get()),
	  ps(ParticleSystem(20000)),
	  ren(Renderer::get())
{
	memset(attached_objects, 0, sizeof(attached_objects));

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
	ps.ptd.options |= ParticleOptions::LOCAL_SIM;
	ps.draw_layer = DrawLayer::PRE_TILEMAP;
	// ps.ptd.options |= ParticleOptions::NONE;

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

	// sprite.image_file = ImageFiles::MARIO_IMAGE;
	// sprite.shader_type = ShaderTypes::DEFAULT_SHADER;
	// sprite.layer = DrawLayer::FOREGROUND;
	// sprite.world_size = vec2(1.0f, 1.5f);
	// // sprite.world_size = Vec2(25.6f, 16.f);
	// sprite.angle = 0;
	// sprite.tex_rect.top = 903;
	// sprite.tex_rect.left = 17;
	// sprite.tex_rect.width = 34;
	// sprite.tex_rect.height = 34;
	// // sprite.tex_rect.top = 0;
	// // sprite.tex_rect.left = 0;
	// // sprite.tex_rect.width = 2560;
	// // sprite.tex_rect.height = 1600;
	// sprite.color_mod = vec4(1, 1, 1, 1);
}

void Player::attach_object(Transform* t)
{
	int32 empty_loc = -1;
	for (int i = 0; i < Player::max_attached_objects; ++i)
	{
		if (t == attached_objects[i]) return;
		else if (empty_loc < 0 && attached_objects[i] == nullptr) empty_loc = i;
	}

	if (empty_loc >= 0) attached_objects[empty_loc] = t;
}

void Player::detach_object(Transform* t)
{
	for (int i = 0; i < Player::max_attached_objects; ++i)
	{
		if (t == attached_objects[i])
		{
			attached_objects[i] = nullptr;
		}
	}
}

void Player::update_attached_objects()
{
	for (int i = 0; i < Player::max_attached_objects; ++i)
	{
		if (attached_objects[i])
			attached_objects[i]->position = transform.position;
	}
}

void Player::update_and_draw()
{
	static uint32 count = 0;
	if (input)
	{	
		if (input->on_down(SDLK_f))
		{
			delete_this_frame = true;
		}
	}

	if(input->is_down(SDLK_w))
	{
		velocity.y = 0.1f;
	}	
	else if(input->is_down(SDLK_s))
	{
		velocity.y = -0.1f;
	}
	else
	{
		velocity.y = 0;
	}

	if(input->is_down(SDLK_d))
	{
		velocity.x = 0.1f;
	}	
	else if(input->is_down(SDLK_a))
	{
		velocity.x = -0.1f;
	}
	else
	{
		velocity.x = 0;
	}

	std::string p_info("Player x: " + std::to_string(transform.position.x) + "\nPlayer y: " + std::to_string(transform.position.y));
	Console::get()->log_message(p_info);
	// sprite.angle += 0.1f;
	transform.position += velocity;
	update_attached_objects();

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
		draw_call.sd.world_position = transform.position;
		ren->push_draw_call(draw_call, DrawLayer::TILEMAP);
		// ren->draw_sprite(&sprite, transform.position);
	}
}

void Player::paused_update_and_draw()
{
	ren->draw_sprite(&sprite, transform.position);
} // Allows things to happen while the game is paused
