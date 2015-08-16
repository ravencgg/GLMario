#include "console.h"

Console* Console::s_instance = nullptr;

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
