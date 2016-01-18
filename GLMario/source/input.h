#pragma once

#include "SDL.h"

#include "types.h"
#include "mathops.h"

#define NUM_KEYS SDL_NUM_SCANCODES

#define OLD_INPUT

#ifdef OLD_INPUT
enum class MouseButton { LEFT, MIDDLE, RIGHT, COUNT };
enum class KeyState { UP, FRAME_UP, DOWN, FRAME_DOWN, DOWN_UP, UP_DOWN };

struct Mouse
{
	Vec2i p;
	Vec2i delta;
    Vec2 world_position;
	KeyState buttons[(int32)MouseButton::COUNT];
};

struct Input
{
	KeyState key_states[NUM_KEYS];
	Mouse mouse;
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

Vec2i MouseFrameDelta();
Vec2i MouseScreenPoint();
Vec2 MouseWorldPosition();

bool MouseFrameDown(MouseButton);
bool MouseFrameUp(MouseButton);
bool MouseIsDown(MouseButton);
bool MouseIsUp(MouseButton);

#else

enum KeyCode
{
    KeyCode_UNKNOWN = 0,

    KeyCode_RETURN = '\r',
    KeyCode_ESCAPE = '\033',
    KeyCode_BACKSPACE = '\b',
    KeyCode_TAB = '\t',
    KeyCode_SPACE = ' ',
    KeyCode_EXCLAIM = '!',
    KeyCode_QUOTEDBL = '"',
    KeyCode_HASH = '#',
    KeyCode_PERCENT = '%',
    KeyCode_DOLLAR = '$',
    KeyCode_AMPERSAND = '&',
    KeyCode_QUOTE = '\'',
    KeyCode_LEFTPAREN = '(',
    KeyCode_RIGHTPAREN = ')',
    KeyCode_ASTERISK = '*',
    KeyCode_PLUS = '+',
    KeyCode_COMMA = ',',
    KeyCode_MINUS = '-',
    KeyCode_PERIOD = '.',
    KeyCode_SLASH = '/',
    KeyCode_0 = '0',
    KeyCode_1 = '1',
    KeyCode_2 = '2',
    KeyCode_3 = '3',
    KeyCode_4 = '4',
    KeyCode_5 = '5',
    KeyCode_6 = '6',
    KeyCode_7 = '7',
    KeyCode_8 = '8',
    KeyCode_9 = '9',
    KeyCode_COLON = ':',
    KeyCode_SEMICOLON = ';',
    KeyCode_LESS = '<',
    KeyCode_EQUALS = '=',
    KeyCode_GREATER = '>',
    KeyCode_QUESTION = '?',
    KeyCode_AT = '@',

    KeyCode_LEFTBRACKET = '[',
    KeyCode_BACKSLASH = '\\',
    KeyCode_RIGHTBRACKET = ']',
    KeyCode_CARET = '^',
    KeyCode_UNDERSCORE = '_',
    KeyCode_BACKQUOTE = '`',
    KeyCode_a = 'a',
    KeyCode_b = 'b',
    KeyCode_c = 'c',
    KeyCode_d = 'd',
    KeyCode_e = 'e',
    KeyCode_f = 'f',
    KeyCode_g = 'g',
    KeyCode_h = 'h',
    KeyCode_i = 'i',
    KeyCode_j = 'j',
    KeyCode_k = 'k',
    KeyCode_l = 'l',
    KeyCode_m = 'm',
    KeyCode_n = 'n',
    KeyCode_o = 'o',
    KeyCode_p = 'p',
    KeyCode_q = 'q',
    KeyCode_r = 'r',
    KeyCode_s = 's',
    KeyCode_t = 't',
    KeyCode_u = 'u',
    KeyCode_v = 'v',
    KeyCode_w = 'w',
    KeyCode_x = 'x',
    KeyCode_y = 'y',
    KeyCode_z = 'z',

    KeyCode_MaxKeyCodes
};


enum KeyState
{
    KeyState_Up             = 0,
    KeyState_FrameUp        = 0x1,
    KeyState_Down           = 0x10,
    KeyState_FrameDown      = KeyState_Down | 0x20,
};


enum class MouseButton
{
    LEFT,
    MIDDLE,
    RIGHT,
    COUNT,
    UNKNOWN = 0xFF
};

#if 0
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
#endif

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
    KeyState key_states[KeyCode_MaxKeyCodes];
    MouseState mouse_state;
};
#endif
