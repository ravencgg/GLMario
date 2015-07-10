#include "console.h"

Console* Console::s_instance = nullptr;

Console::Console()
{
	head = 0;
}

void Console::log_message(string input)
{
	lines[head++] = input;
	if (head >= max_stored_lines) head = 0;
}

void Console::draw()
{
	Renderer* ren = Renderer::get();
	
	uint32 draw_y = (uint32)((float)ren->get_resolution().height * screen_start.y);
	uint32 draw_x = (uint32)((float)ren->get_resolution().width * screen_start.x);

	TextDrawResult lines_drawn = ren->draw_string(lines[0], draw_x, draw_y);
}