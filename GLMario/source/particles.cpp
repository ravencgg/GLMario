#include "particles.h"

namespace graphics
{


ParticleSystem::ParticleSystem(uint32 max)
	: max_particles(max) 
{
	time = Time::get();
	ren = Renderer::get();
	particles.init(max);
	allocate();
	// init_random(max_particles);
	// prewarm(max_particles);
}

void ParticleSystem::allocate()
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertexData) * max_particles, particles.pvd, GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &vao);
}

void ParticleSystem::init_random(uint32 count)
{
	active_particles = min(count, max_particles);
	particles.last_active_index = active_particles;

	for(uint32 i = 0; i < active_particles; ++i)
	{
		particles.pvd[i].position = Vector2(random_float(-10.f, 10.f), random_float(-10.f, 10.f));
		particles.pvd[i].color = Vector4(random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f));
		particles.pvd[i].scale = 1.0f;

		particles.pfd[i].lifetime = random_float(0.5f, 3.0f);
		particles.pfd[i].start_time = (float) time->current_time;
	}
}

void ParticleSystem::prewarm(uint32 count)
{
	active_particles = min(count, max_particles);
	particles.last_active_index = active_particles;

	for(uint32 i = 0; i < active_particles; ++i)
	{
		create_particle(particles.pvd[i], particles.pfd[i], (float)time->current_time);
	}
}

void ParticleSystem::update()
{
	uint32 new_particles = ped.spawn_rate;
	// float current_time = (float)time->current_time;
	float current_time = (float)time->current_time;
	float dt = (float)time->delta_time;
	Vector2 frame_gravity = ptd.gravity * dt;

	uint32 i;
	for(i = 0; i < max_particles; ++i)
	{
		ParticleVertexData& pvd = particles.pvd[i];
		ParticleFrameData& pfd = particles.pfd[i];

		if(pfd.is_active(current_time))
		{
			update_particle(pvd, pfd, frame_gravity, dt, current_time);
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
				pvd = particles.pvd[particles.last_active_index];
				pfd = particles.pfd[particles.last_active_index];
				particles.pfd[particles.last_active_index].lifetime = 0;
				update_particle(pvd, pfd, frame_gravity, dt, current_time);
				--particles.last_active_index;
			}
		}
	}
	active_particles = i;

	for (int i = 0; i < 10; ++i)
	{
		std::string message("Particle count: " + std::to_string(active_particles));
		Console::get()->log_message(message);
	}
	
}

void ParticleSystem::update_particle(ParticleVertexData& pvd, ParticleFrameData& pfd, Vector2& frame_gravity, float dt, float current_time)
{
	pfd.velocity += frame_gravity;
	pvd.position += pfd.velocity * dt;
	pvd.color = lerp(pfd.start_color, pfd.end_color, 1.f - pfd.remaining_lifetime_percent(current_time));
}

void ParticleSystem::create_particle(ParticleVertexData& pvd, ParticleFrameData& pfd, float start_time)
{
	pvd.position.x = ped.spawn_position.x + random_float(-ped.spawn_radius, ped.spawn_radius);
	pvd.position.y = ped.spawn_position.y + random_float(-ped.spawn_radius, ped.spawn_radius);
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
	ren->activate_texture(ImageFiles::PARTICLE_IMAGE);
	ren->activate_shader(ShaderTypes::PARTICLE_SHADER);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleVertexData) * active_particles, particles.pvd);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertexData) * max_particles, particles.pvd, GL_STREAM_DRAW);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)(2 * sizeof(float)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleVertexData), (GLvoid*)(6 * sizeof(float)));

	GLint mat_loc = glGetUniformLocation(ren->shaders[(uint32)ShaderTypes::PARTICLE_SHADER].shader_handle, "mvp");
	glUniformMatrix4fv(mat_loc, 1, GL_FALSE, (GLfloat*)&Renderer::vp_matrix);

	float world_scale = ren->viewport_width();
	GLint scl_loc = glGetUniformLocation(ren->shaders[(uint32)ShaderTypes::PARTICLE_SHADER].shader_handle, "w_scale");
	glUniform1f(scl_loc, world_scale);

	glDrawArrays(GL_POINTS, 0, active_particles);
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


}
