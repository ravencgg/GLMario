#pragma once

#include <assert.h>
#include "SDL.h" 
#include "types.h"


class Time
{
public:

	double current_time;
	double delta_time;

	uint32 last_frame_ticks;
	uint32 current_frame_ticks;
	uint32 ticks_per_frame;

	uint32 frame_count;

	uint32 ticks_for_frame_cap();
	void begin_frame();

	Time();
	~Time() {}
	static Time* get() { if (!s_time) { s_time = new Time(); } return s_time; }

private:
	static Time* s_time;
};

struct Timer
{
	float start_time;
	float duration;

	uint32 last_frame_count;

	Timer(float);
	bool is_finished();
	float remaining_time();
	void reset();
	void reset(bool, float);
};
