#pragma once

#include "SDL.h"

#include "types.h"
#include "mathops.h"

#define OLD_INPUT

#ifdef OLD_INPUT
#define NUM_KEYS SDL_NUM_SCANCODES

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

// TODO: Verify all these codes are what we are expecting
//  KeyCode                   Character         Hex         Decimal
    KeyCode_BACKSPACE       = '\b',             // 0x08     // 8
    KeyCode_TAB             = '\t',             // 0x09     // 9
    KeyCode_ENTER           = '\r',             // 0x0D     // 13
    KeyCode_ESCAPE          = '\033',           // 0x1B     // 27
    KeyCode_SPACE           = ' ',              // 0x20     // 32
    KeyCode_EXCLAIM         = '!',              // 0x21     // 33
    KeyCode_QUOTEDBL        = '"',              // 0x22     // 34
    KeyCode_HASH            = '#',              // 0x23     // 35
    KeyCode_DOLLAR          = '$',              // 0x24     // 36
    KeyCode_PERCENT         = '%',              // 0x25     // 37
    KeyCode_AMPERSAND       = '&',              // 0x26     // 38
    KeyCode_QUOTE           = '\'',             // 0x27     // 39
    KeyCode_LEFTPAREN       = '(',              // 0x28     // 40
    KeyCode_RIGHTPAREN      = ')',              // 0x29     // 41
    KeyCode_ASTERISK        = '*',              // 0x2A     // 42
    KeyCode_PLUS            = '+',              // 0x2B     // 43
    KeyCode_COMMA           = ',',              // 0x2C     // 44
    KeyCode_MINUS           = '-',              // 0x2D     // 45
    KeyCode_PERIOD          = '.',              // 0x2E     // 46
    KeyCode_SLASH           = '/',              // 0x2F     // 47
    KeyCode_0               = '0',              // 0x30     // 48
    KeyCode_1               = '1',              // 0x31     // 49
    KeyCode_2               = '2',              // 0x32     // 50
    KeyCode_3               = '3',              // 0x33     // 51
    KeyCode_4               = '4',              // 0x34     // 52
    KeyCode_5               = '5',              // 0x35     // 53
    KeyCode_6               = '6',              // 0x36     // 54
    KeyCode_7               = '7',              // 0x37     // 55
    KeyCode_8               = '8',              // 0x38     // 56
    KeyCode_9               = '9',              // 0x39     // 57
    KeyCode_COLON           = ':',              // 0x3A     // 58
    KeyCode_SEMICOLON       = ';',              // 0x3B     // 59
    KeyCode_LESS            = '<',              // 0x3C     // 60
    KeyCode_EQUALS          = '=',              // 0x3D     // 61
    KeyCode_GREATER         = '>',              // 0x3E     // 62
    KeyCode_QUESTION        = '?',              // 0x3F     // 63
    KeyCode_AT              = '@',              // 0x40     // 64

// Uppercase

    KeyCode_LEFTBRACKET     = '[',              // 0x5B     // 91
    KeyCode_BACKSLASH       = '\\',             // 0x5C     // 92
    KeyCode_RIGHTBRACKET    = ']',              // 0x5D     // 93
    KeyCode_CARET           = '^',              // 0x5E     // 94
    KeyCode_UNDERSCORE      = '_',              // 0x5F     // 95
    KeyCode_BACKQUOTE       = '`',              // 0x60     // 96
    KeyCode_a               = 'a',              // 0x61     // 97
    KeyCode_b               = 'b',              // 0x62     // 98
    KeyCode_c               = 'c',              // 0x63     // 99
    KeyCode_d               = 'd',              // 0x64     // 100
    KeyCode_e               = 'e',              // 0x65     // 101
    KeyCode_f               = 'f',              // 0x66     // 102
    KeyCode_g               = 'g',              // 0x67     // 103
    KeyCode_h               = 'h',              // 0x68     // 104
    KeyCode_i               = 'i',              // 0x69     // 105
    KeyCode_j               = 'j',              // 0x6A     // 106
    KeyCode_k               = 'k',              // 0x6B     // 107
    KeyCode_l               = 'l',              // 0x6C     // 108
    KeyCode_m               = 'm',              // 0x6D     // 109
    KeyCode_n               = 'n',              // 0x6E     // 110
    KeyCode_o               = 'o',              // 0x6F     // 111
    KeyCode_p               = 'p',              // 0x70     // 112
    KeyCode_q               = 'q',              // 0x71     // 113
    KeyCode_r               = 'r',              // 0x72     // 114
    KeyCode_s               = 's',              // 0x73     // 115
    KeyCode_t               = 't',              // 0x74     // 116
    KeyCode_u               = 'u',              // 0x75     // 117
    KeyCode_v               = 'v',              // 0x76     // 118
    KeyCode_w               = 'w',              // 0x77     // 119
    KeyCode_x               = 'x',              // 0x78     // 120
    KeyCode_y               = 'y',              // 0x79     // 121
    KeyCode_z               = 'z',              // 0x7A     // 122

    KeyCode_F1              = 0x80,             // 0x80     // 123
    KeyCode_F2              = 0x81,             // 0x81     // 124
    KeyCode_F3              = 0x82,             // 0x82     // 125
    KeyCode_F4              = 0x83,             // 0x83     // 126
    KeyCode_F5              = 0x84,             // 0x84     // 127
    KeyCode_F6              = 0x85,             // 0x85     // 128
    KeyCode_F7              = 0x86,             // 0x86     // 129
    KeyCode_F8              = 0x87,             // 0x87     // 130
    KeyCode_F9              = 0x88,             // 0x88     // 131
    KeyCode_F10             = 0x89,             // 0x89     // 132
    KeyCode_F11             = 0x90,             // 0x8A     // 133
    KeyCode_F12             = 0x91,             // 0x8A     // 134

    KeyCode_MaxKeyCodes
};

#define NUM_KEYS KeyCode_MaxKeyCodes

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

struct Input
{
    KeyState key_states[KeyCode_MaxKeyCodes];
    MouseState mouse_state;
};
#endif
