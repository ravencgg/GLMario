#include "particles.h"
#include "containers.h"
#include "renderer.h"

#include "console.h"
#include "time.h"
#include "input.h"

#include "utility.h"
#include "mathops.h"

#include "game_types.h"

ParticleSystem::ParticleSystem()
    : max_particles(0),
      active_particles(0),
      burst_particles(0),
      draw_layer((DrawLayer)0),
	  initialized(false)
{

}

ParticleSystem::ParticleSystem(GameState* game_state, uint32 max, DrawLayer dl)
    : max_particles(max),
      active_particles(0),
      burst_particles(0),
      draw_layer(dl),
	  initialized(false)
{
	initialize(game_state, max, dl);
}

void ParticleSystem::initialize(GameState* game_state, uint32 max_particles, DrawLayer layer)
{
	assert(!initialized);
	if (initialized) return;
	this->max_particles = max_particles;
	draw_layer = layer;

    ren = game_state->renderer;
	particles.init(max_particles);

    // Allocate GPU buffers
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertexData) * max_particles, particles.pvd, GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

// TODO: change to SOA

	// Position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)0);
	// Color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)(2 * sizeof(float)));
	// Scale
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)(6 * sizeof(float)));
	glBindVertexArray(0);
}

ParticleSystem::~ParticleSystem()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	int i = 0;
	particles.destroy();
}

void ParticleSystem::init_random(float current_time, uint32 count)
{
	active_particles = min(count, max_particles);
	particles.last_active_index = active_particles;

	for(uint32 i = 0; i < active_particles; ++i)
	{
		particles.pvd[i].position = vec2(random_float(-10.f, 10.f), random_float(-10.f, 10.f));
		particles.pvd[i].color = vec4(random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f));
		particles.pvd[i].scale = 1.0f;

		particles.pfd[i].lifetime = random_float(0.5f, 3.0f);
		particles.pfd[i].start_time = current_time;
	}
}

void ParticleSystem::create_particle_burst(uint32 num_particles)
{
	burst_particles += num_particles;
	// if(count + active_particles > max_particles)
	// {
	// 	int count = max_particles - active_particles;
	// }
	// active_particles += count;
	// particles.last_active_index = active_particles;

	// for(uint32 i = 0; i < active_particles; ++i)
	// {
	// 	create_particle(particles.pvd[i], particles.pfd[i], (float)time->current_time);
	// }
}

