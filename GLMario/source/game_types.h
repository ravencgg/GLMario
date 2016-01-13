#pragma once

#include "SDL.h"
#include "entity.h"
#include "types.h"

struct TileMap;
struct Renderer;

enum ScreenMode
{
    ScreenMode_Windowed = 0,
    ScreenMode_FullScreen = 1, // SDL_WINDOW_FULLSCREEN,
    ScreenMode_Borderless = 2, //SDL_WINDOW_FULLSCREEN_DESKTOP
};

struct Window
{
	SDL_Window* sdl_window;
	SDL_GLContext sdl_gl_context;

    ScreenMode current_mode;
    Vec2i resolution;
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
 * Scene State
 *
 ******/


struct Scene
{
    uint32 max_entities;
    uint32 active_entities;
    EntityID player_id;
    Entity*  entities;
    Entity** entity_delete_list;
    uint32   entity_delete_count;

    TileMap* tilemap;
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
    Camera* active_camera;

    Renderer* renderer;
    Window window;
    GameTime time;
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
 * Time functions
 *
 ******/

double CurrentTimePrecise(GameState*);

float CurrentTime(GameState*);

float FrameTime(GameState*);

uint32 FrameCount(GameState*);

void InitializeTime(GameState*, uint32 ms_per_frame);

void TimeBeginFrame(GameState*);

uint32 RemainingTicksInFrame(GameState*);

uint32 RealTimeSinceStartup(GameState*);

/****************************
 *
 * Time functions
 *
 ******/

#define MAX_FRAME_TIME 0.25f

inline double CurrentTimePrecise(GameState* game_state)
{
    return game_state->time.current_time;
}

inline float CurrentTime(GameState* game_state)
{
    return (float)game_state->time.current_time;
}

inline float FrameTime(GameState* game_state)
{
    // TODO: This is really debug only
    float result = min((float)game_state->time.delta_time, MAX_FRAME_TIME);
    return result;
}

inline uint32 FrameCount(GameState* game_state)
{
    return game_state->time.frame_count;
}

inline void InitializeTime(GameState* game_state, uint32 ms_per_frame)
{
    // TODO: convert this to high precision
    memset(&game_state->time, 0, sizeof(game_state->time));
	game_state->time.ticks_per_frame = 16;
	game_state->time.current_frame_ticks = SDL_GetTicks();
}

inline void TimeBeginFrame(GameState* game_state)
{
	game_state->time.last_frame_ticks = game_state->time.current_frame_ticks;
	game_state->time.current_frame_ticks = SDL_GetTicks();

	game_state->time.current_time = (double)game_state->time.current_frame_ticks / 1000.0;
	game_state->time.delta_time = (double)(game_state->time.current_frame_ticks - game_state->time.last_frame_ticks) / 1000.0;

	++game_state->time.frame_count;
}

inline uint32 RemainingTicksInFrame(GameState* game_state)
{
    // TODO: Needs to be all high precision
	uint32 cur_time = SDL_GetTicks();
	uint32 next_frame_start = game_state->time.current_frame_ticks + game_state->time.ticks_per_frame;

	uint32 result = 0;

	if(cur_time < next_frame_start)
	{
		result = next_frame_start - cur_time;
	}

	return result;
}

inline uint32 RealTimeSinceStartup()
{
    // TODO: High precision
    uint32 result = SDL_GetTicks();
    return result;
}

/****************************
 *
 * Timer functions
 *
 ******/

inline void StartTimer(GameState* game_state, Timer* timer, float new_duration)
{
    if(new_duration > 0)
    {
		timer->duration = new_duration;
    }
    timer->start_time = CurrentTime(game_state);
}

inline float RemainingTime(GameState* game_state, Timer* timer)
{
	float result = timer->duration - (float)(CurrentTime(game_state) - timer->start_time);
	return result;
}

inline bool TimerIsFinished(GameState* game_state, Timer* timer)
{
	bool result = RemainingTime(game_state, timer) <= 0;
	return result;
}


/****************************
 *
 * Timer functions
 *
 ******/

Timer* CreateTimer(float dur);

void DestroyTimer(Timer*);

// If new_duration is 0, then the timer is restarted with the old duration.
void StartTimer(GameState*, Timer*, float new_duration = 0);

bool TimerIsFinished(GameState*, Timer*);

float RemainingTime(GameState*, Timer*);


