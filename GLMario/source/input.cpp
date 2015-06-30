#include "input.h"


Input* Input::s_input = nullptr;

Input::Input()
{
	memset(&key_states[0], 0, NUM_KEYS * sizeof(key_states[0]));
	mouse = {};
}

Input* Input::get_instance()
{
	if(!s_input)
	{
		s_input = new Input();
	}
	return s_input;
}

void Input::begin_frame()
{
	mouse_loc_updated_this_frame = false;

	update_keys();
	update_mouse();
}

void Input::update_keys()
{
	for(int i = 0; i < NUM_KEYS; ++i)
	{
		if(key_states[i] == KeyState::FRAME_UP || key_states[i] == KeyState::DOWN_UP)
		{
			key_states[i] = KeyState::UP;
		}
		else if(key_states[i] == KeyState::FRAME_DOWN || key_states[i] == KeyState::UP_DOWN)
		{
			key_states[i] = KeyState::DOWN;
		}
	}
}

void Input::process_key_press(int32 key)
{
	if(key_states[key] == KeyState::FRAME_UP)
		key_states[key] = KeyState::UP_DOWN;

	else if (key_states[key] == KeyState::DOWN_UP)
		key_states[key] = KeyState::UP_DOWN;

	else if (key_states[key] != KeyState::DOWN)
		key_states[key] = KeyState::FRAME_DOWN;
}

void Input::process_key_release(int32 key)
{
	volatile int i = 0; 
	i++;
	if(key_states[key] == KeyState::FRAME_DOWN)
		key_states[key] = KeyState::DOWN_UP;

	else if(key_states[key] == KeyState::UP_DOWN)
		key_states[key] = KeyState::DOWN_UP;

	else if(key_states[key] != KeyState::UP)
		key_states[key] = KeyState::FRAME_UP; 
}

bool Input::on_down(int32 key)
{
	bool result = (key_states[key] == KeyState::FRAME_DOWN 
				  || key_states[key] == KeyState::DOWN_UP 
				  || key_states[key] == KeyState::UP_DOWN);

	return result;
}

bool Input::on_up(int32 key)
{
	bool result = (key_states[key] == KeyState::FRAME_UP
				  || key_states[key] == KeyState::DOWN_UP 
				  || key_states[key] == KeyState::UP_DOWN);

	return result;
}

bool Input::is_down(int32 key)
{
	bool result = (key_states[key] == KeyState::FRAME_DOWN 
				  || key_states[key] == KeyState::DOWN
				  || key_states[key] == KeyState::UP_DOWN
				  || key_states[key] == KeyState::DOWN_UP);

	return result;
}

bool Input::is_up(int32 key)
{
	bool result = (key_states[key] == KeyState::FRAME_UP 
				  || key_states[key] == KeyState::UP
				  || key_states[key] == KeyState::DOWN_UP
				  || key_states[key] == KeyState::UP_DOWN); 

	return result;
}

void Input::update_mouse()
{
	if(mouse.buttons[(int32)MouseButton::LEFT] == KeyState::FRAME_UP) mouse.buttons[(int32)MouseButton::LEFT] = KeyState::UP;
	else if(mouse.buttons[(int32)MouseButton::LEFT] == KeyState::FRAME_DOWN) mouse.buttons[(int32)MouseButton::LEFT] = KeyState::DOWN;

	if(mouse.buttons[(int32)MouseButton::MIDDLE] == KeyState::FRAME_UP) mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::UP;
	else if(mouse.buttons[(int32)MouseButton::MIDDLE] == KeyState::FRAME_DOWN) mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::DOWN;

	if(mouse.buttons[(int32)MouseButton::RIGHT] == KeyState::FRAME_UP) mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::UP;
	else if(mouse.buttons[(int32)MouseButton::RIGHT] == KeyState::FRAME_DOWN) mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::DOWN;
}

void Input::update_mouse_position()
{
	Point2 new_position;
	SDL_GetMouseState(&new_position.x, &new_position.y);

	if(!mouse_loc_updated_this_frame)
	{
		mouse_loc_updated_this_frame = true;
		mouse.delta.x = 0;
		mouse.delta.y = 0;
	}

	mouse.delta.x += new_position.x - mouse.p.x;	
	mouse.delta.y += new_position.y - mouse.p.y;
	mouse.p = new_position;
}

