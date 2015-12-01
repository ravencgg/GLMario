#pragma once

#include "types.h"
#include "game_types.h"

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
 * Timer functions
 *
 ******/

Timer* CreateTimer(float dur);

void DestroyTimer(Timer*);

// If new_duration is 0, then the timer is restarted with the old duration.
void StartTimer(GameState*, Timer*, float new_duration = 0);

bool TimerIsFinished(GameState*, Timer*);

float RemainingTime(GameState*, Timer*);

