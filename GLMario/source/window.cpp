#include "window.h"

Window::Window(char* title, int32 width, int32 height)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL init error\n");
	}

	sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if(sdl_window == nullptr)
	{
		printf("Window creation error\n");
	}

	sdl_gl_context = SDL_GL_CreateContext(sdl_window);
	if(sdl_gl_context == nullptr)
	{
		printf("GL Context creation error\n");
	}

	SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
}

Window::~Window()
{
	if(sdl_gl_context) SDL_GL_DeleteContext(sdl_gl_context);
	if(sdl_window)	   SDL_DestroyWindow(sdl_window);
}

Dimension Window::get_resolution()
{
	Dimension result;
	SDL_GetWindowSize(sdl_window, &result.width, &result.height);
	return result;
}

void Window::set_resolution(uint32 w, uint32 h)
{
	SDL_SetWindowSize(sdl_window, w, h);
}

void Window::swap_buffer()
{
	SDL_GL_SwapWindow(sdl_window);
}

void Window::set_window_mode(ScreenMode mode)
{
	SDL_SetWindowFullscreen(sdl_window, (uint32)mode);
}
