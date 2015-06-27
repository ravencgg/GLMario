#pragma once

#include "types.h"
#include <string.h> // memset
#include "SDL.h"
#define NUM_KEYS 256

enum class MouseButton { LEFT, MIDDLE, RIGHT, COUNT };
enum class KeyState { UP, FRAME_UP, DOWN, FRAME_DOWN, DOWN_UP, UP_DOWN };

struct Mouse
{
	Point2 p;
	Point2 delta;
	KeyState buttons[(int32)MouseButton::COUNT];
};

class Input
{
public:

	Input();
	~Input() {};
	static Input* get_instance();

	void begin_frame();

// Keyboard handling
	void process_key_press(int32 key);
	void process_key_release(int32 key);

	bool on_down(int32 key);
	bool on_up(int32 key);
	bool is_down(int32 key);
	bool is_up(int32 key);

// Mouse handling
	void update_mouse_position();
	void mouse_button_event();

	Point2 mouse_loc();
	Point2 mouse_delta();

	bool mouse_on_down(MouseButton);
	bool mouse_on_up(MouseButton);
	bool mouse_is_down(MouseButton);
	bool mouse_is_up(MouseButton); 

private:

	static Input* s_input;

	void update_keys();
	void update_mouse();

	KeyState key_states[NUM_KEYS];
	Mouse mouse;
	bool mouse_loc_updated_this_frame = false;
};