#pragma once

#include "SDL.h"
#include "entity.h"
#include "tilemap.h"

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
//    SceneType type;
    uint32 max_entities;
    uint32 active_entities;
    uint32 next_entity_id;
    uint32 player_id;
    EntityVtable entity_vtable[EntityType_Count];
    Entity*  entities;
    Entity** entity_delete_list;
    uint32   entity_delete_count;

// The physics one
    Tilemap* tmap;
    Physics* physics;

// TODO: combine tilemap code, or just pick one
// The grid one
    TileMap tilemap;
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
};

