#include "SDL.h"

#include <stdio.h>

#include "renderer.h"
#include "input.h"
#include "scene_manager.h"
#include "time.h"
#include "game_types.h"
#include "entity.h"

#include <inttypes.h> // For 64 bit int printf output

#include "containers.h"

// Should be like 16.66666
#define MS_PER_FRAME 16
#define TARGET_FPS (1000 / MS_PER_FRAME)

#define FRAME_TEMPORARY_MEMORY_SIZE MEGABYTES(256)
#define GAME_PERMANENT_MEMORY_SIZE  MEGABYTES(128)

#define MAX_GAME_ENTITES 500
#define MAX_GAME_OBJECTS 500

void DebugControlCamera(Camera* camera)
{
	if(KeyIsDown(SDLK_LEFT))
	{
		camera->position.x -= 0.1f;
	}

	if(KeyIsDown(SDLK_RIGHT))
	{
		camera->position.x += 0.1f;
	}

    if(KeyIsDown(SDLK_UP))
    {
        if(KeyIsDown(SDLK_RCTRL) || KeyIsDown(SDLK_LCTRL))
        {
            camera->position.y += 0.1f;
        }
        else
        {
            camera->viewport_size *= 1.1f;
        }
    }

    if(KeyIsDown(SDLK_DOWN))
    {
        if(KeyIsDown(SDLK_RCTRL) || KeyIsDown(SDLK_LCTRL))
        {
            camera->position.y -= 0.1f;
        }
        else
        {
            camera->viewport_size *= 0.9f;
        }
    }
}

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

static GameState* CreateNewGameState(char* window_title, int res_x, int res_y)
{
    // TODO: allocate the permanent memory first and place a GameState at the front of it?
    GameState* result = new GameState;

    AllocateMemoryArena(&result->temporary_memory, FRAME_TEMPORARY_MEMORY_SIZE);
    AllocateMemoryArena(&result->permanent_memory, GAME_PERMANENT_MEMORY_SIZE);
    StartupWindow(&result->window, window_title, res_x, res_y);
    InitializeTime(result, MS_PER_FRAME);
    return result;
}

static Scene* PushScene(MemoryArena* arena, uint32 num_entities, uint32 num_objects)
{
    Scene* result = PushStruct(arena, Scene);

    result->max_entities = num_entities;
    result->max_objects  = num_objects;
    result->entities = PushStructs(arena, GameEntity, num_entities);
    result->objects  = PushStructs(arena, GameObject, num_objects);

    // This should be done when loading a level

    const uint32 width = 50; // LOAD FROM FILE!
    const uint32 height = 50;

    AllocateTileMap(arena, &result->tilemap, width, height);

    return result;
}

int main(int argc, char* argv[])
{
	assert(argc || argv[0]); // Fixes the compiler complaining about unused values;

    GameState* game_state = CreateNewGameState("EnGen", 1200, 700);
    game_state->active_scene = PushScene(&game_state->permanent_memory, MAX_GAME_ENTITES, MAX_GAME_OBJECTS);

// Global initialization
	Renderer::create_instance(&game_state->window);
    InitializeInput();
    InitializeDebugConsole();
// End Global initialization

// TODO: no more singletons!
	Renderer* renderer = Renderer::get();

//	SceneManager scene;
	Camera main_camera = {}; // maybe put this in game_state?
    main_camera.position = vec2(0, 0);
    main_camera.viewport_size.x = 16;
    main_camera.viewport_size.y = 9;

//	scene.SetMainCamera(&main_camera);
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

		InputBeginFrame();
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
				ProcessKeyPress(e.key.keysym.sym);
			}
			if (e.type == SDL_KEYUP)
			{
				ProcessKeyRelease(e.key.keysym.sym);
			}

			if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
			{
				MouseButtonEvent();
			}
			if(e.type == SDL_MOUSEMOTION)
			{
				UpdateMousePosition();
			}
		}

        UpdateMouseWorldPosition(game_state->window.resolution, main_camera.viewport_size, main_camera.position);

        ProfileEndSection(Profile_Input);

        Vec2 mouse_pos = MouseWorldPosition();
        DebugPrintf("Mouse World Position: (%.2f, %.2f)",  mouse_pos.x, mouse_pos.y);
        DebugPrintf("Main Camera Position: (%.2f, %.2f)",  main_camera.position.x, main_camera.position.y);

		if(KeyFrameDown(SDLK_ESCAPE))
		{
			running = false;
			break;
		}

		if(KeyFrameDown(SDLK_z))
		{
			WindowSetScreenMode(&game_state->window, ScreenMode_Windowed);
			renderer->force_color_clear(); // TODO: useless if it doesn't swap the buffers
                                            // but that may look bad with vsync
		}
		else if(KeyFrameDown(SDLK_c))
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
		DebugPrintf("FPS: \t\t%d \tFrames: \t%d", fps, FrameCount(game_state));

        DebugControlCamera(&main_camera);

		// TODO(cgenova): separate update and render calls so that things can be set up when rendering begins;
		renderer->begin_frame();
//		main_camera.Tick(game_state);

        ProfileBeginSection(Profile_SceneUpdate);
//		scene.update_scene(game_state);
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

        DrawTileMap(&game_state->active_scene->tilemap);

		renderer->Flush();

        ProfileEndSection(Profile_Frame);
        ProfileEndFrame(renderer, TARGET_FPS);

//        debugprintf("swap time: %"priu64"", post_swap_time - pre_swap_time);
        DebugDrawConsole(renderer);

        uint64 pre_swap_time = GetCycleCount();
        SwapBuffer(game_state);
        uint64 post_swap_time = GetCycleCount();

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

