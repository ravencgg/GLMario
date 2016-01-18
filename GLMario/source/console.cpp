#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

#include "types.h"
#include "mathops.h"

#include "console.h"
#include "time.h"

#include <inttypes.h>

// Immediate mode text output / per frame rendering
#define DEBUG_CONSOLE_STACK_SIZE 1024
struct DebugConsole
{
    char* output_string;
    uint32 used_chars;
    uint32 array_size;
    StringTextColor text_color_stack[DEBUG_CONSOLE_STACK_SIZE];
    size_t text_color_stack_used = 0;
    StringTextColor text_format_array[DEBUG_CONSOLE_STACK_SIZE];
    size_t text_format_array_used = 0;
	Vec2 screen_start = vec2( 0.01f, 0.95f);
};

#define CONSOLE_STRING_START_SIZE 1024

namespace
{
    // Debug Graph
    ProfileSection profile_sections[Profile_Count] = {0};
    uint64 profile_frame_clock_start;
    uint64 profile_frame_clock_end;
    uint64 cycles_per_second;

    // Console Output
    DebugConsole console;
}

Vec4 profile_colors[] =
{
    vec4(1, 0,    0,    1), // Red        Profile_Input,
    vec4(0, 1,    0,    1), // Green      Profile_PhysicsStepCollider,
    vec4(0, 0,    1,    1), // Blue       Profile_PhysicsInnerLoop,
    vec4(1, 1,    0,    1), // Yellow     Profile_RenderFinish,
    vec4(1, 1,    1,    1), // White      Profile_ParticleUpdate,
    vec4(0, 1,    1,    1), // Teal       Profile_Frame,
    vec4(0, 0.5f, 0.5f, 1), // Something  Profile_SceneUpdate,
    vec4(1, 0,    1,    1), // Magenta    Profile_Console,
    vec4(1, 1,    1,    1), // White      Unused,
};

char* GetProfileSectionName(ProfileSectionName name)
{
#define Case(input) {case input: \
                    char* result = #input; \
                    return result; \
                    }

    switch(name)
    {
        Case(Profile_Input)
        Case(Profile_PhysicsStepCollider)
        Case(Profile_PhysicsInnerLoop)
        Case(Profile_RenderFinish)
        Case(Profile_ParticleUpdate)
        Case(Profile_Frame)
        Case(Profile_SceneUpdate)
        Case(Profile_Console)
    }
    return "Unknown";

#undef Case
}

void ProfileBeginFrame()
{
    profile_frame_clock_start = GetCycleCount();
    for(int i = 0; i < Profile_Count; ++i)
    {
        ProfileSection* ps = profile_sections + i;
        ps->cycle_count_start = 0;
        ps->sum = 0;
        ps->hits = 0;
        ps->min_cycles = (uint64)-1;
        ps->max_cycles = 0;
    }
}

void ProfileEndFrame(Renderer* ren, uint32 target_fps)
{
    ProfileBeginSection(Profile_Console);

    profile_frame_clock_end = GetCycleCount();
    cycles_per_second       = GetCyclesPerSecond();
    uint64 frame_cycles     = profile_frame_clock_end - profile_frame_clock_start;
    uint64 target_cycles    = cycles_per_second / target_fps;

    // Graph drawing variables

    // In screen ratio coords with (0, 0) being the lower left corner
    Rectf data_box = { 0.55f, 0.01f, 0.33f, 0.33f };

    for(uint32 i = 0; i < Profile_Count; ++i)
    {
        ProfileSection* section = profile_sections + i;

        if(i == Profile_Console)
        {
//            ProfileEndSection(Profile_Console);
        }

        // Text output
        if(1)
        {
            u32 hits = section->hits;

//            DebugPrintPushColor(profile_colors[i]);
            DebugPrintPushGradient(profile_colors[i], profile_colors[i + 1]);

            if(hits > 1)
            {
                DebugPrintf("Function: (%s) Hits: %d Average: %"PRIu64" Min: %"PRIu64" Max: %"PRIu64"",
                                        GetProfileSectionName((ProfileSectionName) i),
                                        hits, (section->sum / section->hits),
                                        section->min_cycles, section->max_cycles);
            }
            else if(hits == 1)
            {
                DebugPrintf("Function: (%s) Hits: %d Average Cycles: %"PRIu64"",
                                        GetProfileSectionName((ProfileSectionName) i),
                                        hits, (section->sum / section->hits));
            }
            else
            {
                DebugPrintf("Function: (%s) Hits: %d",
                                        GetProfileSectionName((ProfileSectionName) i),
                                        hits);
            }

            DebugPrintPopColor();
        }

        {
            if(section->used_history < PROFILE_HISTORY_SIZE)
            {
                ++section->used_history;
            }
            else
            {
                SimpleVertex* curr = section->history;
                SimpleVertex* next = curr + 1;
                for (size_t i = 0; i < section->used_history - 1; ++i)
                {
                    curr->position.y = next->position.y;
                    //curr->color = next->color; // Only needed if changing the color per vertex, not per type
                    ++curr;
                    ++next;
                }
            }

            SimpleVertex* vertex = section->history + section->used_history - 1;
            float current_pos  = section->used_history / (float)PROFILE_HISTORY_SIZE;
            vertex->position.x = current_pos * data_box.w + data_box.x;

            vertex->position.y = ((float)(section->sum) / (float)target_cycles) * data_box.h;
            vertex->position.y += data_box.y;

            assert(i < ArrayCount(profile_colors));
            vertex->color = profile_colors[i];

            if(ren)
            {
                PrimitiveDrawParams params = {};
                params.line_draw_flags |= Draw_ScreenSpace;
                params.line_width = 1;
                DrawLine(ren, section->history, section->used_history, params);
            }
        }
    }

    DebugPrintf("Target cycles: %"PRIu64" Cycles per second: %"PRIu64"\n", target_cycles, cycles_per_second);

    const Vec4 color = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    if(ren)
    {
        PrimitiveDrawParams params = {};
        params.line_draw_flags = Draw_ScreenSpace;
        params.line_width = 2;
        DrawRect(ren, data_box, color, 0, params);
    }
}

