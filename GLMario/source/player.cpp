#include "player.h"

Player::Player()
	: input(Input::get_instance())
{
	memset(attached_objects, 0, sizeof(attached_objects));

	sprite.image_file = ImageFiles::MARIO_IMAGE;
	sprite.shader_type = ShaderTypes::DEFAULT_SHADER;
	sprite.layer = DrawLayer::FOREGROUND;
	sprite.world_size = Vector2(1.0f, 1.5f);
	sprite.angle = 0;
	sprite.tex_rect.top = 903;
	sprite.tex_rect.left = 17;
	sprite.tex_rect.width = 34;
	sprite.tex_rect.height = 34;
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

	// sprite.angle += 0.1f;
	transform.position += Vector3(velocity);
	update_attached_objects();

	drawer->draw_sprite(&sprite, transform.position.xy());
}

void Player::paused_update_and_draw(IDrawer* drawer)
{
	drawer->draw_sprite(&sprite, transform.position.xy());
} // Allows things to happen while the game is paused
