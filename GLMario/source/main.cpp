
// TODO: REMOVE
#define _CRT_SECURE_NO_WARNINGS

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
#include "entity.h"
#include "particles.h"
#include "input.h"
#include "console.h"
#include "audio.h"
#include "tilemap.h"
#include "game_types.h"
#include "entity.h"

#include "Editor.h"

#include "audio.h"

#include "platform/platform.h"

// Should be like 16.66666
#define MS_PER_FRAME 16
#define TARGET_FPS (1000 / MS_PER_FRAME)

#define FRAME_TEMPORARY_MEMORY_SIZE MEGABYTES(256)
#define GAME_PERMANENT_MEMORY_SIZE  MEGABYTES(256)

#define TOTAL_MEMORY_ALLOCATION_SIZE FRAME_TEMPORARY_MEMORY_SIZE + GAME_PERMANENT_MEMORY_SIZE

#define MAX_GAME_ENTITES 4096 * 2

void DebugControlCamera(GameState* game_state, Camera* camera)
{
    if(IsDown(game_state->input, KeyCode_LEFT))
    {
        camera->position.x -= 0.1f;
    }

    if(IsDown(game_state->input, KeyCode_RIGHT))
    {
        camera->position.x += 0.1f;
    }

    if(IsDown(game_state->input, KeyCode_UP))
    {
        if (IsDown(game_state->input, KeyCode_CONTROL))
        {
            camera->position.y += 0.1f;
        }
        else
        {
            camera->viewport_size *= 1.1f;
        }
    }

    if(IsDown(game_state->input, KeyCode_DOWN))
    {
        if (IsDown(game_state->input, KeyCode_CONTROL))
        {
            camera->position.y -= 0.1f;
        }
        else
        {
            camera->viewport_size *= 0.9f;
        }
    }
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

    result->input = PushStruct(&result->permanent_memory, NewInput);
    result->renderer = CreateRenderer(&result->permanent_memory);
    InitializeTime(result, MS_PER_FRAME);
    return result->renderer ? result : nullptr;
}

static Scene* PushScene(MemoryArena* arena, uint32 num_entities)
{
    Scene* result = PushStruct(arena, Scene);

    result->max_entities = num_entities;
    result->entities = PushArray(arena, Entity, num_entities);

    // This should be done when loading a level
    // This needs to subarena for a quad tree
    result->tilemap = PushStruct(arena, TileMap);

    Vec2 map_size = { 100.f, 100.f };
    uint32 max_tiles = 1000;
    AllocateTileMap(arena, result->tilemap, map_size, max_tiles);

    return result;
}

