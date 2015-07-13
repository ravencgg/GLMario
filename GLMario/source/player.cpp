#include "player.h"

Player::Player()
	: input(Input::get()),
	  ps(graphics::ParticleSystem(200000))
{
	memset(attached_objects, 0, sizeof(attached_objects));

	ps.ped.spawn_position	= Vector2(0.f, 0.f);
	ps.ped.spawn_size = Vector2(0.5f, 0.75f);
	ps.ped.start_size = FRange(1.f, 10.f);
	ps.ped.lifetime = FRange(0.5f, 1.0f);
	ps.ped.min_start_speed = Vector2(-0.5f, -0.5f);
	ps.ped.max_start_speed = Vector2(0.5f, 0.5f);
	ps.ped.start_color = Vector4(1.f, 0, 0, 1.f);
	ps.ped.end_color = Vector4(0.0f, 1.f, 0.f, 1.f);
	ps.ped.spawn_rate = 2500;
	// This is fucking stupid........
	ps.ptd.options = (graphics::ParticleOptions)((uint32)ps.ptd.options | (uint32)graphics::ParticleOptions::WORLD_SPACE_TRANSFORM);
	//ps.ptd.options = graphics::ParticleOptions::NONE;// (graphics::ParticleOptions)((uint32)ps.ptd.options | (uint32)graphics::ParticleOptions::WORLD_SPACE_TRANSFORM);

	sprite.image_file = ImageFiles::MARIO_IMAGE;
	sprite.shader_type = ShaderTypes::DEFAULT_SHADER;
	sprite.layer = DrawLayer::FOREGROUND;
	sprite.world_size = Vector2(1.0f, 1.5f);
	sprite.angle = 0;
	sprite.tex_rect.top = 903;
	sprite.tex_rect.left = 17;
	sprite.tex_rect.width = 34;
	sprite.tex_rect.height = 34;
	sprite.color_mod = Vector4(1, 1, 1, 1);
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

void Player::update_and_draw(IDrawer* drawer)
{
	static uint32 count = 0;
	if (input)
	{	
		std::cout << "Player is alive: " << count++ << std::endl;
		std::cout << "Press 'f' to kill player" << std::endl;
		if (input->on_down(SDLK_f))
		{
			delete_this_frame = true;
		}
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

	if(input->is_down(SDLK_SPACE))
	{
		velocity.y = 1.0f;	
	}
	else
	{
		velocity.y = 0;
	}


	std::string p_info("Player x: " + std::to_string(transform.position.x) + "\nPlayer y: " + std::to_string(transform.position.y));
	Console::get()->log_message(p_info);
	// sprite.angle += 0.1f;
	transform.position += Vector3(velocity);
	update_attached_objects();


	ps.update(this->transform.position.xy());
	ps.render();
	//sprite.color_mod.w = 0.5f;
	drawer->draw_sprite(&sprite, transform.position.xy());
}

void Player::paused_update_and_draw(IDrawer* drawer)
{
	drawer->draw_sprite(&sprite, transform.position.xy());
} // Allows things to happen while the game is paused
