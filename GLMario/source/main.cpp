#include "SDL.h"

#include "stdio.h"

#include "window.h"
#include "renderer.h"
#include "input.h"
#include "scene_manager.h"

//TODO(chris):
//	Window class -> stores resolution information, has methods to change the resolution, in charge of SDL windowing
//  Camera class -> stores location of the camera
//  Renderer class -> stores 
// 	Time class -> uptime, frame time, 


int main(int argc, char* argv[])
{
	Window window("Title", 800, 600);

	Camera main_camera;
	main_camera.viewport_size = Vector2(16.f, 9.f);
	main_camera.screen_resolution = window.get_resolution();

	Renderer renderer(&window, Vector4(0, 0, 0, 1.0f));
	renderer.set_camera(&main_camera);

	Input input;

	SceneManager scene(&input, &renderer, &window, &main_camera);

	SDL_Event e;
	bool running = true;
	while(running)
	{
		input.begin_frame();
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
				input.process_key_press(e.key.keysym.sym);
			}
			if (e.type == SDL_KEYUP)
			{
				input.process_key_release(e.key.keysym.sym);
			}

			if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
			{
				input.mouse_button_event();
			}
			if(e.type == SDL_MOUSEMOTION)
			{
				input.update_mouse_position();
			}
		}
#endif
		if(input.on_down(SDLK_ESCAPE))
		{
			running = false;
		}


		// Rendering stuff

		if(input.on_down(SDLK_z))
		{
			window.set_window_mode(ScreenMode::WINDOWED);
		}
		else if(input.on_down(SDLK_c))
		{
			window.set_window_mode(ScreenMode::BORDERLESS);
		}

		renderer.begin_frame();

		scene.update_scene();

		renderer.end_frame();
		// End rendering


		// TODO(chris): Dynamic frame rate;
		SDL_Delay(1000 / 60);

	}// End main loop	

	SDL_Quit();

	return 0;	
}