#pragma once
#include "types.h"
#include <string>
#include "renderer.h"
using std::string;

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
