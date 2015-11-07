#include "SDL.h"

#include <stdio.h>

#include "window.h"
#include "renderer.h"
#include "input.h"
#include "scene_manager.h"
#include "time.h"

#include "containers.h"

int main(int argc, char* argv[])
{
	assert(argc || argv[0]); // Fixes the compiler complaining about unused values;
	Window window("Title", 1400, 900);

	Time* time = Time::get(); // Initialize the static time object

	// main_camera.viewport_size = Vec2(16.f, 9.f);

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

        std::string mouse_world_pos("Mouse World Position: " + ::to_string(input->mouse_world_position()));
        Console::get()->log_message(mouse_world_pos);



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

		time->begin_frame();

		// Update the scene first, pushing draw calls if necessary.
		// Then call begin_frame which builds matrices and clears buffers;

		if(time->current_time - last_fps_time > 1.0f)
		{
			last_fps_time = time->current_time;
			fps = frame_count;
			frame_count = 0;
		}
		frame_count++;
		Console::get()->log_message(std::string("FPS: \t\t" + std::to_string(fps)));
		Console::get()->log_message(std::string("Frames:\t" + std::to_string(time->frame_count)));

		// TODO(cgenova): separate update and render calls so that things can be set up when rendering begins;
		renderer->begin_frame();
		main_camera.Tick((float)time->delta_time);
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
				v[i].position.y = sin((float)time->current_time + i / (PI * 20));
            }
        }
        renderer->DrawLine(v, DrawLayer_UI);
		 //renderer->render_draw_buffer();


        ProfileBeginSection(Profile_Console);
		Console::get()->draw();
        ProfileEndSection(Profile_Console);

        ProfileBeginSection(Profile_RenderFinish);
		renderer->end_frame();
        ProfileEndSection(Profile_RenderFinish);
		// End rendering

        ProfileEndFrame();

		// TODO(cgenova): High granularity sleep function!
		uint32 delay_time = time->ticks_for_frame_cap();
		if(delay_time > 5) {
			//std::cout << "Delaying: " << delay_time << " ms" << std::endl;
			SDL_Delay(delay_time);
		}
		//std::cout << "Delta T : " << delay_time << " ms" << std::endl;

	}// End main loop

	SDL_Quit();

	return 0;
}
