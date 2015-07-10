#pragma once
#include <random>
#include "glew.h"

#include "types.h"
#include "mathops.h"
#include "renderer.h"

namespace graphics
{

struct Particle
{
	Vector2 position;
	Vector4 color;
	float scale;
};

class ParticleSystem
{
public:
	ParticleSystem(uint32 max);

	uint32 max_particles;

	Particle* particles = nullptr;
	int32 p_index;

	int32 active_particles;
	
	void init_random(int32);
	void update();
	void render();

private:
	void allocate();

	float random_float(float, float);

	Renderer* ren;

	GLuint vbo;
	GLuint vao;
};

}