void ParticleSystem::update(GameState* game_state, Vec2 new_position)
{
    ProfileBeginSection(Profile_ParticleUpdate);

	uint64 cycle_start = GetCycleCount();
	static uint64 avg_cycles = 0;

	float current_time = CurrentTime(game_state);
	float dt = FrameTime(game_state);
	Vec2 frame_gravity = ptd.gravity * dt;
	uint32 new_particles = (uint32)((float)ped.spawn_rate * dt);
	new_particles = max(new_particles + burst_particles, (uint32)1);
	burst_particles = 0;
	ptd.world_position = new_position;

#if 0
	std::string new_particle_str("New Particles: " + std::to_string(new_particles));
	Console::get()->log_message(new_particle_str);
#endif

	Vec2 delta_p = ptd.world_position - ptd.last_world_position;
	ptd.last_world_position = ptd.world_position;

    if(!KeyIsDown(SDLK_l)) // Hold l to disable SIMD
                                // SIMD is currently ~2x as fast as normal path
    {

        const int wide_count = 4;
        uint32 j;
        for(j = 0; j < max_particles;)
        {
            bool valid_particle = true;
            uint32 i;
            for(i = j; i < j + wide_count; ++i)
            {
                if (!(i < max_particles))
                {
                    valid_particle = false;
                    break;
                }
                ParticleVertexData& pvd = particles.pvd[i];
                ParticleFrameData& pfd = particles.pfd[i];

                if(pfd.is_active(current_time))
                {
                    // update_particle(pvd, pfd, frame_gravity, dt, current_time, delta_p);
                }
                else // Create particle in this new slot
                {
                    if(new_particles > 0) // If there is a new particle to create this frame, put it here;
                    {
                        --new_particles;
                        create_particle(pvd, pfd, current_time);
                        particles.last_active_index = max(particles.last_active_index, i); // Inconsistent without an update call on new particles every time
                    }
                    else // otherwise pack the particle data tightly
                    {
                        if (i >= particles.last_active_index)
                        {
                            valid_particle = false;
                            break;
                        }

                        while (!particles.pfd[particles.last_active_index].is_active(current_time) && particles.last_active_index > i)
                        {
                            --particles.last_active_index;
                        }

                        if (particles.last_active_index <= i)
                        {
                            valid_particle = false; // If this is false then all particles have been updated, no need to copy, continue on non-wide path
                        }
                        else
                        {
                            assert(particles.pfd[particles.last_active_index].is_active(current_time));
                            pvd = particles.pvd[particles.last_active_index];
                            pfd = particles.pfd[particles.last_active_index];
                            particles.pfd[particles.last_active_index].lifetime = 0;
                            --particles.last_active_index;
                        }
                    }
                }

                if(!valid_particle) break;
            }
            if (!valid_particle) break;
            update_particle_wide(j, frame_gravity, dt, current_time, delta_p, wide_count);
            j = i;
        }

        // TODO(cgenova): update remaining particles here.
        assert((int32)particles.last_active_index - (int32)j < 4);
        while(j <= particles.last_active_index)
        {
            bool valid_particle = true;
            if(!particles.pfd[j].is_active(current_time))
            {
                if (j >= particles.last_active_index)
                    break;
                while (!particles.pfd[particles.last_active_index].is_active(current_time) && particles.last_active_index > j)
                {
                    --particles.last_active_index;
                }

                if (particles.last_active_index <= j) break;
                assert(particles.pfd[particles.last_active_index].is_active(current_time));
                particles.pvd[j] = particles.pvd[particles.last_active_index];
                particles.pfd[j] = particles.pfd[particles.last_active_index];
                particles.pfd[particles.last_active_index].lifetime = 0;
                update_particle(particles.pvd[j], particles.pfd[j], frame_gravity, dt, current_time, delta_p);
                --particles.last_active_index;
            }
            else
            {
                update_particle(particles.pvd[j], particles.pfd[j], frame_gravity, dt, current_time, delta_p);
            }
            j++;
        }
        active_particles = j;
    }
    else // single path
        // #else
    {
        uint32 i;
        for(i = 0; i < max_particles; ++i)
        {
            ParticleVertexData& pvd = particles.pvd[i];
            ParticleFrameData& pfd = particles.pfd[i];

            if(pfd.is_active(current_time))
            {
                update_particle(pvd, pfd, frame_gravity, dt, current_time, delta_p);
            }
            else // Create particle in this new slot
            {
                if(new_particles > 0) // If there is a new particle to create this frame, put it here;
                {
                    --new_particles;
                    create_particle(pvd, pfd, current_time);
                    particles.last_active_index = max(particles.last_active_index, i);
                }
                else // otherwise pack the particle data tightly
                {
                    if (i >= particles.last_active_index)
                        break;
                    while (!particles.pfd[particles.last_active_index].is_active(current_time) && particles.last_active_index > i)
                    {
                        --particles.last_active_index;
                    }
                    if (particles.last_active_index <= i) break;
                    assert(particles.pfd[particles.last_active_index].is_active(current_time));
                    pvd = particles.pvd[particles.last_active_index];
                    pfd = particles.pfd[particles.last_active_index];
                    particles.pfd[particles.last_active_index].lifetime = 0;
                    update_particle(pvd, pfd, frame_gravity, dt, current_time, delta_p);
                    --particles.last_active_index;
                }
            }
        }
        active_particles = i;
        // #endif
    }

#if 0
	uint64 cycle_end = GetCycleCount();
	uint64 cycle_count = cycle_end - cycle_start;
	avg_cycles += cycle_count;
	avg_cycles = avg_cycles >> 1;
	std::string perf_message("Cycle count: " + std::to_string(cycle_count / max(active_particles, (uint32)1)));
	Console::get()->log_message(perf_message);
	std::string message("Particle count: " + std::to_string(active_particles));
	Console::get()->log_message(message);
#endif

    ProfileEndSection(Profile_ParticleUpdate);
}

inline void ParticleSystem::update_particle(ParticleVertexData& pvd, ParticleFrameData& pfd, Vec2& frame_gravity, float dt, float current_time, Vec2& delta_p)
{
	pfd.velocity += frame_gravity;

	if(ptd.options & ParticleOptions::LOCAL_SIM)
	{
		pvd.position += pfd.velocity * dt + delta_p;
	}
	else
	{
		pvd.position += pfd.velocity * dt;
	}
	pvd.color = lerp(pfd.start_color, pfd.end_color, 1.f - pfd.remaining_lifetime_percent(current_time));
}

