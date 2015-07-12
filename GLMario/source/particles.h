#pragma once
#include <random>
// #include "glew.h"

#include "types.h"
#include "mathops.h"
#include "renderer.h"
#include "time.h"
#include "console.h"

namespace graphics
{

struct FRange
{
	float min;
	float max;

	FRange() : min(0), max(0) {}
	FRange(float min, float max) : min(min), max(max) {}
};

struct ParticleVertexData
{
	Vector2 position;
	Vector4 color;
	float scale;
};

struct ParticleFrameData
{
	Vector2 velocity;
	float start_time;
	float lifetime;
	Vector4 start_color;
	Vector4 end_color;
	bool is_alive;

	float remaining_lifetime(float current_time)
	{
		float result = lifetime - (current_time - start_time);
		return result;
	}
	bool is_active(float current_time) { return remaining_lifetime(current_time) > 0; }
	float remaining_lifetime_percent(float current_time)
	{
		float result = remaining_lifetime(current_time) / lifetime;
		return max(result, 0.f);
	}
};

struct ParticleData
{
	ParticleVertexData* pvd;
	ParticleFrameData* pfd;

	uint32 last_active_index;

	~ParticleData() { destroy(); }
	void destroy() { if (memory) delete[] memory; memory = nullptr; }
	void init(int32 count)
	{
		if (memory) delete[] memory;
		size_t mem_size = count * (sizeof(ParticleVertexData) + sizeof(ParticleFrameData)); 
		memory = new uint8[mem_size];
		memset(memory, 0, mem_size);
		this->pvd = (ParticleVertexData*) memory;
		this->pfd = (ParticleFrameData*)(memory + (count * sizeof(ParticleVertexData)));
		last_active_index = 0;
	}
private:
	uint8* memory = nullptr;
};

struct ParticleEmmissionData
{
	Vector2 spawn_position	= Vector2(-8.0f, -4.f);
	float spawn_radius		= 1.f;
	FRange start_size		= FRange(0.5f, 10.5f);
	FRange lifetime			= FRange(1.5f, 16.0f);
	Vector2 min_start_speed = Vector2(0.5f, 1.0f);
	Vector2 max_start_speed = Vector2(1.6f, 5.0f);
	Vector4 start_color		= Vector4(0, 0, 1, 0.6f);
	Vector4 end_color		= Vector4(0, 1, 0.5f, 0);

	uint32 spawn_rate = 500;
};

struct ParticleTransformData
{
	Vector2 gravity = Vector2(0, -1.f);
};

class ParticleSystem
{
public:
	ParticleSystem(uint32 max);
	~ParticleSystem() { particles.destroy(); }//if (particles) delete[] particles; }

	uint32 max_particles;

	ParticleData particles;
	ParticleTransformData ptd;
	ParticleEmmissionData ped;

	uint32 active_particles;
	
	void init_random(uint32);
	void prewarm(uint32);
	void update();
	void render();

	void create_particle(ParticleVertexData&, ParticleFrameData&, float TEMPTIME);
	void pack_particles();
private:
	void allocate();

	float random_float(float, float);
	void update_particle(ParticleVertexData&, ParticleFrameData&, Vector2&, float, float);

	Renderer* ren;
	Time* time;

	GLuint vbo;
	GLuint vao;
};

}