#include "input.h"

//#define OLD_INPUT

#ifdef OLD_INPUT
static Input s_input;


void InitializeInput()
{
	memset(&s_input.key_states[0], 0, NUM_KEYS * sizeof(s_input.key_states[0]));
	s_input.mouse = {};
}

inline uint32 GetIndexFromMask(SDL_Keycode code)
{
    assert((code & (~SDLK_SCANCODE_MASK)) < SDL_NUM_SCANCODES);
    int32 result = code & 0x2FF; // Strip off everything above
    return result;
}

void InputBeginFrame()
{
    // Update Keys
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

    // Mouse
    if (s_input.mouse.buttons[(int32)MouseButtons::LEFT] == KeyState::FRAME_UP)
    {
        s_input.mouse.buttons[(int32)MouseButtons::LEFT] = KeyState::UP;
    }
    else if (s_input.mouse.buttons[(int32)MouseButtons::LEFT] == KeyState::FRAME_DOWN)
    {
        s_input.mouse.buttons[(int32)MouseButtons::LEFT] = KeyState::DOWN;
    }

    if (s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] == KeyState::FRAME_UP)
    {
        s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] = KeyState::UP;
    }
    else if (s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] == KeyState::FRAME_DOWN)
    {
        s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] = KeyState::DOWN;
    }

    if (s_input.mouse.buttons[(int32)MouseButtons::RIGHT] == KeyState::FRAME_UP)
    {
        s_input.mouse.buttons[(int32)MouseButtons::RIGHT] = KeyState::UP;
    }
    else if (s_input.mouse.buttons[(int32)MouseButtons::RIGHT] == KeyState::FRAME_DOWN)
    {
        s_input.mouse.buttons[(int32)MouseButtons::RIGHT] = KeyState::DOWN;
    }

    s_input.mouse.delta.x = 0;
    s_input.mouse.delta.y = 0;
}

void ProcessKeyPress(int32 key)
{
    key = GetIndexFromMask(key);
	if(s_input.key_states[key] == KeyState::FRAME_UP)
		s_input.key_states[key] = KeyState::UP_DOWN;

	else if (s_input.key_states[key] == KeyState::DOWN_UP)
		s_input.key_states[key] = KeyState::UP_DOWN;

	else if (s_input.key_states[key] != KeyState::DOWN)
		s_input.key_states[key] = KeyState::FRAME_DOWN;
}

void ProcessKeyRelease(int32 key)
{
    key = GetIndexFromMask(key);
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
    key = GetIndexFromMask(key);
	bool result = (s_input.key_states[key] == KeyState::FRAME_DOWN
				  || s_input.key_states[key] == KeyState::DOWN_UP
				  || s_input.key_states[key] == KeyState::UP_DOWN);

	return result;
}

bool KeyFrameUp(int32 key)
{
    key = GetIndexFromMask(key);
	bool result = (s_input.key_states[key] == KeyState::FRAME_UP
				  || s_input.key_states[key] == KeyState::DOWN_UP
				  || s_input.key_states[key] == KeyState::UP_DOWN);

	return result;
}

bool KeyIsDown(int32 key)
{
    key = GetIndexFromMask(key);
	bool result = (s_input.key_states[key] == KeyState::FRAME_DOWN
				  || s_input.key_states[key] == KeyState::DOWN
				  || s_input.key_states[key] == KeyState::UP_DOWN
				  || s_input.key_states[key] == KeyState::DOWN_UP);

	return result;
}

bool KeyIsUp(int32 key)
{
    key = GetIndexFromMask(key);
	bool result = (s_input.key_states[key] == KeyState::FRAME_UP
				  || s_input.key_states[key] == KeyState::UP
				  || s_input.key_states[key] == KeyState::DOWN_UP
				  || s_input.key_states[key] == KeyState::UP_DOWN);

	return result;
}

void UpdateMousePosition()
{
    Vec2i new_position = {};
	SDL_GetMouseState(&new_position.x, &new_position.y);

	s_input.mouse.delta.x += new_position.x - s_input.mouse.p.x;
	s_input.mouse.delta.y += new_position.y - s_input.mouse.p.y;
	s_input.mouse.p = new_position;
}

void UpdateMouseWorldPosition(Vec2i screen_resolution, Vec2 viewport_size, Vec2 camera_pos)
{
    int x, y;
	SDL_GetMouseState(&x, &y);

    Vec2 relative = vec2( (float) x / (float)screen_resolution.x, 1.f - (float) y / (float)screen_resolution.y);

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
		if (s_input.mouse.buttons[(int32)MouseButtons::LEFT] != KeyState::DOWN)
		{
			s_input.mouse.buttons[(int32)MouseButtons::LEFT] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (s_input.mouse.buttons[(int32)MouseButtons::LEFT] != KeyState::UP)
		{
			s_input.mouse.buttons[(int32)MouseButtons::LEFT] = KeyState::FRAME_UP;
		}
	}

	if (m & SDL_BUTTON(SDL_BUTTON_MIDDLE)) // Button down happened
	{
		if (s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] != KeyState::DOWN)
		{
			s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] != KeyState::UP)
		{
			s_input.mouse.buttons[(int32)MouseButtons::MIDDLE] = KeyState::FRAME_UP;
		}
	}

	if (m & SDL_BUTTON(SDL_BUTTON_RIGHT)) // Button down happened
	{
		if (s_input.mouse.buttons[(int32)MouseButtons::RIGHT] != KeyState::DOWN)
		{
			s_input.mouse.buttons[(int32)MouseButtons::RIGHT] = KeyState::FRAME_DOWN;
		}
	}
	else // Button up happened
	{
		if (s_input.mouse.buttons[(int32)MouseButtons::RIGHT] != KeyState::UP)
		{
			s_input.mouse.buttons[(int32)MouseButtons::RIGHT] = KeyState::FRAME_UP;
		}
	}
}

