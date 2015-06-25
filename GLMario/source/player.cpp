#include "player.h"

Player::Player(Input* input)
	: input(input)
{

	sprite.image_file = MAIN_IMAGE; 
	sprite.shader_type = DEFAULT_SHADER;
	sprite.layer = FOREGROUND;
	sprite.world_size = Vector2(1.0f, 1.0f);
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

	drawer->draw_sprite(&sprite, &transform);
}

void Player::paused_update_and_draw(IDrawer* drawer)
{
	drawer->draw_sprite(&sprite, &transform);
} // Allows things to happen while the game is paused
