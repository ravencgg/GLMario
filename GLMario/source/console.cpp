#include "console.h"

Console* Console::s_instance = nullptr;

ProfileSection profile_sections[Profile_Count] = {0};

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
    memset(profile_sections, 0, sizeof(profile_sections));
}

void ProfileEndFrame()
{
    Console* console = Console::get();
    for(uint32 i = 0; i < Profile_Count; ++i)
    {
        u32 hits = profile_sections[i].hits;
        
        std::string output;
        output.reserve(256);
        output.append("Function: ");
        output.append(GetProfileSectionName((ProfileSectionName) i));
        output.append(" Hits: ");
        output.append(std::to_string(hits));

        if(hits)
        {
            output.append(" Average Cycles: ");
            output.append(std::to_string(profile_sections[i].sum / profile_sections[i].hits));
        }

        console->log_message(output);
    }
}

// Note: won't be thread safe;
// Since we are all single threaded for now, this will probably not be called again until the last one is done
// Put a sum in there so that the average can be found with a division 
void _ProfileBeginSection(ProfileSectionName name, char* file, int line)
{
    profile_sections[name].hits++;

    // summation will be wrong if this is not true
    assert(profile_sections[name].clock_start == 0);
    profile_sections[name].clock_start = __rdtsc();
}

void _ProfileEndSection(ProfileSectionName name, char* file, int line)
{
    profile_sections[name].sum += __rdtsc() - profile_sections[name].clock_start;
    profile_sections[name].clock_start = 0;
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
