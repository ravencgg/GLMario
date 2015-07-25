#pragma once

struct Sprite;
struct Animation;
struct Transform;

class IDrawer
{
public:

	// virtual void draw(Sprite*);
	// virtual void draw(Animation*);

	virtual void draw_sprite(Sprite*, Vec2) = 0;
	virtual void draw_animation(Animation*, Transform*, float) = 0;
};
