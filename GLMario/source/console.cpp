#include "console.h"
#include "time.h"

#include <inttypes.h>

Console* Console::s_instance = nullptr;

namespace
{
    ProfileSection profile_sections[Profile_Count] = {0};

    uint64 profile_frame_clock_start;
    uint64 profile_frame_clock_end;

    uint64 cycles_per_second;
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
        ps->min_cycles = -1;
        ps->max_cycles = 0;

        if(ps->history.size() == 0)
        {
            ps->history.reserve(PROFILE_HISTORY_SIZE);
        }
    }
}

void ProfileEndFrame(Renderer* ren, uint32 target_fps)
{
    ProfileBeginSection(Profile_Console);

    profile_frame_clock_end = GetCycleCount();
    cycles_per_second       = GetCyclesPerSecond();
    uint64 frame_cycles     = profile_frame_clock_end - profile_frame_clock_start;
    uint64 target_cycles    = cycles_per_second / target_fps;

    Console* console = Console::get();

    // Graph drawing variables

    // In screen ratio coords with (0, 0) being the lower left corner
    Rectf data_box = { 0.55f, 0.01f, 0.33f, 0.33f };
    float res_x = (float)ren->get_resolution().width;
    float res_y = (float)ren->get_resolution().height;

    for(uint32 i = 0; i < Profile_Count; ++i)
    {
        ProfileSection* section = profile_sections + i;

        if(i == Profile_Console)
        {
            ProfileEndSection(Profile_Console);
        }

        // Text output
        {
            u32 hits = section->hits;

            if(hits > 1)
            {
                console->LogMessage("Function: (%s) Hits: %d Average: %"PRIu64" Min: %"PRIu64" Max: %"PRIu64"",
                                        GetProfileSectionName((ProfileSectionName) i),
                                        hits, (section->sum / section->hits),
                                        section->min_cycles, section->max_cycles);
            }
            else if(hits == 1)
            {
                console->LogMessage("Function: (%s) Hits: %d Average Cycles: %"PRIu64"",
                                        GetProfileSectionName((ProfileSectionName) i),
                                        hits, (section->sum / section->hits));
            }
            else
            {
                console->LogMessage("Function: (%s) Hits: %d",
                                        GetProfileSectionName((ProfileSectionName) i),
                                        hits);
            }
        }

        {
            if(section->history.size() < PROFILE_HISTORY_SIZE)
            {
                section->history.push_back({});
                SimpleVertex* vertex = &section->history.back();
            }
            else
            {
                for(uint32 j = 0; j < section->history.size() - 1; ++j)
                {
                    section->history[j].position.y = section->history[j + 1].position.y;
                }
            }

            SimpleVertex* vertex = &section->history.back();

            float current_pos  = section->history.size() / (float)PROFILE_HISTORY_SIZE;
            float screen_width = data_box.w * res_x;
            float screen_height = data_box.h * res_y;

            vertex->position.x = current_pos * screen_width + data_box.x * res_x;

            //assert(section->sum < frame_cycles);

            float start_y = data_box.y * res_y;
            vertex->position.y = ((float)(section->sum) / (float)target_cycles) * screen_height;

            vertex->position.y += start_y;

            assert(i < ArrayCount(profile_colors));
            vertex->color = profile_colors[i];

            const uint32 draw_options = LineDrawOptions::SCREEN_SPACE;
            const uint32 line_width = 1;
            ren->DrawLine(section->history, line_width, DrawLayer_UI, draw_options);
        }
    }

    console->LogMessage("Target cycles: %"PRIu64" Cycles per second: %"PRIu64"\n", target_cycles, cycles_per_second);

//    const Vec2 line_start = vec2(data_box.x * res_x, (data_box.y + data_box.height) * res_y);
//    const Vec2 line_end   = vec2((data_box.x + data_box.width) * res_x, (data_box.y + data_box.height) * res_y);
//    ren->DrawLine(line_start, line_end, color, 2, DrawLayer_UI, LineDrawOptions::SCREEN_SPACE);

    data_box.x *= res_x;
    data_box.w *= res_x;
    data_box.y *= res_y;
    data_box.h *= res_y;

    const Vec4 color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    ren->DrawRect(data_box, 2, DrawLayer_UI, color, LineDrawOptions::SCREEN_SPACE);

// NOTE: Second flush of the frame, just to push out the console data;
// TODO: don't do the performance stuff on that one.
    ren->Flush();
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

#define CONSOLE_STRING_START_SIZE 20
Console::Console()
:   used_chars(0),
    array_size(CONSOLE_STRING_START_SIZE)
{
    this->output_string = new char[this->array_size];
}

void Console::LogMessage(char* format, ... )
{
    uint32 remaining_buffer_size = this->array_size - this->used_chars;

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
        uint32 new_size = (needed_space < this->array_size) ? this->array_size * 2 : needed_space * 2 + this->array_size;
        this->output_string = ExpandArray<char>(this->output_string, this->array_size, new_size);
        this->array_size = new_size;

        remaining_buffer_size = this->array_size - this->used_chars;
    }

    char* start_position = this->output_string + used_chars;

    va_start(args, format);
    input_size = vsnprintf(start_position, remaining_buffer_size, format, args);
    va_end(args);

    start_position += input_size;
    *start_position = '\n'; // This can overwrite the terminating null because the size is tracked through
                            // to rendering.

    this->used_chars += input_size + 1;
    assert(input_size >= 0);
}

void Console::draw()
{
	Renderer* ren = Renderer::get();

	uint32 draw_y = (uint32)((float)ren->get_resolution().height * screen_start.y);
	uint32 draw_x = (uint32)((float)ren->get_resolution().width * screen_start.x);

	uint32 num_chars = 0;

    ren->DrawString(this->output_string, this->used_chars, draw_x, draw_y);
    //ren->DrawString(this->output_string, 100, 100, this->used_chars);
    this->used_chars = 0;
}

