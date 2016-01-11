#include "input.h"
#include "entity.h"
#include "game_types.h"
#include "particles.h"
#include "console.h"

#include "renderer.h"
// Move this
void RenderRandomParticles(GameState* game_state)
{
#if 1
	static ParticleSystem ps1(game_state, 10000);
	ps1.draw_layer = DrawLayer_PreTilemap;
	//ps1.draw_layer = DrawLayer::POST_TILEMAP;
	static ParticleEmissionData data[2];
	static uint32 active_data = 0;

	static ParticleSystem ps2(game_state, 7500);
	ps2.draw_layer = DrawLayer_PreTilemap;
	static bool setup = false;
	if(!setup)
	{
		setup = true;

		ps1.ped.spawn_position.x = -2.f;
		ps1.ped.spawn_rate = 1000;
		ps1.ped.spawn_size.x = 1.f;
		ps1.ped.spawn_size.y = 1.f;
		ps1.ped.lifetime.min_range = 9.0f;
		ps1.ped.lifetime.max_range = 10.f;
		ps1.ptd.gravity = vec2(-10.0f, 1.0f);

		ps2.ptd.gravity = vec2(1.0f, 0.3f);
		ps2.ped.spawn_size.x = 1.f;
		ps2.ped.spawn_size.y = 1.f;
		ps2.ped.spawn_rate = 1000;
		ps2.ped.spawn_position.x = 2.f;
		ps2.ped.start_color = vec4(0, 1.f, 0, 1.f);
		ps2.ped.end_color   = vec4(0.2f, 0.8f, 0.2f, 0.4f);

		data[0] = ps2.ped;
		data[1] = ps2.ped;
		data[0].start_color = vec4(1, 0, 0, 1);
		data[0].end_color   = vec4(0, 1, 0, 0.5f);
		data[1].start_color = vec4(0, 1, 0, 1);
		data[1].end_color   = vec4(1, 1, 0, 0);
	}

	ps2.ped.start_color = vec4((float)sin(CurrentTime(game_state)), (float)cos(CurrentTime(game_state)), 1, 1.f);
	ps2.ped.end_color = vec4(0.2f, sin(CurrentTime(game_state)), cos(CurrentTime(game_state)), 0.4f);

	static Timer timer = (StartTimer(game_state, &timer, 1.0f), timer);
	if (TimerIsFinished(game_state, &timer))
	{
		StartTimer(game_state, &timer);
		active_data ^= 1;
		ps2.ped = data[active_data];
	}

	ps1.update(game_state);
	ps1.render();
	ps2.update(game_state);
	ps2.render();
#endif
}

EntityUpdateFunc(UpdatePlayer)
{
    EntityPlayer* player = &entity->player;

//    const float gravity = -20.f;
    static uint32 count = 0;

    if(KeyIsDown(SDLK_SPACE))
    {
        //RemoveEntity(entity);
        player->velocity.y = 10.f;
    }
    else
    {
//        player->velocity.y += gravity * dt;
        player->velocity.y = max(player->velocity.y, -100.0f);
    }
    if(KeyIsDown(SDLK_d))
    {
        player->velocity.x += 50.f * dt;
        player->velocity.x = min(player->velocity.x, 5.0f);
    }
    else if(KeyIsDown(SDLK_a))
    {
        player->velocity.x -= 50.f * dt;
        player->velocity.x = max(player->velocity.x, -5.0f);
    }
    else
    {
        player->velocity.x = 0;
    }
    if(KeyIsDown(SDLK_1))
    {
        entity->delete_this_frame = true;
    }

    entity->transform.position += player->velocity * dt;

    Vec2 old_velocity = player->velocity;
}