#ifdef UPDATE_PARTICLE_WIDE
void ParticleSystem::update_particle_wide(uint32 s_index, Vec2& frame_gravity, float dt, float current_time, Vec2& delta_p, uint32 count)
{
	__m128 x_gravity = _mm_set1_ps(frame_gravity.x);
	__m128 y_gravity = _mm_set1_ps(frame_gravity.y);
	__m128 delta_time = _mm_set1_ps(dt);

	__m128 delta_px = _mm_set1_ps(delta_p.x);
	__m128 delta_py = _mm_set1_ps(delta_p.y);

	float times[4] = {
		1.f - particles.pfd[s_index].remaining_lifetime_percent(current_time),
		1.f - particles.pfd[s_index + 1].remaining_lifetime_percent(current_time),
		1.f - particles.pfd[s_index + 2].remaining_lifetime_percent(current_time),
		1.f - particles.pfd[s_index + 3].remaining_lifetime_percent(current_time)
	};
	__m128 lerp_t = _mm_setr_ps(*(float *)&times[0], *(float *)&times[1], *(float *)&times[2], *(float *)&times[3]);

// Start Colors
	__m128 r_scol = _mm_setr_ps(*(float *) &particles.pfd[s_index].start_color.r,
								*(float *) &particles.pfd[s_index + 1].start_color.r,
								*(float *) &particles.pfd[s_index + 2].start_color.r,
								*(float *) &particles.pfd[s_index + 3].start_color.r);

	__m128 g_scol = _mm_setr_ps(*(float *) &particles.pfd[s_index].start_color.g,
								*(float *) &particles.pfd[s_index + 1].start_color.g,
								*(float *) &particles.pfd[s_index + 2].start_color.g,
								*(float *) &particles.pfd[s_index + 3].start_color.g);

	__m128 b_scol = _mm_setr_ps(*(float *) &particles.pfd[s_index].start_color.b,
								*(float *) &particles.pfd[s_index + 1].start_color.b,
								*(float *) &particles.pfd[s_index + 2].start_color.b,
								*(float *) &particles.pfd[s_index + 3].start_color.b);

	__m128 a_scol = _mm_setr_ps(*(float *) &particles.pfd[s_index].start_color.a,
								*(float *) &particles.pfd[s_index + 1].start_color.a,
								*(float *) &particles.pfd[s_index + 2].start_color.a,
								*(float *) &particles.pfd[s_index + 3].start_color.a);

// End colors
	__m128 r_ecol = _mm_setr_ps(*(float *) &particles.pfd[s_index].end_color.r,
								*(float *) &particles.pfd[s_index + 1].end_color.r,
								*(float *) &particles.pfd[s_index + 2].end_color.r,
								*(float *) &particles.pfd[s_index + 3].end_color.r);

	__m128 g_ecol = _mm_setr_ps(*(float *) &particles.pfd[s_index].end_color.g,
								*(float *) &particles.pfd[s_index + 1].end_color.g,
								*(float *) &particles.pfd[s_index + 2].end_color.g,
								*(float *) &particles.pfd[s_index + 3].end_color.g);

	__m128 b_ecol = _mm_setr_ps(*(float *) &particles.pfd[s_index].end_color.b,
								*(float *) &particles.pfd[s_index + 1].end_color.b,
								*(float *) &particles.pfd[s_index + 2].end_color.b,
								*(float *) &particles.pfd[s_index + 3].end_color.b);

	__m128 a_ecol = _mm_setr_ps(*(float *) &particles.pfd[s_index].end_color.a,
								*(float *) &particles.pfd[s_index + 1].end_color.a,
								*(float *) &particles.pfd[s_index + 2].end_color.a,
								*(float *) &particles.pfd[s_index + 3].end_color.a);

// Position
	__m128 x_pos  = _mm_setr_ps(*(float *) &particles.pvd[s_index].position.x,
								*(float *) &particles.pvd[s_index + 1].position.x,
								*(float *) &particles.pvd[s_index + 2].position.x,
								*(float *) &particles.pvd[s_index + 3].position.x);

	__m128 y_pos  = _mm_setr_ps(*(float *) &particles.pvd[s_index].position.y,
								*(float *) &particles.pvd[s_index + 1].position.y,
								*(float *) &particles.pvd[s_index + 2].position.y,
								*(float *) &particles.pvd[s_index + 3].position.y);

// Velocity
	__m128 x_vel  = _mm_setr_ps(*(float *) &particles.pfd[s_index].velocity.x,
								*(float *) &particles.pfd[s_index + 1].velocity.x,
								*(float *) &particles.pfd[s_index + 2].velocity.x,
								*(float *) &particles.pfd[s_index + 3].velocity.x);

	__m128 y_vel  = _mm_setr_ps(*(float *) &particles.pfd[s_index].velocity.y,
								*(float *) &particles.pfd[s_index + 1].velocity.y,
								*(float *) &particles.pfd[s_index + 2].velocity.y,
								*(float *) &particles.pfd[s_index + 3].velocity.y);

// Velocity and Position calculations
	x_vel = _mm_add_ps(x_vel, x_gravity);
	y_vel = _mm_add_ps(y_vel, y_gravity);

	__m128 x_veldt = _mm_mul_ps(x_vel, delta_time);
	__m128 y_veldt = _mm_mul_ps(y_vel, delta_time);

	x_pos = _mm_add_ps(x_pos, x_veldt);
	y_pos = _mm_add_ps(y_pos, y_veldt);

	if((uint32) ptd.options & (uint32) ParticleOptions::LOCAL_SIM)
	{
		x_pos = _mm_add_ps(x_pos, delta_px);
		y_pos = _mm_add_ps(y_pos, delta_py);
	}

// Resulting colors after the lerp
	__m128 r_col  = lerp(r_scol, r_ecol, lerp_t);
	__m128 g_col  = lerp(g_scol, g_ecol, lerp_t);
	__m128 b_col  = lerp(b_scol, b_ecol, lerp_t);
	__m128 a_col  = lerp(a_scol, a_ecol, lerp_t);


// NOTE(chris): This would all be faster in SOA instead of Vec2s
	uint32 p;
	for(uint32 i = 0; i < count; ++i)
	{
		p = s_index + i;
		particles.pvd[p].position.x = ((float *)&(x_pos))[i];
		particles.pvd[p].position.y = ((float *)&(y_pos))[i];

		particles.pvd[p].color.r = ((float *)&(r_col))[i];
		particles.pvd[p].color.g = ((float *)&(g_col))[i];
		particles.pvd[p].color.b = ((float *)&(b_col))[i];
		particles.pvd[p].color.a = ((float *)&(a_col))[i];
	}

	for (uint32 i = 0; i < count; ++i)
	{
		p = s_index + i;
		particles.pfd[p].velocity.x = ((float *)&(x_vel))[i];
		particles.pfd[p].velocity.y = ((float *)&(y_vel))[i];
	}
}
#endif
/*
	pfd.velocity += frame_gravity;
	pvd.position += pfd.velocity * dt;
	pvd.color = lerp(pfd.start_color, pfd.end_color, 1.f - pfd.remaining_lifetime_percent(current_time));
*/


