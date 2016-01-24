#include "input.h"

//#define OLD_INPUT

#ifdef OLD_INPUT
static Input s_input;

static void UpdateKeys()
{
}

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
    if (s_input.mouse.buttons[(int32)MouseButton::LEFT] == KeyState::FRAME_UP) 
    {
        s_input.mouse.buttons[(int32)MouseButton::LEFT] = KeyState::UP;
    } 
    else if (s_input.mouse.buttons[(int32)MouseButton::LEFT] == KeyState::FRAME_DOWN) 
    {
        s_input.mouse.buttons[(int32)MouseButton::LEFT] = KeyState::DOWN;
    }

    if (s_input.mouse.buttons[(int32)MouseButton::MIDDLE] == KeyState::FRAME_UP) 
    {
        s_input.mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::UP;
    } 
    else if (s_input.mouse.buttons[(int32)MouseButton::MIDDLE] == KeyState::FRAME_DOWN) 
    {
        s_input.mouse.buttons[(int32)MouseButton::MIDDLE] = KeyState::DOWN;
    }

    if (s_input.mouse.buttons[(int32)MouseButton::RIGHT] == KeyState::FRAME_UP) 
    {
        s_input.mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::UP;
    } 
    else if (s_input.mouse.buttons[(int32)MouseButton::RIGHT] == KeyState::FRAME_DOWN) 
    {
        s_input.mouse.buttons[(int32)MouseButton::RIGHT] = KeyState::DOWN;
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


#else

void Input::BeginMessageLoop ()
{
    for (uint32 i = 0; i < KeyCode_Count; ++i)
    {
        m_key_states[i].started_down = m_key_states[i].ended_down;
        m_key_states[i].half_presses = 0;
    }
    for (uint32 i = 0; i < (uint32) MouseButton::COUNT; ++i)
    {
        m_key_states[i].started_down = m_key_states[i].ended_down;
        m_mouse_state.buttons[i].half_presses = 0;
    }

    m_mouse_state.last_position = m_mouse_state.new_position;
}

inline MouseButton GetMouseButton(uint8 button)
{
#if 0
    switch (button)
    {
    case SDL_BUTTON_LEFT:
        return MouseButton::LEFT;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::MIDDLE;
    case SDL_BUTTON_RIGHT:
        return MouseButton::RIGHT;
    default:
        return MouseButton::UNKNOWN;
        assert(!"Unhandled mouse button");
    }
#endif
}

#if 0
inline uint32 GetIndexFromMask(SDL_Keycode code)
{
    assert((code & (~SDLK_SCANCODE_MASK)) < SDL_NUM_SCANCODES);
    int32 result = code & 0x2FF; // Strip off everything above
    return result;
}
#endif

Vec2 MouseWorldPosition()
{
    return { 0, -1 };

}

void UpdateMouseWorldPosition(Vec2i, Vec2, Vec2)
{
}

void Input::HandleInputMessage(const InputEvent& event)
{
    switch (event.input_type)
    {
    case INPUTTYPE_Keyboard:
    {
        switch (event.key_press.key_press_flags)
        {
        case INPUTFLAG_Press:
        {
//            uint32 index = GetIndexFromMask(event.key_press.key_code);
            m_key_states[event.key_press.key_code].ended_down = true;
            m_key_states[event.key_press.key_code].half_presses += 1;
        }break;
        case INPUTFLAG_Release:
        {
            m_key_states[event.key_press.key_code].ended_down = false;
            m_key_states[event.key_press.key_code].half_presses += 1;
        }break;
        }

    }break;
    case INPUTTYPE_MouseButton:
    {
    }break;
    case INPUTTYPE_MouseMotion:
    {
    }break;

    InvalidDefaultCase;
    }

#if 0
    switch (event.type)
    {
    case SDL_KEYDOWN:
    {
        uint32 index = GetIndexFromMask(event.key.keysym.sym);
        m_key_states[index].ended_down = true;
        m_key_states[index].half_presses += 1;

    }break;
    case SDL_KEYUP:
    {
        uint32 index = GetIndexFromMask(event.key.keysym.sym);
        m_key_states[index].ended_down = false;
        m_key_states[index].half_presses += 1;

    }break;
    case SDL_MOUSEBUTTONDOWN:
    {
        MouseButton mouse_button = GetMouseButton(event.button.button);
        if (mouse_button < MouseButton::COUNT)
        {
            m_mouse_state.buttons[(uint32)mouse_button].ended_down = true;
            m_mouse_state.buttons[(uint32)mouse_button].half_presses += 1;
        }
        m_mouse_state.new_position.x = event.button.x;
        m_mouse_state.new_position.y = event.button.y;

    }break;
    case SDL_MOUSEBUTTONUP:
    {
        MouseButton mouse_button = GetMouseButton(event.button.button);
        if (mouse_button < MouseButton::COUNT)
        {
            m_mouse_state.buttons[(uint32)mouse_button].ended_down = false;
            m_mouse_state.buttons[(uint32)mouse_button].half_presses -= 1;
        }
        m_mouse_state.new_position.x = event.button.x;
        m_mouse_state.new_position.y = event.button.y;

    }break;
    case SDL_MOUSEMOTION:
    {
        m_mouse_state.new_position.x = event.motion.x;
        m_mouse_state.new_position.y = event.motion.y;
    }break;

    default:
        assert(!"Unhandled input event!");
    }
#endif
}

bool Input::IsDown(KeyCode key_code) const
{
//    uint32 index = GetIndexFromMask(key_code);
    bool result = m_key_states[key_code].IsDown();
    return result;
}

bool Input::OnDown(KeyCode key_code) const
{
 //   uint32 index = GetIndexFromMask(key_code);
    bool result = m_key_states[key_code].OnDown();
    return result;
}

bool Input::IsUp(KeyCode key_code) const
{
//    uint32 index = GetIndexFromMask(key_code);
    bool result = m_key_states[key_code].IsUp();
    return result;
}

bool Input::OnUp(KeyCode key_code) const
{
    //uint32 index = GetIndexFromMask(key_code);
    bool result = m_key_states[key_code].OnUp();
    return result;
}

bool Input::IsDown(MouseButton mouse_button) const
{
    bool result = false;
    uint32 index = static_cast<uint32>(mouse_button);
    if (index < static_cast<uint32>(MouseButton::COUNT))
    {
        result = m_mouse_state.buttons[index].IsDown();
    }

    return result;
}

bool Input::OnDown(MouseButton mouse_button) const
{
    bool result = false;
    uint32 index = static_cast<uint32>(mouse_button);
    if (index < static_cast<uint32>(MouseButton::COUNT))
    {
        result = m_mouse_state.buttons[index].OnDown();
    }

    return result;
}

bool Input::IsUp(MouseButton mouse_button) const
{
    bool result = false;
    uint32 index = static_cast<uint32>(mouse_button);
    if (index < static_cast<uint32>(MouseButton::COUNT))
    {
        result = m_mouse_state.buttons[index].IsUp();
    }

    return result;
}

bool Input::OnUp(MouseButton mouse_button) const
{
    bool result = false;
    uint32 index = static_cast<uint32>(mouse_button);
    if (index < static_cast<uint32>(MouseButton::COUNT))
    {
        result = m_mouse_state.buttons[index].OnUp();
    }

    return result;
}

#endif
