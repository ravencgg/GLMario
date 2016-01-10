
// TODO: REMOVE
#define _CRT_SECURE_NO_WARNINGS

#include "SDL.h"
#include <string>
#include "types.h"
#include "mathops.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h> // For 64 bit int printf output
#include <random>

#include "time.h"
#include "renderer.h"
#include "physics.h"
#include "entity.h"
#include "particles.h"
#include "input.h"
#include "console.h"
#include "audio.h"
#include "tilemap.h"
#include "game_types.h"
#include "entity.h"

#include "audio.h"
#include "platform.h"


// Should be like 16.66666
#define MS_PER_FRAME 16
#define TARGET_FPS (1000 / MS_PER_FRAME)

#define FRAME_TEMPORARY_MEMORY_SIZE MEGABYTES(256)
#define GAME_PERMANENT_MEMORY_SIZE  MEGABYTES(256)

#define TOTAL_MEMORY_ALLOCATION_SIZE FRAME_TEMPORARY_MEMORY_SIZE + GAME_PERMANENT_MEMORY_SIZE

#define MAX_GAME_ENTITES 5000
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
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        printf("SDL init error\n");
    }

    window->resolution.x = width;
    window->resolution.y = height;
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
    if(window->sdl_window)     SDL_DestroyWindow(window->sdl_window);
}

void WindowSetResolution(Window* window, uint32 w, uint32 h)
{
    window->resolution.x = w;
    window->resolution.y = h;
    SDL_SetWindowSize(window->sdl_window, w, h);
}

void WindowSetScreenMode(Window* window, ScreenMode mode)
{
    window->current_mode = mode;
    SDL_SetWindowFullscreen(window->sdl_window, (uint32)mode);
}

static GameState* CreateNewGameState(char* window_title, int res_x, int res_y)
{
    MemoryArena arena;
    // NOTE: this is the allocation for the game, store this somewhere if this should be freed manually at some point
    AllocateMemoryArena(&arena, sizeof(GameState) + GAME_PERMANENT_MEMORY_SIZE + FRAME_TEMPORARY_MEMORY_SIZE);

    GameState* result = PushStruct(&arena, GameState);
    result->temporary_memory = CreateSubArena(&arena, FRAME_TEMPORARY_MEMORY_SIZE);
    result->permanent_memory = CreateSubArena(&arena, GAME_PERMANENT_MEMORY_SIZE);
    assert(arena.used == arena.size);

    StartupWindow(&result->window, window_title, res_x, res_y); 

    result->renderer = CreateRenderer(&result->permanent_memory);
    InitializeTime(result, MS_PER_FRAME);
    return result;
}

static Scene* PushScene(MemoryArena* arena, uint32 num_entities, uint32 num_objects)
{
    Scene* result = PushStruct(arena, Scene);

    result->max_entities = num_entities;
    result->entities = PushArray(arena, Entity, num_entities);

    BuildEntityVTable(result);

    // This should be done when loading a level
    // This needs to subarena for a quad tree
    const uint32 width = 50; // LOAD FROM FILE!
    const uint32 height = 50;
    AllocateTileMap(arena, &result->tilemap, width, height);

    return result;
}

