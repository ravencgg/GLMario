#pragma once

#include "types.h"
#include "renderer.h"
#include "mathops.h"
#include <vector>
#include <string>


#define PROFILE_HISTORY_SIZE 100

#if 1
#define ProfileBeginSection(name) _ProfileBeginSection(name, __FILE__, __LINE__)
#define ProfileEndSection(name) _ProfileEndSection(name, __FILE__, __LINE__)
#else
#define ProfileBeginSection(name)
#define ProfileEndSection(name)
#endif

enum ProfileSectionName : uint32
{
    Profile_Input,
    Profile_PhysicsStepCollider,
    Profile_PhysicsInnerLoop,
    Profile_RenderFinish,
    Profile_ParticleUpdate,
    Profile_Frame,
    Profile_SceneUpdate,
    Profile_Console,

    // NOTE: don't add any more her without adding a new color to the profile_colors array
    // TODO: Fix that note

    Profile_Count,
};

struct ProfileSection
{
    u64 cycle_count_start;
    u64 sum;
    u64 min_cycles;
    u64 max_cycles;
    u32 hits;

    Array<SimpleVertex> history;
};

struct DebugProfile
{
    uint32 history_size;
    ProfileSection profiles[Profile_Count];
};

void _ProfileEndSection(ProfileSectionName name, char* file, int line);
void _ProfileBeginSection(ProfileSectionName name, char* file, int line);
void ProfileEndFrame(Renderer* ren, uint32 desired_frame_time);
void ProfileBeginFrame();
char* GetProfileSectionName(ProfileSectionName name);

void InitializeDebugConsole();
void DebugDrawConsole(Renderer* renderer);
void DebugPrintPushGradient(Vec4 start_color, Vec4 end_color);
void DebugPrintPushColor(Vec4 solid_color);
void DebugPrintPopColor();
void DebugPrintf(char* format, ...);

