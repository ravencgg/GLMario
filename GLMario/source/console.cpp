#include "console.h"
#include "time.h"

Console* Console::s_instance = nullptr;

namespace
{
    ProfileSection profile_sections[Profile_Count] = {0};

    uint64 profile_frame_clock_start;
    uint64 profile_frame_clock_end;

}

Vec4 profile_colors[] =
{
    vec4(1, 0, 0, 1), // Red
    vec4(0, 1, 0, 1), // Green
    vec4(0, 0, 1, 1), // Blue
    vec4(1, 1, 0, 1), // Yellow
    vec4(1, 0, 1, 1), // Magenta
    vec4(0, 1, 1, 1), // Teal
};

std::string GetProfileSectionName(ProfileSectionName name)
{
#define Case(input) {case input: \
                    std::string result(#input); \
                    return result; \
                    }

    switch(name)
    {
        Case(Profile_Input)
        Case(Profile_PhysicsStepCollider)
        Case(Profile_PhysicsInnerLoop)
        Case(Profile_RenderFinish)
        Case(Profile_Console)
    }
    return std::string("Unknown");

#undef Case
}

void ProfileBeginFrame()
{
    profile_frame_clock_start = __rdtsc();
    for(int i = 0; i < Profile_Count; ++i)
    {
        ProfileSection* ps = profile_sections + i;
        ps->cycle_count_start = 0;
        ps->sum = 0;
        ps->hits = 0;

        if(ps->history.size() == 0)
        {
            ps->history.reserve(PROFILE_HISTORY_SIZE);
        }
    }
}

void ProfileEndFrame(Renderer* ren, uint32 )
{
    profile_frame_clock_end = __rdtsc();
    uint64 frame_cycles = profile_frame_clock_end - profile_frame_clock_start;

    Console* console = Console::get();

    const Rectf data_box = { 0.01f, 0.01f, 0.33f, 0.33f };

    for(uint32 i = 0; i < Profile_Count; ++i)
    {
        ProfileSection* section = profile_sections + i;
        u32 hits = section->hits;

        std::string output;
        output.reserve(256);
        output.append("Function: ");
        output.append(GetProfileSectionName((ProfileSectionName) i));
        output.append(" Hits: ");
        output.append(std::to_string(hits));

        if(hits)
        {
            output.append(" Average Cycles: ");
            output.append(std::to_string(section->sum / section->hits));
        }

        console->log_message(output);

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

        float res_x = (float)ren->get_resolution().width;
        float res_y = (float)ren->get_resolution().height;

        float current_pos  = section->history.size() / (float)PROFILE_HISTORY_SIZE;
        float screen_width = data_box.w * res_x;
        float screen_height = data_box.h * res_y;

        vertex->position.x = current_pos * screen_width + data_box.x * res_x;

        assert(section->sum < frame_cycles);

        float start_y = data_box.y * res_y;
        vertex->position.y = ((float)(section->sum) / (float)frame_cycles) * screen_height;

        vertex->position.y += start_y;


        assert(i < ArrayCount(profile_colors));
        vertex->color = profile_colors[i];

        if(ren)
        {
            const uint32 draw_options = LineDrawOptions::SCREEN_SPACE;
            const uint32 line_width = 1;
            ren->DrawLine(section->history, line_width, DrawLayer_UI, draw_options);
        }
    }
}

// Note: won't be thread safe;
// Since we are all single threaded for now, this will probably not be called again until the last one is done
// Put a sum in there so that the average can be found with a division
void _ProfileBeginSection(ProfileSectionName name, char* file, int line)
{
    profile_sections[name].hits++;
    profile_sections[name].clock_start = RealTimeSinceStartup();

    // summation will be wrong if this is not true
    assert(profile_sections[name].cycle_count_start == 0);
    profile_sections[name].cycle_count_start = __rdtsc();
}

void _ProfileEndSection(ProfileSectionName name, char* file, int line)
{
    profile_sections[name].sum += __rdtsc() - profile_sections[name].cycle_count_start;
    profile_sections[name].clock_end = RealTimeSinceStartup();
    profile_sections[name].cycle_count_start = 0;
}

Console::Console()
{
	count = 0;
}

void Console::log_message(string input)
{
	if(count < max_stored_lines)
	{
		lines[count++] = input;
	}
	else
	{
		assert(!"Too many lines being drawn to the console");
	}
}

void Console::draw()
{
	Renderer* ren = Renderer::get();

	volatile int w = 0;

	uint32 draw_y = (uint32)((float)ren->get_resolution().height * screen_start.y);
	uint32 draw_x = (uint32)((float)ren->get_resolution().width * screen_start.x);

	uint64 start = __rdtsc();

	uint32 num_chars = 0;

	for(uint32 i = 0; i < count; ++i)
	{
		TextDrawResult info = ren->draw_string(lines[i], draw_x, draw_y);
		draw_y = info.bottom_right.y;
		num_chars += lines[i].length();
	}
	count = 0;


	uint64 end = __rdtsc();
	uint64 total = end - start;
	uint64 average = (end - start) / num_chars;
	uint64 t2 = total;

	FILE* file;
	fopen_s(&file, "test.log", "ab");
	if (file)
	{
		std::string tot = std::to_string(total);
		std::string avg = std::to_string(total);
		fprintf(file, "Total: %s\tAverage: %s\r\n", tot.c_str(), avg.c_str());
		fclose(file);
	}

	int i = 0;
}
