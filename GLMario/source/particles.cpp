#include "particles.h"


ParticleSystem::ParticleSystem(uint32 max, DrawLayer dl) 
	: max_particles(max),
	  active_particles(0),
	  burst_particles(0),
	  draw_layer(dl) 
{
	time = Time::get();
	ren = Renderer::get();
	particles.init(max);
	allocate();
	// init_random(max_particles);
}

void ParticleSystem::allocate()
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertexData) * max_particles, particles.pvd, GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

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
	particles.destroy(); 
}

void ParticleSystem::init_random(uint32 count)
{
	active_particles = min(count, max_particles);
	particles.last_active_index = active_particles;

	for(uint32 i = 0; i < active_particles; ++i)
	{
		particles.pvd[i].position = vec2(random_float(-10.f, 10.f), random_float(-10.f, 10.f));
		particles.pvd[i].color = vec4(random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f));
		particles.pvd[i].scale = 1.0f;

		particles.pfd[i].lifetime = random_float(0.5f, 3.0f);
		particles.pfd[i].start_time = (float) time->current_time;
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

void ParticleSystem::update(Vec2 new_position)
{
	uint64 cycle_start = __rdtsc();
	static uint64 avg_cycles = 0;

	float current_time = (float)time->current_time;
	float dt = (float)time->delta_time;
	Vec2 frame_gravity = ptd.gravity * dt;
	uint32 new_particles = (uint32)((float)ped.spawn_rate * dt);
	new_particles = max(new_particles + burst_particles, (uint32)1);
	burst_particles = 0;
	ptd.world_position = new_position;

	std::string new_particle_str("New Particles: " + std::to_string(new_particles));
	Console::get()->log_message(new_particle_str);

	Vec2 delta_p = ptd.world_position - ptd.last_world_position;
	ptd.last_world_position = ptd.world_position;

	static Input* input = Input::get();
if(input->is_down(SDLK_m)) // Hold m for SIMD, currently not optimized
{
// #ifdef UPDATE_PARTICLE_WIDE

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
					if (particles.last_active_index <= i) valid_particle = false;
					assert(particles.pfd[particles.last_active_index].is_active(current_time));
					pvd = particles.pvd[particles.last_active_index];
					pfd = particles.pfd[particles.last_active_index];
					particles.pfd[particles.last_active_index].lifetime = 0;
					--particles.last_active_index;
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
else
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


	uint64 cycle_end = __rdtsc();
	uint64 cycle_count = cycle_end - cycle_start;
	avg_cycles += cycle_count;
	avg_cycles = avg_cycles >> 1;
	std::string perf_message("Cycle count: " + std::to_string(cycle_count / max(active_particles, (uint32)1)));
	Console::get()->log_message(perf_message);

	std::string message("Particle count: " + std::to_string(active_particles));
	Console::get()->log_message(message);

	// std::string long_message("\nThis is a very long message that has no chance of fitting on a single line in the program, \n\tand as such will have to wrap around \tor face many consequences of its actions.\n\t\tThat is final. I have nothing else to say on the matter.");
	// Console::get()->log_message(long_message);
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
	pvd.scale = random_float(ped.start_size.min, ped.start_size.max);

	pfd.velocity.x = random_float(ped.min_start_speed.x, ped.max_start_speed.x);
	pfd.velocity.y = random_float(ped.min_start_speed.y, ped.max_start_speed.y);

	pfd.start_time = start_time;// (float)time->current_time;
	pfd.lifetime = random_float(ped.lifetime.min, ped.lifetime.max);
	pfd.start_color = ped.start_color;
	pfd.end_color = ped.end_color;
}

void ParticleSystem::render()
{
#if 1
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleVertexData) * active_particles, particles.pvd);

	DrawCall draw_call = {};
	draw_call.draw_type = DrawType::PARTICLE_ARRAY_BUFFER;
	draw_call.image = ImageFiles::PARTICLE_IMAGE; 
	draw_call.shader = ShaderTypes::PARTICLE_SHADER;
	draw_call.options |= DrawOptions::WHOLE_TEXTURE; //TODO(cgenova): support animated textures
	draw_call.pbd.vao = vao;
	draw_call.pbd.vbo = vbo;
	draw_call.pbd.draw_method = GL_POINTS;
	draw_call.pbd.num_vertices = active_particles;

	ren->push_draw_call(draw_call, draw_layer);
#else
	ren->activate_texture(ImageFiles::PARTICLE_IMAGE);
	ren->activate_shader(ShaderTypes::PARTICLE_SHADER);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleVertexData) * active_particles, particles.pvd);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)0);
	// Color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)(2 * sizeof(float)));
	// Scale
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)(6 * sizeof(float)));

	GLint mat_loc = glGetUniformLocation(ren->shaders[(uint32)ShaderTypes::PARTICLE_SHADER].shader_handle, "mvp");
	glUniformMatrix4fv(mat_loc, 1, GL_FALSE, (GLfloat*)&Renderer::vp_matrix);

	float world_scale = ren->viewport_width();
	GLint scl_loc = glGetUniformLocation(ren->shaders[(uint32)ShaderTypes::PARTICLE_SHADER].shader_handle, "w_scale");
	glUniform1f(scl_loc, world_scale);

	glDrawArrays(GL_POINTS, 0, active_particles);
#endif	
}

float ParticleSystem::random_float(float x_min, float x_max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(0, 1);

    float result = dist(mt);
    result *= (x_max - x_min); // range;
    result += x_min;
    return result;
}
