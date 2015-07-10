#include "particles.h"

namespace graphics
{


ParticleSystem::ParticleSystem(uint32 max)
	: max_particles(max) 
{
	ren = Renderer::get();
	particles = new Particle[max_particles];
	allocate();
	init_random(max_particles);
}

void ParticleSystem::allocate()
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * max_particles, particles, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
}


void ParticleSystem::init_random(int32 count)
{
	active_particles = min(count, max_particles);

	for(int32 i = 0; i < active_particles; ++i)
	{
		particles[i].position = Vector2(random_float(-10.f, 10.f), random_float(-10.f, 10.f));
		particles[i].color = Vector4(random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f), random_float(0, 1.f));
		particles[i].scale = 1.0f;
	}
}

void ParticleSystem::update()
{
	//TODO(cgenova): update particle information here.
}

void ParticleSystem::render()
{
	ren->activate_texture(ImageFiles::PARTICLE_IMAGE);
	ren->activate_shader(ShaderTypes::PARTICLE_SHADER);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	for(int32 i = 0; i < active_particles; ++i)
	{
		particles[i].position.x += random_float(-0.1f, 0.1f);
		particles[i].position.y += random_float(-0.1f, 0.1f);
		particles[i].scale += random_float(-0.01f, 0.01f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * active_particles, particles);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * active_particles, particles, GL_STATIC_DRAW);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(2 * sizeof(float)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(6 * sizeof(float)));

	GLint mat_loc = glGetUniformLocation(ren->shaders[(uint32)ShaderTypes::PARTICLE_SHADER].shader_handle, "mvp");
	glUniformMatrix4fv(mat_loc, 1, GL_FALSE, (GLfloat*)&Renderer::vp_matrix);

	glDrawArrays(GL_POINTS, 0, active_particles);
}

float ParticleSystem::random_float(float x_min, float x_max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(x_min, x_max);

    float result = dist(mt);
    return result;
}


}
