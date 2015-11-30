#pragma once

#include "types.h"


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
 * Time functions
 *
 ******/


double CurrentTimePrecise();

float CurrentTime();

float FrameTime();

uint32 FrameCount();

void InitializeTime(uint32 ms_per_frame);

void TimeBeginFrame();

uint32 RemainingTicksInFrame();

uint32 RealTimeSinceStartup();


/****************************
 *
 * Timer functions
 *
 ******/

Timer* CreateTimer(float dur);

void DestroyTimer(Timer* timer);

// If new_duration is 0, then the timer is restarted with the old duration.
void StartTimer(Timer* timer, float new_duration = 0);

bool TimerIsFinished(Timer* timer);

float RemainingTime(Timer* timer);
