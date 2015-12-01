#pragma once

#include "SDL.h"
#include "types.h"
#include "utility.h"

enum ScreenMode
{
    ScreenMode_Windowed   = 0,
    ScreenMode_FullScreen = SDL_WINDOW_FULLSCREEN,
    ScreenMode_Borderless = SDL_WINDOW_FULLSCREEN_DESKTOP
};

struct Window
{
	SDL_Window* sdl_window;
	SDL_GLContext sdl_gl_context;

    ScreenMode current_mode;
    Dimension resolution;
};

struct GameTime
{
	double current_time;
	double delta_time;

	uint32 last_frame_ticks;
	uint32 current_frame_ticks;
	uint32 ticks_per_frame;

	uint32 frame_count;
};

struct Timer
{
	float start_time;
	float duration;
};

/****************************
 *
 * Window functions
 *
 ******/

void StartupWindow(Window* window, char* title, int32 width, int32 height);

void ShutdownWindow(Window* window);

void WindowSetResolution(Window* window, uint32 w, uint32 h);

void WindowSetScreenMode(Window* window, ScreenMode mode);


/****************************
 *
 * Game State
 *
 ******/

struct GameState
{
    // TODO: INITIALIZE THIS
    MemoryArena temporary_memory;
    // TODO: THE ABOVE TODO

    Window window;
    GameTime time;
};