// int main(int argc, char* argv[])
int GameMain()
{
//    assert(argc || argv[0]); // Fixes the compiler complaining about unused values;

    GameState* game_state = CreateNewGameState("EnGen", 1600, 900);
    Renderer* renderer = game_state->renderer;
    game_state->active_scene = PushScene(&game_state->permanent_memory, MAX_GAME_ENTITES);

    TileMap* tilemap = game_state->active_scene->tilemap;

    for (int32 i = 0; i < 10; ++i)
    {
        Vec2 pos = { (float)i, 2.f };
        AddTileToMap(tilemap, pos);
    }

    for (int32 i = 0; i < 10; ++i)
    {
        Vec2 pos = { 0, (float)i };
        AddTileToMap(tilemap, pos);
    }

    for (int32 i = 0; i < 10; ++i)
    {
        Vec2 pos = { 10.f, (float)i };
        AddTileToMap(tilemap, pos);
    }

    UIWindow* ui = PushStruct(&game_state->permanent_memory, UIWindow);
    SetTitle(ui, "Editor UI!");

    SetSize(ui, { 0.1f, 0.3f, 0.2f, 0.2f }, 0.05f);

    UIWindow* ui2 = PushStruct(&game_state->permanent_memory, UIWindow);
    SetTitle(ui2, "Editor UI2!");

    SetSize(ui2, { 0.2f, 0.3f, 0.2f, 0.2f }, 0.05f);

    InitializeDebugConsole();

#if 0
    InitializeAudio();
    char* test_sound_file = "C:\\projects\\imperial_march.wav";
    bool test_sound_loaded = LoadWavFile(test_sound_file);
    if(test_sound_loaded)
    {
        printf("Loaded File\n");
    }
    PauseAudio(false);
#endif

    Camera default_camera = {}; // maybe put this in game_state?
    default_camera.position = vec2(0, 0);
    default_camera.viewport_size.x = 16;
    default_camera.viewport_size.y = 9;

    uint32 frame_count = 0;
    uint32 fps = 0;
    double last_fps_time = 0;

    bool running = true;

    while (running)
    {
        ProfileBeginFrame();
        ProfileBeginSection(Profile_Frame);
        ProfileBeginSection(Profile_Input);

        Platform_RunMessageLoop(game_state->input);

        Camera* draw_camera = game_state->active_camera ? game_state->active_camera : &default_camera;

        game_state->window.resolution = Platform_GetResolution();
        UpdateMouseWorldPosition(game_state->input, game_state->window.resolution, draw_camera->viewport_size, draw_camera->position);

        ProfileEndSection(Profile_Input);

        Vec2i mouse_pos = MousePosition(game_state->input);
        //DebugPrintf("Mouse World Position: (%.2f, %.2f)", mouse_pos.x, mouse_pos.y);
        DebugPrintf("Mouse World Position: (%d, %d)", mouse_pos.x, mouse_pos.y);
        DebugPrintf("Main Camera Position: (%.2f, %.2f)", default_camera.position.x, default_camera.position.y);


        DebugPrintf("Key Pressed: %s", IsDown(game_state->input, KeyCode_a) ? "TRUE" : "FALSE");

        if (OnDown(game_state->input, KeyCode_ESCAPE))
        {
            running = false;
            break;
        }

#if 0 // TODO: Platform layer
        if (OnDown(game_state->input, KeyCode_z))
        {
            ForceColorClear();
            SwapBuffer(game_state);
            //WindowSetScreenMode(&game_state->window, ScreenMode_Windowed);
        }
        else if (OnDown(game_state->input, KeyCode_c))
        {
            ForceColorClear();
            SwapBuffer(game_state);
            //WindowSetScreenMode(&game_state->window, ScreenMode_Borderless);
        }
#endif

        static bool draw_debug = true;
        if (OnDown(game_state->input, KeyCode_BACKQUOTE))
        {
            draw_debug = !draw_debug;
        }

        Renderer* debug_renderer = draw_debug ? renderer : 0;

        TimeBeginFrame(game_state);

        // Update the scene first, pushing draw calls if necessary.
        // Then call begin_frame which builds matrices and clears buffers;
        float current_time = CurrentTime(game_state);
        if (current_time - last_fps_time > 1.0f)
        {
            last_fps_time = current_time;
            fps = frame_count;
            frame_count = 0;
        }
        frame_count++;
        DebugPrintf("FPS: \t\t%d \tFrames: \t%d", fps, FrameCount(game_state));

        DebugControlCamera(game_state, &default_camera);

        // TODO(cgenova): separate update and render calls so that things can be set up when rendering begins;
        BeginFrame(renderer, &game_state->window);

        ProfileBeginSection(Profile_SceneUpdate);

        DebugPrintPushColor(vec4(1.0f, 0, 0, 1.0f));
        DebugPrintf("Active scene entity usage: (%d / %d)", game_state->active_scene->active_entities, MAX_GAME_ENTITES);
        DebugPrintPopColor();

        UpdateSceneEntities(game_state, game_state->active_scene);
        DrawSceneEntities(game_state->active_scene, renderer);

        ProfileEndSection(Profile_SceneUpdate);

#if 1 // Spaghetti test
        const size_t num_verts = 200;
        static SimpleVertex v[num_verts];
        static bool initialized = false;
        if (!initialized)
        {
            initialized = true;
            for (uint32 i = 0; i < num_verts; ++i)
            {
                SimpleVertex verts = {};
                verts.position = vec2((float)(i / 50.f) - 2.f, (float)i);
                verts.color = vec4(1, 1, 0, 1.f);
                v[i] = verts;
            }
        }
        else
        {
            for (uint32 i = 0; i < num_verts; ++i)
            {
                v[i].position.y = sin(CurrentTime(game_state) + i / (PI * 20));
            }
        }

        PrimitiveDrawParams spaghetti_params = {};
        spaghetti_params.line_draw_flags |= PrimitiveDraw_Smooth;
        //      spaghetti_params.line_draw_flags |= Draw_ScreenSpace;
        spaghetti_params.line_width = 0;
        DrawLine(renderer, v, num_verts, spaghetti_params);
#endif

        DrawTileMap(game_state, game_state->active_scene->tilemap);

        UpdateUIWindow(game_state, ui);
        UpdateUIWindow(game_state, ui2);

        RenderDrawBuffer(renderer, draw_camera);

        ProfileEndSection(Profile_Frame);
        ProfileEndFrame(debug_renderer, TARGET_FPS);
        DebugDrawConsole(debug_renderer);

        // NOTE:
        // For drawing Debug info, the profiling in this section will be discarded,
        // but it is only drawing text and the debug graph.
        RenderDrawBuffer(renderer, draw_camera);

        SwapBuffer(game_state);

        // TODO(cgenova): High granularity sleep function!

        ResetArena(&game_state->temporary_memory);

    }// End main loop

    return 1;
}