EntityUpdateFunc(UpdateSpawner)
{
    EntitySpawner* spawner = &entity->spawner;

    float current_time = CurrentTime(game_state);

    if (current_time - spawner->last_spawn_time > spawner->time_between_spawns)
    {
        Vec2 spawn_pos = entity->transform.position;
        if(spawner->num_spawned > 0)
        {
            spawn_pos.y += 1.0f * (float) spawner->num_spawned;
            SpawnEntity(game_state, scene, EntityType_Enemy, spawn_pos);
        }
        else
        {
            spawn_pos.x -= 1.0f;
            Entity* new_spawner = SpawnEntity(game_state, scene, EntityType_Spawner, spawn_pos);
            new_spawner->spawner.last_spawn_time = current_time;
        }

        ++spawner->num_spawned;
        spawner->last_spawn_time = current_time;
    }
}

EntityUpdateFunc(UpdateEnemy)
{
    EntityEnemy* enemy = &entity->enemy;

    if(TimerIsFinished(game_state, &enemy->despawn_timer))
    {
        RemoveEntity(scene, entity);
    }
}

EntityUpdateFunc(UpdateCamera)
{
    EntityCamera* camera = &entity->camera;

    Entity* follow_target;
    if(FindEntityWithID(scene, camera->follow_target_id, &follow_target))
    {
        entity->transform.position = follow_target->transform.position;
        entity->camera.camera.position = follow_target->transform.position;
    }
    else
    {
        camera->follow_target_id = 0;
    }
}

EntityDrawFunc(DrawEnemy)
{
    DrawCall draw_call = {};
    draw_call.draw_type = DrawType::SINGLE_SPRITE;
    draw_call.image = ImageFiles::MARIO_IMAGE;
    draw_call.shader = Shader_Default;
    draw_call.options = DrawOptions::TEXTURE_RECT;
    draw_call.sd.tex_rect = { 17, 903, 34, 34 };
    draw_call.sd.world_size = vec2(1.0f, 1.5f);
    draw_call.sd.draw_angle = 0;

    draw_call.sd.world_position = entity->transform.position;
    PushDrawCall(renderer, draw_call, DrawLayer_Player);
}

EntityDrawFunc(DrawPlayer)
{
    DrawCall draw_call = {};
    draw_call.draw_type = DrawType::SINGLE_SPRITE;
    draw_call.image = ImageFiles::MARIO_IMAGE;
    draw_call.shader = Shader_Default;
    draw_call.options = DrawOptions::TEXTURE_RECT;
    draw_call.sd.tex_rect = { 17, 903, 34, 34 };
    draw_call.sd.world_size = vec2(1.0f, 1.5f);
    draw_call.sd.draw_angle = 0;
    draw_call.sd.world_position = entity->transform.position;

    PushDrawCall(renderer, draw_call, DrawLayer_Player);
}

EntityDrawFunc(DrawSpawner)
{
    DrawCall draw_call = {};
    draw_call.draw_type = DrawType::SINGLE_SPRITE;
    draw_call.image = ImageFiles::MARIO_IMAGE;
    draw_call.shader = Shader_Default;
    draw_call.options = DrawOptions::TEXTURE_RECT;
    draw_call.sd.tex_rect = { 34, 903, 34, 34 };
    draw_call.sd.world_size = vec2(1.0f, 1.5f);
    draw_call.sd.draw_angle = 0;
    draw_call.sd.world_position = entity->transform.position;

    PushDrawCall(renderer, draw_call, DrawLayer_Player);
}


bool FindEntityWithID(Scene* scene, uint32 id, Entity** out)
{
    if(id == 0) return false;

    bool result = false;

    Entity* entity = scene->entities;
    for(uint32 i = 0; i < scene->max_entities; ++i, ++entity)
    {
        if(entity->id == id)
        {
            result = true;
            *out = entity;
            break;
        }
    }
    return result;
}

static void DespawnRemovedEntities(Scene* scene)
{
    for(uint32 i = 0; i < scene->entity_delete_count; ++i)
    {
        Entity* entity = scene->entity_delete_list[i];
        assert(entity);
        if(entity)
        {

            switch(entity->type)
            {
                case EntityType_Player:
                    {
                    }break;
            }
            --scene->active_entities;
            // This resets EntityType to EntityType_Null!
            memset(entity, 0, sizeof(*entity));
        }
    }
}

