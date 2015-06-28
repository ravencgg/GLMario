#include "player.h"

Player::Player()
: input(Input::get_instance())
{
	sprite.image_file = MARIO_IMAGE; 
	sprite.shader_type = DEFAULT_SHADER;
	sprite.layer = FOREGROUND;
	sprite.world_size = Vector2(1.0f, 1.5f);
	sprite.angle = 0;
	sprite.tex_rect.top = 903;
	sprite.tex_rect.left = 17;
	sprite.tex_rect.width = 34;
	sprite.tex_rect.height = 34;
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
		velocity.x += 0.1f;
	}	
	if(input->is_down(SDLK_a))
	{
		velocity.x -= 0.1f;
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

	drawer->draw_sprite(&sprite, transform.position.xy());
}

void Player::paused_update_and_draw(IDrawer* drawer)
{
	drawer->draw_sprite(&sprite, transform.position.xy());
} // Allows things to happen while the game is paused
