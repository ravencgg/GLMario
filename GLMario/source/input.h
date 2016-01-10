#pragma once

#include "SDL.h"

#include "types.h"
#include "mathops.h"
#include "platform.h"

#define NUM_KEYS SDL_NUM_SCANCODES

#define OLD_INPUT
#ifdef OLD_INPUT
enum class MouseButton { LEFT, MIDDLE, RIGHT, COUNT };
enum class KeyState { UP, FRAME_UP, DOWN, FRAME_DOWN, DOWN_UP, UP_DOWN };

struct Mouse
{
	Point2 p;
	Point2 delta;
    Vec2 world_position;
	KeyState buttons[(int32)MouseButton::COUNT];
};

struct Input
{
	KeyState key_states[NUM_KEYS];
	Mouse mouse;
	bool mouse_loc_updated_this_frame = false;
};

void InitializeInput();

void InputBeginFrame();

// Keyboard handling
void ProcessKeyPress(int32 key);
void ProcessKeyRelease(int32 key);

bool KeyFrameDown(int32 key);
bool KeyFrameUp(int32 key);
bool KeyIsDown(int32 key);
bool KeyIsUp(int32 key);

// Mouse handling
void UpdateMousePosition();
void UpdateMouseWorldPosition(Vec2i screen_resolution, Vec2 viewport_size, Vec2 camera_pos);
void UpdateMouseWorldPosition();
void MouseButtonEvent();

Point2 MouseFrameDelta();
Point2 MouseScreenPoint();
Vec2 MouseWorldPosition();

bool MouseFrameDown(MouseButton);
bool MouseFrameUp(MouseButton);
bool MouseIsDown(MouseButton);
bool MouseIsUp(MouseButton);


#else


enum class MouseButton
{
    LEFT,
    MIDDLE,
    RIGHT,
    COUNT,
    UNKNOWN = 0xFF
};

struct KeyState
{
    bool started_down = false;
    bool ended_down = false;
    uint16 half_presses = 0;

    bool IsDown() const { return ended_down || half_presses > 1; }
    bool OnDown() const { return !started_down && ended_down || half_presses > 1; }
    bool IsUp() const { return !ended_down; }
    bool OnUp() const { return (started_down && IsUp()); }
};

struct MouseState
{
    Vec2i last_position;
    Vec2i new_position;
    KeyState buttons[(uint32)MouseButton::COUNT];


    Vec2i ScreenPosition() { return new_position; }
    Vec2i Delta() { return new_position - last_position; }
};


Vec2 MouseWorldPosition();
void UpdateMouseWorldPosition(Vec2i, Vec2, Vec2);

class Input
{
public:
    Input();
    ~Input();

    void BeginMessageLoop();
    void HandleInputMessage(const InputEvent&);

    MouseState GetMouseState() { return m_mouse_state; }

    bool IsDown(KeyCode) const;
    bool OnDown(KeyCode) const;

    bool IsUp(KeyCode) const;
    bool OnUp(KeyCode) const;

    bool IsDown(MouseButton) const;
    bool OnDown(MouseButton) const;

    bool IsUp(MouseButton) const;
    bool OnUp(MouseButton) const;

private:

    // NOTE: These can be duplicates in rare circumstances? wiki.libsdl.org/SDLKeycodeLookup
    KeyState m_key_states[KeyCode_Count];
    MouseState m_mouse_state;
};
#endif