Vec2i MouseScreenPoint()
{
	return s_input.mouse.p;
}

Vec2i MouseFrameDelta()
{
	return s_input.mouse.delta;
}

Vec2 MouseWorldPosition()
{
	return s_input.mouse.world_position;
}

bool MouseFrameDown(MouseButtons button)
{
	bool result = (s_input.mouse.buttons[(int32)button] == KeyState::FRAME_DOWN);
	return result;
}

bool MouseFrameUp(MouseButtons button)
{
	bool result = s_input.mouse.buttons[(int32)button] == KeyState::FRAME_UP;
	return result;
}

bool MouseIsDown(MouseButtons button)
{
	bool result = (s_input.mouse.buttons[(int32)button] == KeyState::FRAME_DOWN || s_input.mouse.buttons[(int32)button] == KeyState::DOWN);
	return result;
}

bool MouseIsUp(MouseButton button)
{
	bool result = (s_input.mouse.buttons[(int32)button] == KeyState::FRAME_UP || s_input.mouse.buttons[(int32)button] == KeyState::UP);
	return result;
}

#endif
#if defined(NEW_INPUT)

void BeginMessageLoop(NewInput* input)
{
    for (uint32 i = 0; i < KeyCode_MaxKeyCodes; ++i)
    {
        input->key_states[i].started_down = input->key_states[i].ended_down;
        input->key_states[i].half_presses = 0;
    }
    for (uint32 i = 0; i < MouseButton_COUNT; ++i)
    {
        input->key_states[i].started_down = input->key_states[i].ended_down;
        input->mouse_state.buttons[i].half_presses = 0;
    }

    input->mouse_state.last_position = input->mouse_state.new_position;
}

void _ProcessKeyboardMessage(NewInput* input, KeyCode key, bool pressed)
{
    if(pressed)
    {
        input->key_states[key].ended_down = true;
        input->key_states[key].half_presses += 1;
    }
    else
    {
        input->key_states[key].ended_down = false;
        input->key_states[key].half_presses += 1;
    }
}

void _ProcessMouseButtonMessage(NewInput*, MouseButton, bool)
{
    assert(0);
}

Vec2i MousePosition(NewInput* input)
{
    Vec2i result = input->mouse_state.new_position;
    return result;
}

Vec2i MouseDelta(NewInput* input)
{
    Vec2i result = input->mouse_state.last_position - input->mouse_state.new_position;
    return result;
}

bool IsDown(NewInput* input, KeyCode key_code)
{
    bool result = false;
    if(key_code < KeyCode_MaxKeyCodes)
    {
        result = input->key_states[key_code].IsDown();
    }

    return result;
}

bool OnDown(NewInput* input, KeyCode key_code)
{
    bool result = false;
    if(key_code < KeyCode_MaxKeyCodes)
    {
        result = input->key_states[key_code].OnDown();
    }

    return result;
}

bool IsUp(NewInput* input, KeyCode key_code)
{
    bool result = false;
    if(key_code < KeyCode_MaxKeyCodes)
    {
        result = input->key_states[key_code].IsUp();
    }

    return result;
}

bool OnUp(NewInput* input, KeyCode key_code)
{
    bool result = false;
    if(key_code < KeyCode_MaxKeyCodes)
    {
        result = input->key_states[key_code].OnUp();
    }

    return result;
}

bool IsDown(NewInput* input, MouseButton mouse_button)
{
    bool result = false;
    if (mouse_button < MouseButton_COUNT)
    {
        result = input->mouse_state.buttons[mouse_button].IsDown();
    }

    return result;
}

bool OnDown(NewInput* input, MouseButton mouse_button)
{
    bool result = false;
    if (mouse_button < MouseButton_COUNT)
    {
        result = input->mouse_state.buttons[mouse_button].OnDown();
    }

    return result;
}

bool IsUp(NewInput* input, MouseButton mouse_button)
{
    bool result = false;
    if (mouse_button < MouseButton_COUNT)
    {
        result = input->mouse_state.buttons[mouse_button].IsUp();
    }

    return result;
}

bool OnUp(NewInput* input, MouseButton mouse_button)
{
    bool result = false;
    if (mouse_button < MouseButton_COUNT)
    {
        result = input->mouse_state.buttons[mouse_button].OnUp();
    }

    return result;
}

#endif
