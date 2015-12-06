#include "entity.h"

#include "scene_manager.h"

void SpawnEnemy(GameEntity* entity)
{
    //Entity* p = new Enemy(parent_scene);
    //p->SetPosition(vec2(0, 0));
    //parent_scene->AddEntity(p);

    assert(!"Not written");
}

void SpawnPlayer(GameEntity* new_player)
{

}

void SpawnEnemySpawner(GameEntity* new_spawner)
{

}

UpdateGameEntities(GameState* game_state, GameEntity* entities, uint32 num_entities, float dt)
{
// Entities need to be tightly packed?
// Split entities into smaller groups and update each group?  Groups could be kept to a memory page size

    GameEntity* entity;
    for(uint32 i = 0, entity = entities; i < num_entities; ++i, ++entities)
    {
        switch(entity->type)
        case EntityType_Enemy:
        {
            EntityEnemy* enemy = entity->enemy;

            enemy->velocity.y = -1.f;
            DebugPrintf("Enemy position:  (%.2f, %.2f)", enemy->transform.position.x, enemy->transform.position.y);
            const uint8 line_width = 3;
            Renderer::get()->DrawLine(enemy->transform.position, enemy->transform.position + enemy->velocity, vec4(0, 1, 1, 1), line_width);
            //TODO: physics
//            enemy->transform.position = parent_scene->physics->StepCollider(collider, velocity, FrameTime(game_state));

        }break;
        case EntityType_Player:
        {
            EntityPlayer* player = entity->player;
            const float gravity = -20.f;
            static uint32 count = 0;
            float dt = FrameTime(game_state);

            if(KeyIsDown(SDLK_SPACE))
            {
                player->velocity.y = 10.f;
            }
            else
            {
                player->velocity.y += gravity * dt;
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
                player->delete_this_frame = true;
            }

            DebugPrintf("Player collider.data->velocity: (%.2f, %.2f)", player->velocity.x, player->velocity.y);
            DebugPrintf("Player position: (%.2f, %.2f)", player->transform.position.x, player->transform.position.y);
            Vec2 old_velocity = player->velocity;
            //TODO: integrate physics in the new system
//            transform.position = parent_scene->physics->StepCollider(collider, velocity, FrameTime(game_state));
            const uint8 line_width = 3;
            Renderer::get()->DrawLine(player->transform.position, player->transform.position + old_velocity, vec4(0, 1, 1, 1), line_width);
            DebugPrintf("Player collider.data->velocity: (%.2f, %.2f)", player->velocity.x, player->velocity.y);
            ps.update(game_state, player->transform.position);

        }break;
        case EntityType_Spawner:
        {
            EntitySpawner* spawner = entity->spawner;

            float current_time = CurrentTime(game_state);

            if (current_time - spawner->last_spawn_time > spawner->time_between_spawns)
            {
                SpawnEnemy(); // TODO: Write!
                spawner->last_spawn_time = current_time;
            }

        }break;
        InvalidDefaultCase;
    }
}

void DrawGameEntities(GameEntity* entities, uint32 num_entities)
{
    GameEntity* entity = 0;
    for(uint32 i = 0, entity = entities; i < num_entities; ++i, ++entities)
    {
        switch(entity->type)
        case EntityType_Enemy:
        {

// TODO:drawing
//            draw_call.sd.world_position = transform.position;
//            Renderer::get()->push_draw_call(draw_call, DrawLayer_Player);
        }break;
        case EntityType_Player:
        {

// TODO:drawing
//            draw_call.sd.world_position = transform.position;
//            Renderer::get()->push_draw_call(draw_call, DrawLayer_Player);
        }break;
        case EntityType_Spawner:
        {

            // Invisible

        }break;
        InvalidDefaultCase;
    }

}

Enemy::Enemy(SceneManager* sm)
	: Actor(sm)
{
    Vec2 size = vec2(1.0f, 1.5f);

	draw_call = {};
	draw_call.draw_type = DrawType::SINGLE_SPRITE;
	draw_call.image = ImageFiles::MARIO_IMAGE;
	draw_call.shader = Shader_Default;
	draw_call.options = DrawOptions::TEXTURE_RECT;
    draw_call.sd.tex_rect = { 17, 903, 34, 34 };
	draw_call.sd.world_size = size;
	draw_call.sd.world_position = transform.position;
	draw_call.sd.draw_angle = 0;

    DynamicCollider col;
    col.active = true;
    col.position = transform.position;
    col.rect = { -size.x / 2.f,
                 -size.y / 2.f,
                 size.x,
                 size.y };
    col.parent = this;

    collider = parent_scene->physics->AddDynamicCollider(col);
    collider->position = transform.position;


    entity_type = EntityType_Enemy;
	velocity = vec2(-1.0f, 0);
}

void Enemy::Tick(GameState* game_state)
{
	velocity.y = -1.f;

	DebugPrintf("Enemy position:  (%.2f, %.2f)", transform.position.x, transform.position.y);

    const uint8 line_width = 3;
    Renderer::get()->DrawLine(transform.position, transform.position + velocity, vec4(0, 1, 1, 1), line_width);
    transform.position = parent_scene->physics->StepCollider(collider, velocity, FrameTime(game_state));
}

void Enemy::Draw()
{
	draw_call.sd.world_position = transform.position;
	Renderer::get()->push_draw_call(draw_call, DrawLayer_Player);
}


///////////////////////////////////////////////////
//  Spawner
///////////////////////////////////////////////////
Spawner::Spawner(SceneManager* sm)
    : Entity(sm)
    , time_between_spawns(1.0f)
    , last_spawn_time(0)
{ }

Spawner::~Spawner() {}

void Spawner::Tick(GameState* game_state)
{
    float current_time = CurrentTime(game_state);

    if (current_time - last_spawn_time > time_between_spawns)
    {
        this->SpawnEnemy();
        last_spawn_time = current_time;
    }
}

void Spawner::SpawnEnemy()
{
    Entity* p = new Enemy(parent_scene);
    p->SetPosition(vec2(0, 0));
    parent_scene->AddEntity(p);
}

void Spawner::Draw()
{
    // Invisible
}






