#pragma once

#include "SDL.h"
#include "types.h"
#include "utility.h"
#include "input.h"

#include "entity.h"

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

void InitializeWindow(Window* window, char* title, int32 width, int32 height);

void ShutdownWindow(Window* window);

void WindowSetResolution(Window* window, uint32 w, uint32 h);

void WindowSetScreenMode(Window* window, ScreenMode mode);

/****************************
 *
 * Scene State
 *
 ******/


struct Scene
{
    SceneType type;

    uint32 max_entities;
    uint32 next_entity_id;
    int32  first_entity_index;
    GameEntity* entities;

    int32 first_object_index;
    uint32 max_objects;
    uint32 next_object_id;
    GameObject* objects;
};

struct Camera
{
    Transform transform;
    Vec2 viewport_size;
};


/****************************
 *
 * Game State
 *
 ******/

struct GameState
{
    MemoryArena temporary_memory;
    MemoryArena permanent_memory;

    Scene* active_scene;

    Window window;
    GameTime time;
    Input input;
};

