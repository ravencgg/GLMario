#include "SDL.h"

#include <stdio.h>

#include "window.h"
#include "renderer.h"
#include "input.h"
#include "scene_manager.h"
#include "time.h"

#include "containers.h"

// Should be like 16.66666
#define MS_PER_FRAME 16
#define TARGET_FPS (1000 / MS_PER_FRAME)

int main(int argc, char* argv[])
{
	assert(argc || argv[0]); // Fixes the compiler complaining about unused values;
	Window window("Title", 1400, 900);

    InitializeTime(MS_PER_FRAME);

	Renderer::create_instance(&window);
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
#if 1
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

        input->update_mouse_world_position(window.get_resolution(), main_camera.viewport_size, main_camera.transform.position);

        ProfileEndSection(Profile_Input);

        Vec2 mouse_pos = input->mouse_world_position();
        Console::get()->LogMessage("Mouse World Position: (%.2f, %.2f)",  mouse_pos.x, mouse_pos.y);

#endif
		if(input->on_down(SDLK_ESCAPE))
		{
			running = false;
			break;
		}

		if(input->on_down(SDLK_z))
		{
			window.set_window_mode(ScreenMode::WINDOWED);
			renderer->force_color_clear();
		}
		else if(input->on_down(SDLK_c))
		{
			window.set_window_mode(ScreenMode::BORDERLESS);
			renderer->force_color_clear();
		}
		// TODO(cgenova): profiling;

	    TimeBeginFrame();

		// Update the scene first, pushing draw calls if necessary.
		// Then call begin_frame which builds matrices and clears buffers;

		if(CurrentTime() - last_fps_time > 1.0f)
		{
			last_fps_time = CurrentTime();
			fps = frame_count;
			frame_count = 0;
		}
		frame_count++;
		Console::get()->LogMessage("FPS: \t\t%d Frames: \t%d", fps, FrameCount());
		//Console::get()->log_message("Frames: \t%d",  + std::to_string(FrameCount()));

		// TODO(cgenova): separate update and render calls so that things can be set up when rendering begins;
		renderer->begin_frame();
		main_camera.Tick(CurrentTime());
		scene.update_scene();

        static std::vector<SimpleVertex> v;
        static bool initialized = false;
        if(!initialized)
        {
            initialized = true;
            for(uint32 i = 0; i < 200; ++i)
            {
                SimpleVertex verts = {};
                verts.position = vec2((float) (i / 50.f) - 2.f, (float) i);
                verts.color = vec4(1, 1, 0, 1.f);
                v.push_back(verts);
            }
        }
        else
        {
            for(uint32 i = 0; i < v.size(); ++i)
            {
				v[i].position.y = sin(CurrentTime() + i / (PI * 20));
            }
        }
        renderer->DrawLine(v, 3, DrawLayer_UI, LineDrawOptions::SMOOTH);
		//renderer->render_draw_buffer();

        for(int i = 0; i < 10; ++i)
        {
            Console::get()->LogMessage("this is a test of the console output performance");
        }

        ProfileBeginSection(Profile_RenderFinish);
		renderer->Flush();
        ProfileEndSection(Profile_RenderFinish);

        ProfileEndSection(Profile_Frame);
        ProfileEndFrame(renderer, TARGET_FPS);

        // TODO: what are the values for the renderer profiling if this is outputting already?
		Console::get()->draw();

        renderer->SwapBuffer();
		// End rendering


		// TODO(cgenova): High granularity sleep function!
		uint32 delay_time = RemainingTicksInFrame();
		if(delay_time > 10) {
			//std::cout << "Delaying: " << delay_time << " ms" << std::endl;
			SDL_Delay(delay_time);
		}
		//std::cout << "Delta T : " << delay_time << " ms" << std::endl;

	}// End main loop

	SDL_Quit();

	return 0;
}
