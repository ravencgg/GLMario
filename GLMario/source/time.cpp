#include "time.h"

#include "SDL.h"
#include <assert.h>
#include <string.h>
#include "utility.h"


namespace
{
	double current_time;
	double delta_time;

	uint32 last_frame_ticks;
	uint32 current_frame_ticks;
	uint32 ticks_per_frame;

	uint32 frame_count;
}

/****************************
 *
 * Time functions
 *
 ******/

#define MAX_FRAME_TIME 0.25f


double CurrentTimePrecise()
{
    return current_time;
}

float CurrentTime()
{
    return (float)current_time;
}

float FrameTime()
{
    // TODO: This is really debug only
    float result = MIN((float)delta_time, MAX_FRAME_TIME);
    return (float)delta_time;
}

uint32 FrameCount()
{
    return frame_count;
}

void InitializeTime(uint32 ms_per_frame)
{
	ticks_per_frame = 16;
	current_frame_ticks = SDL_GetTicks();
}

void TimeBeginFrame()
{
	last_frame_ticks = current_frame_ticks;
	current_frame_ticks = SDL_GetTicks();

	current_time = (double)current_frame_ticks / 1000.0;
	delta_time = (double)(current_frame_ticks - last_frame_ticks) / 1000.0;

	++frame_count;
}

uint32 RemainingTicksInFrame()
{
	uint32 cur_time = SDL_GetTicks();
	uint32 next_frame_start = current_frame_ticks + ticks_per_frame;

	uint32 result = 0;

	if(cur_time < next_frame_start)
	{
		result = next_frame_start - cur_time;
	}

	return result;
}

uint32 RealTimeSinceStartup()
{
    uint32 result = SDL_GetTicks();
    return result;
}

/****************************
 *
 * Timer functions
 *
 ******/

Timer* CreateTimer(float dur)
{
    NEW_ZERO(result, Timer);
	assert(dur >= 0);
    StartTimer(result, dur);
    return result;
}

void DestroyTimer(Timer* timer)
{
    delete timer;
}

void StartTimer(Timer* timer, float new_duration)
{
    if(new_duration > 0)
    {
		timer->duration = new_duration;
    }
    timer->start_time = CurrentTime();
}

bool TimerIsFinished(Timer* timer)
{
	bool result = RemainingTime(timer) <= 0;
	return result;
}

float RemainingTime(Timer* timer)
{
	float result = timer->duration - (float)(CurrentTime() - timer->start_time);
	return result;
}