void Input::mouse_button_event()
{
	int x, y;
	uint32 m = SDL_GetMouseState(&x, &y);

	if (m & SDL_BUTTON(SDL_BUTTON_LEFT)) // Button down happened
	{
		if (mouse.buttons[(int32)MouseButton::LEFT] != KeyState::DOWN)
		{
			mouse.buttons[(int32)MouseButton::LEFT] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (mouse.buttons[(int32)MouseButton::LEFT] != KeyState::UP)
		{
			mouse.buttons[(int32)MouseButton::LEFT] = KeyState::FRAME_UP;
		}
	}

	if (m & SDL_BUTTON(SDL_BUTTON_MIDDLE)) // Button down happened
	{
		if (mouse.buttons[(int32)MouseButton::MIDDLE] != KeyState::DOWN)
		{
			mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (mouse.buttons[(int32)MouseButton::MIDDLE] != KeyState::UP)
		{
			mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::FRAME_UP;
		}
	}

	if (m & SDL_BUTTON(SDL_BUTTON_RIGHT)) // Button down happened
	{
		if (mouse.buttons[(int32)MouseButton::RIGHT] != KeyState::DOWN)
		{
			mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (mouse.buttons[(int32)MouseButton::RIGHT] != KeyState::UP)
		{
			mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::FRAME_UP;
		}
	}
}

Point2 Input::mouse_loc()
{
	return mouse.p;
}

Point2 Input::mouse_delta()
{
	return mouse.delta;
}

bool Input::mouse_on_down(MouseButton button)
{
	bool result = (mouse.buttons[(int32)button] == KeyState::FRAME_DOWN);
	return result;
}

bool Input::mouse_on_up(MouseButton button)
{
	bool result = mouse.buttons[(int32)button] == KeyState::FRAME_UP;
	return result;
}

bool Input::mouse_is_down(MouseButton button)
{
	bool result = (mouse.buttons[(int32)button] == KeyState::FRAME_DOWN || mouse.buttons[(int32)button] == KeyState::DOWN);
	return result;
}

bool Input::mouse_is_up(MouseButton button)
{
	bool result = (mouse.buttons[(int32)button] == KeyState::FRAME_UP || mouse.buttons[(int32)button] == KeyState::UP);
	return result;
}

/*

			//If user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
			{
				update_mouse_next_frame = true;

				uint32 m = SDL_GetMouseState(&game_state->mouse.x, &game_state->mouse.y);

				if (m & SDL_BUTTON(SDL_BUTTON_LEFT)) // Button down happened
				{
					if (game_state->mouse.left == game_state->mouse.UP || game_state->mouse.left == game_state->mouse.RELEASED)
					{
						game_state->mouse.left = game_state->mouse.PRESSED;
					}
				}
				else // Button up happened
				{
					if (game_state->mouse.left == game_state->mouse.PRESSED || game_state->mouse.left == game_state->mouse.HELD)
					{
						game_state->mouse.left = game_state->mouse.RELEASED;
					}
				}

				if (m & SDL_BUTTON(SDL_BUTTON_RIGHT)) // Button down happened
				{
					if (game_state->mouse.right == game_state->mouse.UP)
					{
						game_state->mouse.right = game_state->mouse.PRESSED;
					}
				}
				else // Button up happened
				{
					if (game_state->mouse.right == game_state->mouse.PRESSED || game_state->mouse.right == game_state->mouse.HELD)
					{
						game_state->mouse.right = game_state->mouse.RELEASED;
					}
				}
			}
			// TODO(chris): not super accurate mouse movement when moving UI panels.
			if (e.type == SDL_MOUSEMOTION)
			{
				mouse_moved = true;

				int last_x = game_state->mouse.x;
				int last_y = game_state->mouse.y;
				SDL_GetMouseState(&game_state->mouse.x, &game_state->mouse.y);
				game_state->mouse.delta_x = game_state->mouse.x - last_x;
				game_state->mouse.delta_y = game_state->mouse.y - last_y;
			}
		}
*/