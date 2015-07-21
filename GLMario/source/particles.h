#pragma once
#include <random>
// #include "glew.h"

#include "types.h"
#include "mathops.h"
#include "renderer.h"
#include "time.h"
#include "console.h"

#include "SDL.h"
#include "input.h" // REMOVE Me


namespace ParticleOptions
{
	enum ParticleOptions : uint32
	{
		NONE = 0,
		LOCAL_SIM = 0x01,
	};
}

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
		uint32 alignment = 16; // 16 byte align data for SIMD.
		size_t mem_size = count * (sizeof(ParticleVertexData) + sizeof(ParticleFrameData)) + alignment;
		memory = new uint8[mem_size];
		memory = (uint8*)(((uint32)memory + (alignment - 1)) & ~(alignment - 1));
		assert(((size_t)memory & 0xF) == 0);
		memset(memory, 0, mem_size);
		this->pvd = (ParticleVertexData*) memory;
		this->pfd = (ParticleFrameData*)(memory + (count * sizeof(ParticleVertexData)));
		last_active_index = 0;
	}
private:
	uint8* memory = nullptr;
};

struct ParticleEmissionData
{
	Vector2 spawn_position	= Vector2(0.f, 0.f);
	Vector2 spawn_size      = Vector2(1, 1);
	FRange start_size		= FRange(1.f, 10.f);
	FRange lifetime			= FRange(0.5f, 4.0f);
	Vector2 min_start_speed = Vector2(-0.5f, -0.5f);
	Vector2 max_start_speed = Vector2(0.5f, 0.5f);
	Vector4 start_color		= Vector4(1.f, 0, 0, 1.f);
	Vector4 end_color		= Vector4(0.0f, 1.f, 0.f, 1.f);

	uint32 spawn_rate = 10;
};

struct ParticleTransformData
{
	Vector2 world_position = Vector2(0, 0);
	Vector2 last_world_position = Vector2(0, 0);
	Vector2 gravity = Vector2(0.0f, 0.f);

	uint32 options = ParticleOptions::NONE;
};

class ParticleSystem
{
public:
	ParticleSystem(uint32 max);
	~ParticleSystem() { particles.destroy(); }//if (particles) delete[] particles; }

	uint32 max_particles;
	uint32 active_particles;
	uint32 burst_particles;

	ParticleData particles;
	ParticleTransformData ptd;
	ParticleEmissionData ped;

	
	void init_random(uint32);
	void update(Vector2 p = Vector2(0, 0));
	void render();

	void create_particle(ParticleVertexData&, ParticleFrameData&, float);
	void create_particle_burst(uint32);
	void pack_particles();

private:
	void allocate();

	float random_float(float, float);
	inline void update_particle(ParticleVertexData&, ParticleFrameData&, Vector2&, float, float, Vector2&);
#define UPDATE_PARTICLE_WIDE
#ifdef UPDATE_PARTICLE_WIDE
	void update_particle_wide(uint32, Vector2&, float, float, Vector2&, uint32 count = 4);
#endif

	Renderer* ren;
	Time* time;

	GLuint vbo;
	GLuint vao;
};
