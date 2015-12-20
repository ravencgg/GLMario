#include "input.h"

static Input s_input;

static void UpdateKeys()
{
	for(int i = 0; i < NUM_KEYS; ++i)
	{
		if(s_input.key_states[i] == KeyState::FRAME_UP || s_input.key_states[i] == KeyState::DOWN_UP)
		{
			s_input.key_states[i] = KeyState::UP;
		}
		else if(s_input.key_states[i] == KeyState::FRAME_DOWN || s_input.key_states[i] == KeyState::UP_DOWN)
		{
			s_input.key_states[i] = KeyState::DOWN;
		}
	}
}

static void UpdateMouse()
{
	if(s_input.mouse.buttons[(int32)MouseButton::LEFT] == KeyState::FRAME_UP) s_input.mouse.buttons[(int32)MouseButton::LEFT] = KeyState::UP;
	else if(s_input.mouse.buttons[(int32)MouseButton::LEFT] == KeyState::FRAME_DOWN) s_input.mouse.buttons[(int32)MouseButton::LEFT] = KeyState::DOWN;

	if(s_input.mouse.buttons[(int32)MouseButton::MIDDLE] == KeyState::FRAME_UP) s_input.mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::UP;
	else if(s_input.mouse.buttons[(int32)MouseButton::MIDDLE] == KeyState::FRAME_DOWN) s_input.mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::DOWN;

	if(s_input.mouse.buttons[(int32)MouseButton::RIGHT] == KeyState::FRAME_UP) s_input.mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::UP;
	else if(s_input.mouse.buttons[(int32)MouseButton::RIGHT] == KeyState::FRAME_DOWN) s_input.mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::DOWN;
}


void InitializeInput()
{
	memset(&s_input.key_states[0], 0, NUM_KEYS * sizeof(s_input.key_states[0]));
	s_input.mouse = {};
}


void InputBeginFrame()
{
    s_input.mouse_loc_updated_this_frame = false;

    UpdateKeys();
    UpdateMouse();
}

void ProcessKeyPress(int32 key)
{
	if(s_input.key_states[key] == KeyState::FRAME_UP)
		s_input.key_states[key] = KeyState::UP_DOWN;

	else if (s_input.key_states[key] == KeyState::DOWN_UP)
		s_input.key_states[key] = KeyState::UP_DOWN;

	else if (s_input.key_states[key] != KeyState::DOWN)
		s_input.key_states[key] = KeyState::FRAME_DOWN;
}

void ProcessKeyRelease(int32 key)
{
	if(s_input.key_states[key] == KeyState::FRAME_DOWN)
		s_input.key_states[key] = KeyState::DOWN_UP;

	else if(s_input.key_states[key] == KeyState::UP_DOWN)
		s_input.key_states[key] = KeyState::DOWN_UP;

	else if(s_input.key_states[key] != KeyState::UP)
		s_input.key_states[key] = KeyState::FRAME_UP;
}

// TODO: switch these to returning a key_state enum
// Down
// Frame_down
// Down_not_this_frame
// etc

bool KeyFrameDown(int32 key)
{
	bool result = (s_input.key_states[key] == KeyState::FRAME_DOWN
				  || s_input.key_states[key] == KeyState::DOWN_UP
				  || s_input.key_states[key] == KeyState::UP_DOWN);

	return result;
}

bool KeyFrameUp(int32 key)
{
	bool result = (s_input.key_states[key] == KeyState::FRAME_UP
				  || s_input.key_states[key] == KeyState::DOWN_UP
				  || s_input.key_states[key] == KeyState::UP_DOWN);

	return result;
}

bool KeyIsDown(int32 key)
{
	bool result = (s_input.key_states[key] == KeyState::FRAME_DOWN
				  || s_input.key_states[key] == KeyState::DOWN
				  || s_input.key_states[key] == KeyState::UP_DOWN
				  || s_input.key_states[key] == KeyState::DOWN_UP);

	return result;
}

