#include "input.h"

void BeginMessageLoop(NewInput* input)
{
    for (uint32 i = 0; i < KeyCode_MaxKeyCodes; ++i)
    {
        input->key_states[i].started_down = input->key_states[i].ended_down;
        input->key_states[i].half_presses = 0;
    }
    for (uint32 i = 0; i < MouseButton_COUNT; ++i)
    {
        input->mouse_state.buttons[i].started_down = input->mouse_state.buttons[i].ended_down;
        input->mouse_state.buttons[i].half_presses = 0;
    }

    input->mouse_state.last_position = input->mouse_state.new_position;
}

void _ProcessKeyboardMessage(NewInput* input, KeyCode key, bool pressed)
{
    if (!key) return;
    assert(key > 0 && key < KeyCode_MaxKeyCodes);

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

void _ProcessMouseButtonMessage(NewInput* input, MouseButton button, bool pressed)
{
    assert(button > MouseButton_UNKNOWN && button < MouseButton_COUNT);

    if(pressed)
    {
        input->mouse_state.buttons[button].ended_down = true;
        input->mouse_state.buttons[button].half_presses += 1;
    }
    else
    {
        input->mouse_state.buttons[button].ended_down = false;
        input->mouse_state.buttons[button].half_presses += 1;
    }
}

void UpdateMouseWorldPosition(NewInput* input, Vec2i screen_resolution, Vec2 viewport_size, Vec2 camera_pos)
{
    Vec2 relative = vec2( (float) input->mouse_state.new_position.x / (float)screen_resolution.x,
                          (float) input->mouse_state.new_position.y / (float)screen_resolution.y);

    relative.x -= 0.5f;
    relative.y -= 0.5f;
    relative.x *= viewport_size.x;
    relative.y *= viewport_size.y;
    relative += camera_pos;

	input->mouse_state.world_position = relative;
}

Vec2i MousePosition(NewInput* input)
{
    Vec2i result = input->mouse_state.new_position;
    return result;
}

Vec2i MouseDelta(NewInput* input)
{
    Vec2i result = input->mouse_state.new_position - input->mouse_state.last_position;
    return result;
}

Vec2 MouseWorldPosition(NewInput* input)
{
    Vec2 result = input->mouse_state.world_position;
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
