#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include "types.h"
#include <string>
#include "renderer.h"
#include <vector>

#define PROFILE_HISTORY_SIZE 100

#define ProfileBeginSection(name) _ProfileBeginSection(name, __FILE__, __LINE__)
#define ProfileEndSection(name) _ProfileEndSection(name, __FILE__, __LINE__)

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
    // TODO: Output color table at runtime

    Profile_Count,
};

struct ProfileSection
{
    u64 cycle_count_start;
    u64 sum;
    u64 min_cycles;
    u64 max_cycles;
    u32 hits;

    std::vector<SimpleVertex> history;
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

// Immediate mode text output / per frame rendering
struct Console
{
public:
    char* output_string;
    uint32 used_chars;
    uint32 array_size;

	Vec2 screen_start = vec2( 0.01f, 0.95f);

	void LogMessage(char* format, ...);
	void draw();

	static Console* get() { if (!s_instance) s_instance = new Console(); return s_instance; }

private:
	static Console* s_instance;
	Console();
};

//void DebugLogMessage(DebugConsole* console, char* format, ...);
//void DebugDrawConsole(DebugConsole* console);
//
//struct DebugState
//{
//
//    DebugConsole console
//};