void ParticleSystem::create_particle(ParticleVertexData& pvd, ParticleFrameData& pfd, float start_time)
{
	pvd.position.x = ptd.world_position.x + ped.spawn_position.x + random_float(-ped.spawn_size.x / 2.f, ped.spawn_size.x / 2.f);
	pvd.position.y = ptd.world_position.y + ped.spawn_position.y + random_float(-ped.spawn_size.y / 2.f, ped.spawn_size.y / 2.f);
	pvd.color = ped.start_color;
	pvd.scale = random_float(ped.start_size.min_range, ped.start_size.max_range);

	pfd.velocity.x = random_float(ped.min_start_speed.x, ped.max_start_speed.x);
	pfd.velocity.y = random_float(ped.min_start_speed.y, ped.max_start_speed.y);

	pfd.start_time = start_time;// (float)time->current_time;
	pfd.lifetime = random_float(ped.lifetime.min_range, ped.lifetime.max_range);
	pfd.start_color = ped.start_color;
	pfd.end_color = ped.end_color;
}

void ParticleSystem::render()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleVertexData) * active_particles, particles.pvd);

	DrawCall draw_call = {};
	draw_call.draw_type = DrawType::PARTICLE_ARRAY_BUFFER;
	draw_call.image = ImageFiles::PARTICLE_IMAGE;
	draw_call.shader = Shader_Particle;
	draw_call.options |= DrawOptions::WHOLE_TEXTURE; //TODO(cgenova): support animated textures
	draw_call.abd.vao = vao;
	draw_call.abd.vbo = vbo;
	draw_call.abd.draw_method = GL_POINTS;
	draw_call.abd.num_vertices = active_particles;

    PushDrawCall(ren, draw_call, draw_layer);
}

void ParticleSystem::Tick(GameState* game_state)
{
    update(game_state, this->transform.position);
    render();
}
