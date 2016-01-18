#include "input.h"
#include "entity.h"
#include "game_types.h"
#include "particles.h"
#include "console.h"
#include "tilemap.h"

#include "renderer.h"
// TODO: Move this
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

// Pass nullptr to "out" to see if an EntityID is valid
bool GetEntityWithID(Scene* scene, EntityID id, Entity** out)
{
    bool result = false;
    assert(id.index < scene->max_entities);
    if(id.index >= scene->max_entities || id.generation == 0) return result;

    Entity* entity = scene->entities + id.index;

    if(entity->id.generation == id.generation)
    {
        result = true;
        if(out) *out = entity;
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
            uint32 old_generation = entity->id.generation;

            // This resets EntityType to EntityType_Null!
            memset(entity, 0, sizeof(*entity));

            // 0 is not a valid generation id
            ++old_generation;
            entity->id.generation = old_generation ? old_generation : 1;
        }
    }
}

Rectf GetEntityRect(Entity* entity)
{
    Vec2 half_dim = entity->size * 0.5f;
    float x = entity->transform.position.x;
    float y = entity->transform.position.y;

    Rectf result = { x - half_dim.x, y - half_dim.y,
                     entity->size.x, entity->size.y };

    return result;
}

enum TileCollision
{
    TileCollision_NONE,
    TileCollision_Left   = 0x1,
    TileCollision_Top    = 0x2,
    TileCollision_Right  = 0x4,
    TileCollision_Bottom = 0x8,
};

uint32 MoveEntity(GameState* game_state, Entity* entity, Vec2 velocity)
{
    Scene* scene = game_state->active_scene;

    entity->transform.position += velocity;

    Rectf entity_bounds = GetEntityRect(entity);

    Vec2 lb_corner = { entity_bounds.Left(), entity_bounds.Bot() };
    Vec2 rb_corner = { entity_bounds.Right(), entity_bounds.Bot() };
    Vec2 lt_corner = { entity_bounds.Left(), entity_bounds.Top() };
    Vec2 rt_corner = { entity_bounds.Right(), entity_bounds.Top() };

    // TODO: do a version where this gets passed in or is calculated based on the entity size
    const uint32 points_per_side = 2;
    const uint32 num_regions = points_per_side + 1;

    Vec2 left[points_per_side];
    Vec2 right[points_per_side];
    Vec2 top[points_per_side];
    Vec2 bottom[points_per_side];


    for (int i = 0; i < points_per_side; ++i)
    {
        float side_lerp_value = (i + 1) / (float)num_regions;
        left[i] = Lerp(lb_corner, lt_corner, side_lerp_value);

    }

    for (int i = 0; i < points_per_side; ++i)
    {
        float side_lerp_value = (i + 1) / (float)num_regions;
        top[i] = Lerp(lt_corner, rt_corner, side_lerp_value);

    }

    for (int i = 0; i < points_per_side; ++i)
    {
        float side_lerp_value = (i + 1) / (float)num_regions;
        right[i] = Lerp(rb_corner, rt_corner, side_lerp_value);
    }

    for (int i = 0; i < points_per_side; ++i)
    {
        float side_lerp_value = (i + 1) / (float)num_regions;
        bottom[i] = Lerp(lb_corner, rb_corner, side_lerp_value);

    }

    TileMap* tilemap = scene->tilemap;

    // Find the potentially colliding statics
    Rectf col_plus_velocity = entity_bounds;
    col_plus_velocity.w += abs(velocity.x);
    col_plus_velocity.h += abs(velocity.y);
    if(velocity.x < 0)
    {
        col_plus_velocity.x += velocity.x;
    }
    if(velocity.y < 0)
    {
        col_plus_velocity.y += velocity.y;
    }

    uint32 num_tiles_found = 0;
    MemoryArena* temp_memory = &game_state->temporary_memory;
    Tile** possible_collision_list = GetPotentialCollidingTiles(temp_memory, tilemap, col_plus_velocity, &num_tiles_found);

    Vec2 collision_adjustment = {};

    bool collided_top = false;
    bool collided_bot = false;
    bool collided_left = false;
    bool collided_right = false;

    uint32 result = 0;

    for (uint32 i = 0; i < num_tiles_found; ++i)
    {
        Tile* tile = possible_collision_list[i];

        for (uint32 j = 0; j < points_per_side; ++j)
        {
            if (Contains(tile->aabb, left[j]))
            {
                collision_adjustment.x = tile->aabb.Right() - left[j].x;
                collided_left = true;
                result |= TileCollision_Left;
            }
        }

        for (uint32 j = 0; j < points_per_side; ++j)
        {
            if (Contains(tile->aabb, right[j]))
            {
                collision_adjustment.x = tile->aabb.Left() - right[j].x;
                collided_right = true;
                result |= TileCollision_Right;
            }
        }

        for (uint32 j = 0; j < points_per_side; ++j)
        {
            if (Contains(tile->aabb, top[j]))
            {
                collision_adjustment.y = tile->aabb.Bot() - top[j].y;
                collided_top = true;
                result |= TileCollision_Top;
            }
        }

        for (uint32 j = 0; j < points_per_side; ++j)
        {
            if (Contains(tile->aabb, bottom[j]))
            {
                collision_adjustment.y = tile->aabb.Top() - bottom[j].y;
                collided_bot = true;
                result |= TileCollision_Bottom;
            }
        }
    }

    // NOTE: check for both top and bot collisions and left and right collisions.
    // maybe switch to a penetration amount instead to give a bit of crushing leeway

    entity->transform.position += collision_adjustment;
    PopAllocation(temp_memory, possible_collision_list);

    return result;
}

