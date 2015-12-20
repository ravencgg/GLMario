#pragma once

#include "SDL.h"

#include "types.h"
#include "mathops.h"


#define NUM_KEYS 256

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
void UpdateMouseWorldPosition(Dimension screen_resolution, Vec2 viewport_size, Vec2 camera_pos);
void UpdateMouseWorldPosition();
void MouseButtonEvent();

Point2 MouseFrameDelta();
Point2 MouseScreenPoint();
Vec2 MouseWorldPosition();

bool MouseFrameDown(MouseButton);
bool MouseFrameUp(MouseButton);
bool MouseIsDown(MouseButton);
bool MouseIsUp(MouseButton);

