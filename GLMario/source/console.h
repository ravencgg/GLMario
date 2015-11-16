#pragma once
#include "types.h"
#include <string>
#include "renderer.h"
#include <vector>
using std::string;

#define PROFILE_HISTORY_SIZE 100

#define ProfileBeginSection(name) _ProfileBeginSection(name, __FILE__, __LINE__)
#define ProfileEndSection(name) _ProfileEndSection(name, __FILE__, __LINE__)

enum ProfileSectionName : uint32
{
    Profile_Input,
    Profile_PhysicsStepCollider,
    Profile_PhysicsInnerLoop,
    Profile_RenderFinish,
    Profile_Console,
    Profile_Frame,

    // NOTE: don't add any more her without adding a new color to the profile_colors array

    Profile_Count,
};

struct ProfileSection
{
    u64 cycle_count_start;
    u64 sum;
    u32 hits;

    u32 clock_start;
    u32 clock_end;
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
std::string GetProfileSectionName(ProfileSectionName name);

// Immediate mode console/ per frame rendering
class Console
{
public:
	static const uint32 max_stored_lines = 250;

	string lines[max_stored_lines];
	uint32 count;
	Vec2 screen_start = vec2( 0.01f, 0.975f );

	void log_message(string input);
	void draw();

	static Console* get() { if (!s_instance) s_instance = new Console(); return s_instance; }

private:
	static Console* s_instance;
	Console();
};