static Entity* NextFreeEntitySlot(Scene* scene, EntityType new_type)
{
    Entity* entity = scene->entities;
    uint32 counter = 0;

    for(uint32 i = 0; i < scene->max_entities; ++i)
    {
        if(entity->type != EntityType_Null)
        {
            ++entity;
            continue;
        }
        else
        {
            entity->id.index = i;
            if(entity->id.generation == 0) entity->id.generation = 1;
            entity->type = new_type;
            entity->flags |= EntityFlag_Enabled;

            ++scene->active_entities;
            return entity;
        }
    }

    assert(!"Ran out of entity storage space");
    return 0;
}

Entity* SpawnEntity(GameState* game_state, Scene* scene, EntityType type, Vec2 position)
{
    Entity* entity = NextFreeEntitySlot(scene, type);
    if (!entity) return nullptr;

    entity->size = { 1.f, 1.f };
    switch(type)
    {

        case EntityType_Player:
        {
            if(!GetEntityWithID(scene, scene->player_id, 0))
            {
                scene->player_id = entity->id;
            }
            entity->transform.position = position;
            entity->transform.rotation = 0.f;
            entity->transform.scale = vec2( 1.f, 1.f );
            entity->size = vec2(1.0f, 1.5f);

        }break;
        case EntityType_Enemy:
        {

            entity->transform.position = position;
            entity->transform.rotation = 0.f;
            entity->transform.scale = vec2( 1.f, 1.f );

            // TODO: don't have this require a game_state?
            StartTimer(game_state, &entity->enemy.despawn_timer, random_float(1.f, 1.1f));

            entity->enemy.health = 3;
        }break;
        case EntityType_Spawner:
        {
            entity->spawner.last_spawn_time = 0;
            entity->spawner.time_between_spawns = 2.0f;
            entity->transform.position = position;
        }break;
        case EntityType_Camera:
        {
        }break;
    }

    return entity;
}

