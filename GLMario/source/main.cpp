#include "SDL.h"

#include <stdio.h>

#include "renderer.h"
#include "input.h"
#include "scene_manager.h"
#include "time.h"
#include "game_types.h"

#include <inttypes.h> // For 64 bit int printf output

#include "containers.h"

// Should be like 16.66666
#define MS_PER_FRAME 16
#define TARGET_FPS (1000 / MS_PER_FRAME)

void StartupWindow(Window* window, char* title, int32 width, int32 height)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL init error\n");
	}

    window->resolution.width  = width;
    window->resolution.height = height;
    window->current_mode = ScreenMode_Windowed;

	window->sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  width, height,
                                  SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if(window->sdl_window == nullptr)
	{
		printf("Window creation error\n");
	}

	window->sdl_gl_context = SDL_GL_CreateContext(window->sdl_window);
	if(window->sdl_gl_context == nullptr)
	{
		printf("GL Context creation error\n");
	}

	SDL_GL_MakeCurrent(window->sdl_window, window->sdl_gl_context);
}

void ShutdownWindow(Window* window)
{
	if(window->sdl_gl_context) SDL_GL_DeleteContext(window->sdl_gl_context);
	if(window->sdl_window)	   SDL_DestroyWindow(window->sdl_window);
}

void WindowSetResolution(Window* window, uint32 w, uint32 h)
{
    window->resolution.width = w;
    window->resolution.height = h;
	SDL_SetWindowSize(window->sdl_window, w, h);
}

void WindowSetScreenMode(Window* window, ScreenMode mode)
{
    window->current_mode = mode;
	SDL_SetWindowFullscreen(window->sdl_window, (uint32)mode);
}

int main(int argc, char* argv[])
{
	assert(argc || argv[0]); // Fixes the compiler complaining about unused values;

    GameState* game_state = new GameState;

    StartupWindow(&game_state->window, "Title", 1200, 700);

    InitializeTime(game_state, MS_PER_FRAME);

	Renderer::create_instance(&game_state->window);
	Renderer* renderer = Renderer::get();

	Input* input = Input::get();
	SceneManager scene;
	Camera main_camera(&scene);

	scene.SetMainCamera(&main_camera);
	renderer->set_camera(&main_camera);

	uint32 frame_count = 0;
	uint32 fps = 0;
	double last_fps_time = 0;

	SDL_Event e;
	bool running = true;

	while(running)
	{
        ProfileBeginFrame();

        ProfileBeginSection(Profile_Frame);
        ProfileBeginSection(Profile_Input);

		input->begin_frame();
		while (SDL_PollEvent(&e))
		{
			//If user closes the window
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
			//If user presses any key
			if (e.type == SDL_KEYDOWN)
			{
				input->process_key_press(e.key.keysym.sym);
			}
			if (e.type == SDL_KEYUP)
			{
				input->process_key_release(e.key.keysym.sym);
			}

			if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
			{
				input->mouse_button_event();
			}
			if(e.type == SDL_MOUSEMOTION)
			{
				input->update_mouse_position();
			}
		}

        input->update_mouse_world_position(game_state->window.resolution, main_camera.viewport_size, main_camera.transform.position);

        ProfileEndSection(Profile_Input);

        Vec2 mouse_pos = input->mouse_world_position();
        Console::get()->LogMessage("Mouse World Position: (%.2f, %.2f)",  mouse_pos.x, mouse_pos.y);

		if(input->on_down(SDLK_ESCAPE))
		{
			running = false;
			break;
		}

		if(input->on_down(SDLK_z))
		{
			WindowSetScreenMode(&game_state->window, ScreenMode_Windowed);
			renderer->force_color_clear();
		}
		else if(input->on_down(SDLK_c))
		{
            WindowSetScreenMode(&game_state->window, ScreenMode_Borderless);
			renderer->force_color_clear();
		}
		// TODO(cgenova): profiling;

	    TimeBeginFrame(game_state);

		// Update the scene first, pushing draw calls if necessary.
		// Then call begin_frame which builds matrices and clears buffers;
        float current_time = CurrentTime(game_state);
		if(current_time - last_fps_time > 1.0f)
		{
			last_fps_time = current_time;
			fps = frame_count;
			frame_count = 0;
#if _DEBUG
            printf("FPS: %d\n", fps);
#endif
		}
		frame_count++;
		Console::get()->LogMessage("FPS: \t\t%d \tFrames: \t%d", fps, FrameCount(game_state));

		// TODO(cgenova): separate update and render calls so that things can be set up when rendering begins;
		renderer->begin_frame();
		main_camera.Tick(game_state);

        ProfileBeginSection(Profile_SceneUpdate);
		scene.update_scene(game_state);
        ProfileEndSection(Profile_SceneUpdate);

        static Array<SimpleVertex> v;
        static bool initialized = false;
        if(!initialized)
        {
            initialized = true;
            for(uint32 i = 0; i < 200; ++i)
            {
                SimpleVertex verts = {};
                verts.position = vec2((float) (i / 50.f) - 2.f, (float) i);
                verts.color = vec4(1, 1, 0, 1.f);
                v.Add(verts);
            }
        }
        else
        {
            for(uint32 i = 0; i < v.Size(); ++i)
            {
				v[i].position.y = sin(CurrentTime(game_state) + i / (PI * 20));
            }
        }
        renderer->DrawLine(v, 3, DrawLayer_UI, LineDrawOptions::SMOOTH);
		//renderer->render_draw_buffer();

		renderer->Flush();

		Console::get()->draw();

        ProfileEndSection(Profile_Frame);
        ProfileEndFrame(renderer, TARGET_FPS);

        uint64 pre_swap_time = GetCycleCount();
        SwapBuffer(game_state);
        uint64 post_swap_time = GetCycleCount();

        Console::get()->LogMessage("Swap time: %"PRIu64"", post_swap_time - pre_swap_time);

		// End rendering


		// TODO(cgenova): High granularity sleep function!
//		uint32 delay_time = RemainingTicksInFrame();
//		if(delay_time > 10) {
			//std::cout << "Delaying: " << delay_time << " ms" << std::endl;
//			SDL_Delay(delay_time);
//		}
		//std::cout << "Delta T : " << delay_time << " ms" << std::endl;

	}// End main loop

	SDL_Quit();

	return 0;
}
