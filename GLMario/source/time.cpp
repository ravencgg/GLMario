#include "game_types.h"

#include "time.h"
#include "mathops.h"
#include "sdl.h"

/****************************
 *
 *
 * Time functions
 *
 ******/

#define MAX_FRAME_TIME 0.25f


double CurrentTimePrecise(GameState* game_state)
{
    return game_state->time.current_time;
}

float CurrentTime(GameState* game_state)
{
    return (float)game_state->time.current_time;
}

float FrameTime(GameState* game_state)
{
    // TODO: This is really debug only
    float result = min((float)game_state->time.delta_time, MAX_FRAME_TIME);
    return result;
}

uint32 FrameCount(GameState* game_state)
{
    return game_state->time.frame_count;
}

void InitializeTime(GameState* game_state, uint32 ms_per_frame)
{
    // TODO: convert this to high precision
    memset(&game_state->time, 0, sizeof(game_state->time));
	game_state->time.ticks_per_frame = 16;
	game_state->time.current_frame_ticks = SDL_GetTicks();
}

void TimeBeginFrame(GameState* game_state)
{
	game_state->time.last_frame_ticks = game_state->time.current_frame_ticks;
	game_state->time.current_frame_ticks = SDL_GetTicks();

	game_state->time.current_time = (double)game_state->time.current_frame_ticks / 1000.0;
	game_state->time.delta_time = (double)(game_state->time.current_frame_ticks - game_state->time.last_frame_ticks) / 1000.0;

	++game_state->time.frame_count;
}

uint32 RemainingTicksInFrame(GameState* game_state)
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

uint32 RealTimeSinceStartup()
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

void StartTimer(GameState* game_state, Timer* timer, float new_duration)
{
    if(new_duration > 0)
    {
		timer->duration = new_duration;
    }
    timer->start_time = CurrentTime(game_state);
}

bool TimerIsFinished(GameState* game_state, Timer* timer)
{
	bool result = RemainingTime(game_state, timer) <= 0;
	return result;
}

float RemainingTime(GameState* game_state, Timer* timer)
{
	float result = timer->duration - (float)(CurrentTime(game_state) - timer->start_time);
	return result;
}