static Entity* NextFreeEntitySlot(Scene* scene, EntityType new_type)
{
    Entity* entity = scene->entities;
    uint32 counter = 0;

    for(uint32 i = 0; i < scene->max_entities; ++i)
    {
        if(entity->type != EntityType_Null)
        {
            entity++;
            continue;
        }
        else
        {
            entity->id = ++scene->next_entity_id;
            entity->type = new_type;
            assert(entity->id != 0); // NOTE: handle wrapping?
            entity->flags |= EntityFlag_Enabled;

            ++scene->active_entities;
            return entity;
        }
    }

    assert(!"Ran out of entity storage space");
    return 0;
}

EntitySpawnFunc(SpawnEnemy)
{
    assert(entity);

    EntityEnemy* enemy = &entity->enemy;

    entity->transform.position = position;
    entity->transform.rotation = 0.f;
    entity->transform.scale = vec2( 1.f, 1.f );

    // TODO: don't have this require a game_state?
    StartTimer(game_state, &enemy->despawn_timer, random_float(1.f, 1.1f));

    enemy->health = 3;
}

EntitySpawnFunc(SpawnPlayer)
{
    EntityPlayer* player = &entity->player;

    if(scene->player_id == 0)
    {
        scene->player_id = entity->id;
    }

    entity->transform.position = position;
    entity->transform.rotation = 0.f;
    entity->transform.scale = vec2( 1.f, 1.f );

    Vec2 size = vec2(1.0f, 1.5f);
}

EntitySpawnFunc(SpawnEnemySpawner)
{
    EntitySpawner* spawner = &entity->spawner;

    spawner->last_spawn_time = 0;
    spawner->time_between_spawns = 2.0f;

    entity->transform.position = position;
}

Entity* SpawnEntity(GameState* game_state, Scene* scene, EntityType type, Vec2 position)
{
    Entity* result = NextFreeEntitySlot(scene, type);

    if(result && scene->entity_vtable[type].spawn)
    {
        scene->entity_vtable[type].spawn(game_state, scene, result, position);
    }

    return result;
}

// TODO: despawn entity by ID
void RemoveEntity(Scene* scene, Entity* entity)
{
    assert(scene->entity_delete_list);
    entity->flags |= EntityFlag_Removing;
    scene->entity_delete_list[scene->entity_delete_count++] = entity;
}

void UpdateSceneEntities(GameState* game_state, Scene* scene)
{
    float dt = FrameTime(game_state);
// Entities need to be tightly packed?
// Split entities into smaller groups and update each group?

    RenderRandomParticles(game_state);
    // NOTE: random scene test code
    {
        if (KeyFrameDown(SDLK_t))
        {
            SpawnEntity(game_state, scene, EntityType_Spawner, { 5.f, -1.0f });
        }

        if (KeyFrameDown(SDLK_m))
        {
            for(int i = 0; i < 1; ++i)
            {
                SpawnEntity(game_state, scene, EntityType_Player, { 1.f, 2.0f });
            }
        }

        if (KeyFrameDown(SDLK_n))
        {
            uint32 random_spawn = (uint32)random_float(1.0f, 1000.f);

            for(uint32 i = 0; i < random_spawn; ++i)
            {
                auto multiple = 10.f;
                auto x = random_float(-5.f * multiple, 5.f * multiple);
                auto y = random_float(-5.f * multiple, 5.f * multiple);
                SpawnEntity(game_state, scene, EntityType_Enemy, { x, y });
            }
        }

        if(KeyFrameDown(SDLK_r))
        {
            Entity* new_cam = SpawnEntity(game_state, scene, EntityType_Camera, { 1.f, 1.f });
            if(scene->player_id)
            {
                new_cam->camera.follow_target_id = scene->player_id;
            }
            new_cam->camera.camera.viewport_size = { 16.f, 9.f };
            game_state->active_camera = &new_cam->camera.camera;
        }

        if (MouseFrameDown(MouseButton::LEFT))
        {
            Vec2 mouse_pos = MouseWorldPosition();
            AddTileToMap(scene->tilemap, mouse_pos);
        }
    }

    scene->entity_delete_count = 0;
    scene->entity_delete_list = PushArray(&game_state->temporary_memory, Entity*, scene->max_entities);

    Entity* entity = scene->entities;
    uint32 max_entities = scene->max_entities; // The maximum that could be looked for, though the
                                               // loop will generally early out
                                               // TODO: This is wrong if entities are added during an update
    uint32 updated_entities = 0;
    for(uint32 i = 0; i < max_entities; ++i, ++entity)
    {
        if(updated_entities == scene->active_entities)
        {
            break;
        }
        if(entity->flags & ~EntityFlag_Enabled || entity->type == EntityType_Null)
        {
            continue;
        }

        if(scene->entity_vtable[entity->type].update)
        {
            scene->entity_vtable[entity->type].update(game_state, scene, dt, entity);
        }
        ++updated_entities;
    }

    DespawnRemovedEntities(scene);

    PopAllocation(&game_state->temporary_memory, scene->entity_delete_list);
    scene->entity_delete_list = nullptr;
}