// TODO: de-spawn entity by ID
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
                Entity* new_player = SpawnEntity(game_state, scene, EntityType_Player, { 1.f, 4.0f });
                scene->player_id = new_player->id;
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
            if(GetEntityWithID(scene, scene->player_id, 0))
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

        switch (entity->type)
        {
            case EntityType_Player:
            {
                auto* player = &entity->player;

                const float gravity = -20.f;
                static uint32 count = 0;
                if(KeyIsDown(SDLK_SPACE) && player->grounded)
                {
                    player->velocity.y = 10.f;
                }
                else
                {
                    player->velocity.y += gravity * dt;
                    player->velocity.y = max(player->velocity.y, -100.0f);
                }

#if 0
                if(KeyIsDown(SDLK_w))
                {
                    player->velocity.y += 50.f * dt;
                    player->velocity.y = min(player->velocity.y, 5.0f);
                }
                else if(KeyIsDown(SDLK_s))
                {
                    player->velocity.y -= 50.f * dt;
                    player->velocity.y = min(player->velocity.y, 5.0f);
                }
                else
                {
                    player->velocity.y = 0;
                }
#endif

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
                //entity->transform.position += player->velocity * dt;
                uint32 collision_result = MoveEntity(game_state, entity, player->velocity * dt);

                if (collision_result & TileCollision_Bottom && player->velocity.y < 0)
                {
                    player->grounded = true;
                    player->velocity.y = 0;
                }
                else
                {
                    player->grounded = false;
                }
            }break;
            case EntityType_Enemy:
            {
                if(TimerIsFinished(game_state, &entity->enemy.despawn_timer))
                {
                    RemoveEntity(scene, entity);
                }

            }break;
            case EntityType_Spawner:
            {
                float current_time = CurrentTime(game_state);

                if (current_time - entity->spawner.last_spawn_time > entity->spawner.time_between_spawns)
                {
                    Vec2 spawn_pos = entity->transform.position;
                    if(entity->spawner.num_spawned > 0)
                    {
                        spawn_pos.y += 1.0f * (float) entity->spawner.num_spawned;
                        SpawnEntity(game_state, scene, EntityType_Enemy, spawn_pos);
                    }
                    else
                    {
                        spawn_pos.x -= 1.0f;
                        Entity* new_spawner = SpawnEntity(game_state, scene, EntityType_Spawner, spawn_pos);
                        new_spawner->spawner.last_spawn_time = current_time;
                    }

                    ++entity->spawner.num_spawned;
                    entity->spawner.last_spawn_time = current_time;
                }
            }break;
            case EntityType_Camera:
            {

                Entity* follow_target;
                if(GetEntityWithID(scene, entity->camera.follow_target_id, &follow_target))
                {
                    entity->transform.position = follow_target->transform.position;
                    entity->camera.camera.position = follow_target->transform.position;
                }
                else
                {
                    entity->camera.follow_target_id.generation = 0;
                }
            }break;
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

        switch(entity->type)
        {
            case EntityType_Player:
            {
                DrawCall draw_call = {};
                draw_call.draw_type = DrawType_Sprite;
                draw_call.sprite.image = ImageFiles::MARIO_IMAGE;
                draw_call.shader = Shader_Default;
                draw_call.draw_options = Draw_TextureRect;
                draw_call.sprite.tex_rect = { 17, 903, 34, 34 };
                draw_call.sprite.world_size = vec2(1.0f, 1.5f);
                draw_call.sprite.draw_angle = 0;
                draw_call.sprite.world_position = entity->transform.position;

                DrawSprite(renderer, draw_call, DrawLayer_Player);
            }break;
            case EntityType_Enemy:
            {
                DrawCall draw_call = {};
                draw_call.draw_type = DrawType_Sprite;
                draw_call.sprite.image = ImageFiles::MARIO_IMAGE;
                draw_call.shader = Shader_Default;
                draw_call.draw_options = Draw_TextureRect;
                draw_call.sprite.tex_rect = { 17, 903, 34, 34 };
                draw_call.sprite.world_size = vec2(1.0f, 1.5f);
                draw_call.sprite.draw_angle = 0;

                draw_call.sprite.world_position = entity->transform.position;
                DrawSprite(renderer, draw_call, DrawLayer_Player);
            }break;
            case EntityType_Spawner:
            {
                DrawCall draw_call = {};
                draw_call.draw_type = DrawType_Sprite;
                draw_call.sprite.image = ImageFiles::MARIO_IMAGE;
                draw_call.shader = Shader_Default;
                draw_call.draw_options = Draw_TextureRect;
                draw_call.sprite.tex_rect = { 34, 903, 34, 34 };
                draw_call.sprite.world_size = vec2(1.0f, 1.5f);
                draw_call.sprite.draw_angle = 0;
                draw_call.sprite.world_position = entity->transform.position;

                DrawSprite(renderer, draw_call, DrawLayer_Player);
            }break;
            case EntityType_Camera:
            {
                // TODO: Draw camera bounds
            }break;
        }

        ++drawn_entities;
    }
}

