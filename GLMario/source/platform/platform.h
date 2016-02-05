#pragma once

#include "../types.h"
#include "../input.h"

// Process system messages
bool Platform_RunMessageLoop(NewInput* input);

void Platform_SwapBuffers();

// Get the current drawable client size
void Platform_GetDrawableSize(int* x, int* y);

Vec2i Platform_GetResolution();


/**********************************************
 *
 * Clock
 *
 ***************/


uint64 Platform_GetCycleCount();

uint64 Platform_GetCyclesPerSecond();

uint32 Platform_GetTickCount();


/********************
 *
 *  Implemented by the game, called from the platform layer
 *
 *********/

// Game entry point declared in main.cpp
int GameMain();
