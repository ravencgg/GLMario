#pragma once

#include "glew.h"
#include "SDL.h"

#include "types.h"
#include "mathops.h"


enum class ScreenMode { WINDOWED   = 0,
						FULLSCREEN = SDL_WINDOW_FULLSCREEN,
						BORDERLESS = SDL_WINDOW_FULLSCREEN_DESKTOP };

class Window
{
public:


	Window(char*, int32, int32);
	~Window();

	void swap_buffer();
	void set_window_mode(ScreenMode mode = ScreenMode::WINDOWED);
	Dimension get_resolution();
	void set_resolution(uint32, uint32);

	SDL_Window* sdl_window;
	SDL_GLContext sdl_gl_context;
};