int main(int argc, char* argv[])
{
    assert(argc || argv[0]); // Fixes the compiler complaining about unused values;

    GameState* game_state = CreateNewGameState("EnGen", 1200, 700);
    Renderer* renderer = game_state->renderer;
    game_state->active_scene = PushScene(&game_state->permanent_memory, MAX_GAME_ENTITES, MAX_GAME_OBJECTS);

// Global initialization
    InitializeInput();
    InitializeDebugConsole();
//    InitializeAudio();
// End Global initialization

    game_state->active_scene->physics = new Physics;
    // TODO: dynamically figure out max sizes of things like this
    game_state->active_scene->physics->quadtree_memory = CreateSubArena(&game_state->permanent_memory, MEGABYTES(1));
    game_state->active_scene->physics->quadtree.aabb = { -100.f, -100.f, 200.f, 200.f };
    game_state->active_scene->tmap = new Tilemap(game_state->active_scene->physics);

#if 0
    game_state->active_scene->tmap->MakeWalledRoom(rect(-50, -20, 50, 50));
    game_state->active_scene->tmap->MakeWalledRoom(rect(-5, -20, 30, 3));
    game_state->active_scene->tmap->MakeWalledRoom(rect(-25, -10, 5, 10));
    game_state->active_scene->tmap->MakeWalledRoom(rect(20, 20, 10, 10));
    game_state->active_scene->tmap->MakeWalledRoom(rect(-5, -2, 10, 2));
    game_state->active_scene->tmap->MakeWalledRoom(rect(-5, -2, 4, 4));
#else
    game_state->active_scene->tmap->AddTile(-1.5f, -1.f);
    game_state->active_scene->tmap->AddTile(-3.f, -1.f);

    game_state->active_scene->tmap->AddTile(2.f, -1.f);
#endif

#if 0
    char* test_sound_file = "C:\\projects\\imperial_march.wav";
    bool test_sound_loaded = LoadWavFile(test_sound_file);
    if(test_sound_loaded)
    {
        printf("Loaded File\n");
    }
    PauseAudio(false);
#endif

//  SceneManager scene;
    Camera main_camera = {}; // maybe put this in game_state?
    main_camera.position = vec2(0, 0);
    main_camera.viewport_size.x = 16;
    main_camera.viewport_size.y = 9;

    renderer->camera = &main_camera;
//    renderer->set_camera(&main_camera);

    uint32 frame_count = 0;
    uint32 fps = 0;
    double last_fps_time = 0;

    bool running = true;

    while(running)
    {
        SDL_Event e;
        ProfileBeginFrame();

        ProfileBeginSection(Profile_Frame);
        ProfileBeginSection(Profile_Input);

        InputBeginFrame();
        while (SDL_PollEvent(&e))
        {
            switch(e.type)
            {

            case SDL_QUIT:
            {
                running = false;
            }break;

            case SDL_KEYDOWN:
            {
                ProcessKeyPress(e.key.keysym.sym);
            }break;

            case SDL_KEYUP:
            {
                ProcessKeyRelease(e.key.keysym.sym);
            }break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                MouseButtonEvent();
            }break;

            case SDL_MOUSEMOTION:
            {
                UpdateMousePosition();
            }break;


                case SDL_WINDOWEVENT:
                {
                    switch(e.window.event)
                    {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        game_state->window.resolution.x = e.window.data1;
                        game_state->window.resolution.y = e.window.data2;
                    }break;

                    }
                }
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

        if(KeyFrameDown(SDLK_F1))
        if(KeyFrameDown(SDLK_F1))
        {
            running = false;
            break;
        }

        if(KeyFrameDown(SDLK_z))
        {
            ForceColorClear();
            SwapBuffer(game_state);
            WindowSetScreenMode(&game_state->window, ScreenMode_Windowed);
        }
        else if(KeyFrameDown(SDLK_c))
        {
            ForceColorClear();
            SwapBuffer(game_state);
            WindowSetScreenMode(&game_state->window, ScreenMode_Borderless);
        }

        static bool draw_debug = true;
        if(KeyFrameDown(SDLK_BACKQUOTE))
        {
            draw_debug = !draw_debug;
        }

        Renderer* debug_renderer = draw_debug ? renderer : 0;

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
        BeginFrame(renderer, &game_state->window);

        game_state->active_scene->tmap->update();
        game_state->active_scene->tmap->draw(game_state); // TODO: lol

        ProfileBeginSection(Profile_SceneUpdate);

        DebugPrintPushColor(vec4(1.0f, 0, 0, 1.0f));
        DebugPrintf("Active scene entity usage: (%d / %d)", game_state->active_scene->active_entities, MAX_GAME_ENTITES);
        DebugPrintPopColor();

        UpdateSceneEntities(game_state, game_state->active_scene);
        DrawSceneEntities(game_state->active_scene, renderer);

        game_state->active_scene->physics->DebugDraw(game_state); // TODO: lol

        ProfileEndSection(Profile_SceneUpdate);

#if 1 // Spaghetti test
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

        LineDrawParams spaghetti_params;
        spaghetti_params.line_draw_flags |= LineDraw_Smooth;
        DrawLine(renderer, v, &spaghetti_params);
#endif

#if 1 // Rotation test
        static Rectf rot_rect = { -15.f, 0, 30.f, 1.f };
        static Rectf rot_rect2 = { 2.f, 1.f, 1.f, 3.f };

        static Array<SimpleVertex> rot_array(8);
        static bool rot_init = false;
        static float rot_angle = 0;
        static float rot_angle2 = 0;
        static uint32 num_elements = 8;
        if(!rot_init)
        {
            rot_init = true;
            for(uint32 i = 0; i < num_elements; ++i)
            {
                SimpleVertex verts = {};
                verts.color = vec4(1, 1, (float) i / (float) num_elements, 1.f);
                rot_array.Add(verts);
            }
        }

        DebugPrintf("Angle 1: %.2f, Angle2: %.2f", rot_angle, rot_angle2);

        if(KeyFrameDown(SDLK_1))
        {
            rot_rect = { 2.f, 1.f, 3.f, 3.f };
            rot_rect2 = { 2.f, 1.f, 1.f, 3.f };
            rot_angle = 0;
            rot_angle2 = 0;
        }

        if(KeyIsDown(SDLK_6))
        {
            rot_angle += 0.02f;
        }
        if(KeyIsDown(SDLK_7))
        {
            rot_angle -= 0.02f;
        }

        if(KeyIsDown(SDLK_9))
        {
            rot_angle2 += 0.02f;
        }
        if(KeyIsDown(SDLK_0))
        {
            rot_angle2 -= 0.02f;
        }

        if(rot_angle > TAU)
        {
            rot_angle -= TAU;
            rot_rect2.w = random_float(1.f, 4.f);
            rot_rect2.h = random_float(1.f, 4.f);
        }

        //Vec2_4 rot_points = RotatedRect(rot_rect, rot_angle);

        Rectf aabb;
        Vec2_8 rot_sum = MinkowskiSum(rot_rect, rot_angle, rot_rect2, rot_angle2, &aabb);
        DrawRect(renderer, aabb, vec4(0, 1.f, 1.f, 1.f));

        Vec2 vpoint = { 1.f, 1.f };

//        static float point_theta = 0;
//        point_theta += 0.01f;
//        vpoint = RotatePoint(vpoint, point_theta, vec2(0, 0));
        Rectf point = { vpoint.x, vpoint.y, 0.1f, 0.1f };
        Vec4 point_color = vec4(0.2f, 0.2f, 0.8f, 1.0f);
        if(Contains(rot_rect, rot_angle, vpoint))
        {
            point_color = vec4(1.f, 0, 0, 1.f);
        }

        DrawRect(renderer, point, point_color);

#if 0
        for(uint32 i = 0; i < 4; ++i)
        {
            rot_array[i].position = rot_points.e[i];
        }
#endif

        for(uint32 i = 0; i < 8; ++i)
        {
            rot_array[i].position = rot_sum.e[i];
        }

        LineDrawParams rot_params;
        rot_params.line_draw_flags |= LineDraw_Looped;
        DrawLine(renderer, rot_array, &rot_params);

        for(uint32 i = 0; i < rot_array.Size(); ++i)
        {
            DebugPrintf("Point %d: is (%.1f,%.1f)", i, rot_array[i].position.x, rot_array[i].position.y);
        }

        DrawRotatedRect(renderer, rot_rect, rot_angle, vec4(0.5f, 0.5f, 0.5f, 0.5f));
        DrawRotatedRect(renderer, rot_rect2, rot_angle2, vec4(0.8f, 0.8f, 0.8f, 1.f));
#endif

        //DrawTileMap(&game_state->active_scene->tilemap);

        Flush(renderer);

        ProfileEndSection(Profile_Frame);
        ProfileEndFrame(debug_renderer, TARGET_FPS);
        DebugDrawConsole(debug_renderer);

        // For drawing Debug info, the profiling in this section will be discarded,
        // but it is only drawing text and the debug graph.
        Flush(renderer);

        SwapBuffer(game_state);

        // TODO(cgenova): High granularity sleep function!
        //uint32 delay_time = RemainingTicksInFrame();
        //if(delay_time > 10) {
          //std::cout << "Delaying: " << delay_time << " ms" << std::endl;
        //  SDL_Delay(delay_time);
        //}
        //std::cout << "Delta T : " << delay_time << " ms" << std::endl;
        //

        ResetArena(&game_state->temporary_memory);

    }// End main loop

    SDL_Quit();

    return 1;
}

