#include "time.h"

Time* Time::s_time = nullptr;

Time::Time()
:	delta_time(0),
	last_frame_ticks(0),
	ticks_per_frame(16), // Default is 60 fps
	frame_count(0)
{
	current_frame_ticks = SDL_GetTicks();
}

void Time::begin_frame()
{
	last_frame_ticks = current_frame_ticks;
	current_frame_ticks = SDL_GetTicks();

	current_time = (double)current_frame_ticks / 1000.0;
	delta_time = (double)(current_frame_ticks - last_frame_ticks) / 1000.0;

	++frame_count;
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



Timer::Timer(float d)
{
	assert(d > 0);
	duration = d;
	start_time = (float)Time::get()->current_time;
}

bool Timer::is_finished()
{
	bool result = remaining_time() <= 0;
	return result; 
}

float Timer::remaining_time()
{
	float result = duration - (float)(Time::get()->current_time - start_time); 
	return result;
}

void Timer::reset()
{
	start_time = (float)Time::get()->current_time;
}

void Timer::reset(bool overwrite_duration, float new_duration = 0)
{
	assert (new_duration >= 0);
	if(overwrite_duration)
	{
		duration = new_duration;
	}
	start_time = (float)Time::get()->current_time;
}

