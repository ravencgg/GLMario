#include "time.h"

Time* Time::s_time = nullptr;

Time::Time()
:	delta_time(0),
	last_frame_ticks(0),
	ticks_per_frame(16) // Default is 60 fps
{
	current_frame_ticks = SDL_GetTicks();
}

void Time::begin_frame()
{
	last_frame_ticks = current_frame_ticks;
	current_frame_ticks = SDL_GetTicks();

	current_time = (double)current_frame_ticks / 1000.0;
	delta_time = (double)(current_frame_ticks - last_frame_ticks) / 1000.0;
}

uint32 Time::ticks_for_frame_cap()
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

