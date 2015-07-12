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
		assert("Too many lines being drawn to the console" && 0);
	}
}

void Console::draw()
{
	Renderer* ren = Renderer::get();
	
	uint32 draw_y = (uint32)((float)ren->get_resolution().height * screen_start.y);
	uint32 draw_x = (uint32)((float)ren->get_resolution().width * screen_start.x);

	for(uint32 i = 0; i < count; ++i)
	{
		TextDrawResult info = ren->draw_string(lines[i], draw_x, draw_y);
		draw_y = info.bottom_right.y;
	}
	count = 0;
}