// Note: won't be thread safe;
// Since we are all single threaded for now, this will probably not be called again until the last one is done
// Put a sum in there so that the average can be found with a division
void _ProfileBeginSection(ProfileSectionName name, char* file, int line)
{
    ProfileSection* section = &profile_sections[name];
    section->hits++;

    // summation will be wrong if this is not true
    // this means that recursive functions can't be calculated though
    assert(section->cycle_count_start == 0);
    section->cycle_count_start = GetCycleCount();
}

void _ProfileEndSection(ProfileSectionName name, char* file, int line)
{
    ProfileSection* section = &profile_sections[name];
    u64 cycle_count = GetCycleCount() - section->cycle_count_start;;
    section->min_cycles = MIN(section->min_cycles, cycle_count);
    section->max_cycles = MAX(section->max_cycles, cycle_count);
    section->sum += cycle_count;
    section->cycle_count_start = 0;
}

void InitializeDebugConsole()
{
    console.array_size = CONSOLE_STRING_START_SIZE;
    console.output_string = new char[console.array_size];

    const Vec4 default_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    DebugPrintPushColor(default_color);
}

void DebugPrintPushGradient(Vec4 start_color, Vec4 end_color)
{
    if (console.text_color_stack_used == DEBUG_CONSOLE_STACK_SIZE)
    {
        assert(!"Full Stack");
        return;
    }
    StringTextColor* stc = &console.text_color_stack[console.text_color_stack_used++];

    stc->c.gradient_start = start_color;
    stc->c.gradient_end   = end_color;
    stc->color_options = StringColorOptions_Gradient;
}

void DebugPrintPushColor(Vec4 solid_color)
{
    if (console.text_color_stack_used == DEBUG_CONSOLE_STACK_SIZE)
    {
        assert(!"Full Stack");
        return;
    }
    StringTextColor* stc = &console.text_color_stack[console.text_color_stack_used++];

    stc->c.solid_color = solid_color;
    stc->color_options = StringColorOptions_Solid;
}

void DebugPrintPopColor()
{
    console.text_color_stack_used = MAX(1, console.text_color_stack_used - 1);
}

void DebugPrintf(char* format, ... )
{
    uint32 remaining_buffer_size = console.array_size - console.used_chars;

    if (console.text_format_array_used == DEBUG_CONSOLE_STACK_SIZE)
    {
        assert(!"Full Stack");
        return;
    }
    StringTextColor* current_color = &console.text_format_array[console.text_format_array_used++];
    StringTextColor* color_stack_back = &console.text_color_stack[console.text_color_stack_used - 1];

    current_color->color_options = color_stack_back->color_options;

    if(current_color->color_options == StringColorOptions_Solid)
    {
        current_color->c.solid_color = color_stack_back->c.solid_color;
        current_color->start = console.used_chars;
    }
    else if(current_color->color_options == StringColorOptions_Gradient)
    {
        current_color->c.gradient_start = color_stack_back->c.gradient_start; 
        current_color->c.gradient_end   = color_stack_back->c.gradient_end; 
    }
    else
    {
        assert(!"Unknown color option");
    }
    current_color->start = console.used_chars;

    va_list args;
    va_start(args, format);
    // NOTE: return value does not count the terminating null
    // NOTE: Windows only, linux OSes can use vsnprintf with a NULL buffer to find the required size
    int32 input_size = _vscprintf(format, args) + 1; // Add 1 for the '\n'
    va_end(args);

    assert(input_size >= 0);

    // Not enough space in the buffer, reallocate
    if((uint32) input_size >= remaining_buffer_size)
    {
        uint32 needed_space = input_size - remaining_buffer_size;
        uint32 new_size = (needed_space < console.array_size) ? console.array_size * 2 : needed_space * 2 + console.array_size;
        console.output_string = ExpandArray<char>(console.output_string, console.array_size, new_size);
        console.array_size = new_size;

        remaining_buffer_size = console.array_size - console.used_chars;
    }

    char* start_position = console.output_string + console.used_chars;

    va_start(args, format);
    input_size = vsnprintf(start_position, remaining_buffer_size, format, args);
    va_end(args);

    start_position += input_size;
    *start_position = '\n'; // This can overwrite the terminating null because the size is tracked through
                            // to rendering.

    console.used_chars += input_size + 1;
    assert(input_size >= 0);
    current_color->end = console.used_chars;
}

void DebugDrawConsole(Renderer* ren)
{
	float draw_y = console.screen_start.y;
	float draw_x = console.screen_start.x;
	uint32 num_chars = 0;

// TODO: replace with real system from the input
//    const size_t array_size = 2;
//    StringTextColor colors[array_size];
//    colors[0].start = 0;
//    colors[0].end   = console.used_chars / 2;
//    colors[0].color = { 1.f, 0.f, 1.f, 1.f };
//
//    colors[1].start = colors[0].end;
//    colors[1].end   = console.used_chars;
//    colors[1].color = { 1.f, 0, 0, 1.f };

// TODO: passing the raw array in here is dangerous
//
    if(ren)
    {
        DrawString(ren, console.output_string, console.used_chars, draw_x, draw_y,
                        console.text_format_array, console.text_format_array_used);
    }

    console.used_chars = 0;
    console.text_format_array_used = 0;
    console.text_color_stack_used = 1;
}