bool KeyIsUp(int32 key)
{
	bool result = (s_input.key_states[key] == KeyState::FRAME_UP
				  || s_input.key_states[key] == KeyState::UP
				  || s_input.key_states[key] == KeyState::DOWN_UP
				  || s_input.key_states[key] == KeyState::UP_DOWN);

	return result;
}

void UpdateMousePosition()
{
	Point2 new_position;
	SDL_GetMouseState(&new_position.x, &new_position.y);

	if(!s_input.mouse_loc_updated_this_frame)
	{
		s_input.mouse_loc_updated_this_frame = true;
		s_input.mouse.delta.x = 0;
		s_input.mouse.delta.y = 0;
	}

	s_input.mouse.delta.x += new_position.x - s_input.mouse.p.x;
	s_input.mouse.delta.y += new_position.y - s_input.mouse.p.y;
	s_input.mouse.p = new_position;
}

void UpdateMouseWorldPosition(Dimension screen_resolution, Vec2 viewport_size, Vec2 camera_pos)
{
	int x, y;
	uint32 m = SDL_GetMouseState(&x, &y);

    Vec2 relative = vec2( (float) x / (float)screen_resolution.width , 1.f - (float) y / (float)screen_resolution.height);

    relative.x -= 0.5f;
    relative.y -= 0.5f;
    relative.x *= viewport_size.x;
    relative.y *= viewport_size.y;
    relative += camera_pos;

	s_input.mouse.world_position = relative;
}

void MouseButtonEvent()
{
	int x, y;
	uint32 m = SDL_GetMouseState(&x, &y);

	if (m & SDL_BUTTON(SDL_BUTTON_LEFT)) // Button down happened
	{
		if (s_input.mouse.buttons[(int32)MouseButton::LEFT] != KeyState::DOWN)
		{
			s_input.mouse.buttons[(int32)MouseButton::LEFT] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (s_input.mouse.buttons[(int32)MouseButton::LEFT] != KeyState::UP)
		{
			s_input.mouse.buttons[(int32)MouseButton::LEFT] = KeyState::FRAME_UP;
		}
	}

	if (m & SDL_BUTTON(SDL_BUTTON_MIDDLE)) // Button down happened
	{
		if (s_input.mouse.buttons[(int32)MouseButton::MIDDLE] != KeyState::DOWN)
		{
			s_input.mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (s_input.mouse.buttons[(int32)MouseButton::MIDDLE] != KeyState::UP)
		{
			s_input.mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::FRAME_UP;
		}
	}

	if (m & SDL_BUTTON(SDL_BUTTON_RIGHT)) // Button down happened
	{
		if (s_input.mouse.buttons[(int32)MouseButton::RIGHT] != KeyState::DOWN)
		{
			s_input.mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (s_input.mouse.buttons[(int32)MouseButton::RIGHT] != KeyState::UP)
		{
			s_input.mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::FRAME_UP;
		}
	}
}

Point2 MouseScreenPoint()
{
	return s_input.mouse.p;
}

Point2 MouseFrameDelta()
{
	return s_input.mouse.delta;
}

Vec2 MouseWorldPosition()
{
	return s_input.mouse.world_position;
}

bool MouseFrameDown(MouseButton button)
{
	bool result = (s_input.mouse.buttons[(int32)button] == KeyState::FRAME_DOWN);
	return result;
}

bool MouseFrameUp(MouseButton button)
{
	bool result = s_input.mouse.buttons[(int32)button] == KeyState::FRAME_UP;
	return result;
}

bool MouseIsDown(MouseButton button)
{
	bool result = (s_input.mouse.buttons[(int32)button] == KeyState::FRAME_DOWN || s_input.mouse.buttons[(int32)button] == KeyState::DOWN);
	return result;
}

bool MouseIsUp(MouseButton button)
{
	bool result = (s_input.mouse.buttons[(int32)button] == KeyState::FRAME_UP || s_input.mouse.buttons[(int32)button] == KeyState::UP);
	return result;
}