void DrawSceneEntities(Scene* scene, Renderer* renderer)
{
    Entity* entity = scene->entities;
    uint32 max_entities = scene->max_entities;
    uint32 drawn_entities = 0;
    for(uint32 i = 0; i < max_entities; ++i, ++entity)
    {
        if(drawn_entities == scene->active_entities)
        {
            break;
        }
        if(entity->flags & ~EntityFlag_Enabled || entity->type == EntityType_Null)
        {
            continue;
        }

        if(scene->entity_vtable[entity->type].draw)
        {
            scene->entity_vtable[entity->type].draw(entity, renderer);
        }

        ++drawn_entities;
    }
}

void BuildEntityVTable(Scene* scene)
{
    assert(ArrayCount(scene->entity_vtable) == EntityType_Count);
    memset(&scene->entity_vtable[0], 0, sizeof(scene->entity_vtable));

    EntityVtable* vtable = &scene->entity_vtable[0];

    struct EntityVTableBuilder
    {
        EntityType type;
        EntityVtable vtable;
    };

    EntityVTableBuilder vtable_builder[]
    {
        { EntityType_Enemy,    { UpdateEnemy,   DrawEnemy,   SpawnEnemy         } },
        { EntityType_Spawner,  { UpdateSpawner, DrawSpawner, SpawnEnemySpawner  } },
        { EntityType_Player,   { UpdatePlayer,  DrawPlayer,  SpawnPlayer        } },
        { EntityType_Camera,   { UpdateCamera,  nullptr,     nullptr            } },
    };

#ifdef _DEBUG
    // Don't duplicate an entity in the vtable, except the Null entity;
    bool duplicate_check[EntityType_Count] = { 0 };
    int32 max_iterations = Minimum(EntityType_Count, ArrayCount(vtable_builder));
    for(int i = 0; i < max_iterations; ++i)
    {
        uint32 type = vtable_builder[i].type;

        if(type) // Can have multiple Null entities if they aren't being updated
        {
            assert(duplicate_check[type] == false);
            duplicate_check[type] = true;
        }
    }
#endif

    assert(ArrayCount(vtable_builder) <= EntityType_Count);

    for(uint32 i = 0; i < EntityType_Count; ++i)
    {
        for(uint32 builder = 0; builder < EntityType_Count; ++builder)
        {
            if(vtable_builder[builder].type == i)
            {
                uint8* source = (uint8*) &vtable_builder[builder].vtable;
                memcpy(&vtable[i], source, sizeof(EntityVtable));
                break;
            }
        }
    }
}
