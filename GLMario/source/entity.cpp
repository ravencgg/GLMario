#include "entity.h"

#include "scene_manager.h"

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






