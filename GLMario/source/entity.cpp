#include "entity.h"

#include "scene_manager.h"

Enemy::Enemy(SceneManager* sm)
	: Actor(sm)
{
    Vec2 size = vec2(1.0f, 1.5f);

	draw_call = {};
	draw_call.draw_type = DrawType::SINGLE_SPRITE;
	draw_call.image = ImageFiles::MARIO_IMAGE; 
	draw_call.shader = ShaderTypes::DEFAULT_SHADER;
	draw_call.options = DrawOptions::TEXTURE_RECT;
    draw_call.sd.tex_rect = { 17, 903, 34, 34 };
	draw_call.sd.world_size = size; 
	draw_call.sd.world_position = transform.position;
	draw_call.sd.draw_angle = 0;

    TDynamicCollider col;
    col.active = true;
    col.position = transform.position;
    col.rect = { -size.x / 2.f,
                 -size.y / 2.f, 
                 size.x,
                 size.y };
    col.parent = this;

    collider = parent_scene->physics->AddDynamicCollider(col);
    ColliderSetPosition(collider, transform.position);

	velocity = vec2(-1.0f, 0);
}

void Enemy::Tick(float dt)
{
	velocity.y = -1.f;

	std::string testOut("Enemy position: ");
	testOut.append(::to_string(transform.position));
	Console::get()->log_message(testOut);

    Renderer::get()->DrawLine(transform.position, transform.position + velocity, vec4(0, 1, 1, 1));
    transform.position = parent_scene->physics->StepCollider(collider, velocity, dt);
}

void Enemy::Draw()
{
	draw_call.sd.world_position = transform.position;
	Renderer::get()->push_draw_call(draw_call, DrawLayer::PLAYER);
}
