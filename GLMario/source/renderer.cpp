#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\Dependencies\stb_image.h"

char* Renderer::default_vert_shader = "..\\res\\default_vert.glsl";
char* Renderer::default_frag_shader = "..\\res\\default_frag.glsl";
char* Renderer::main_image = "..\\res\\supermariobros.png";
uint32 Renderer::pixels_to_meters = 30;

Renderer* Renderer::s_instance = nullptr;

Renderer::Renderer(Window* w, Vector4 clear_color)
: draw_window(w)
{
	glewExperimental = GL_TRUE;
	GLenum glew_status = glewInit();
	if(glew_status)
	{
		printf("Error initializing GLEW");
	}
	set_clear_color(clear_color);

	GLuint t = 0;
	glGenVertexArrays(1, &t);

	void* test = glGenVertexArrays;
	void* test2 = glEnable;

	// NOTE(chris): also enables textures and blending
	load_image(Renderer::main_image, MAIN_IMAGE);
	load_shader(Renderer::default_vert_shader, Renderer::default_frag_shader, DEFAULT_SHADER);
	build_buffer_object();
}

void Renderer::create_instance(Window* w)
{
	if(s_instance) return;
	s_instance = new Renderer(w);
}

Renderer* Renderer::get_instance()
{
	assert(s_instance);
	return s_instance;
}

void Renderer::set_camera(Camera* camera)
{
	this->main_camera = camera;	
}

void Renderer::begin_frame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::set_clear_color(Vector4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void Renderer::end_frame()
{
	render_draw_buffer();
	draw_window->swap_buffer();
}

void Renderer::load_image(char* filename, ImageFiles location)
{
	GLuint handle = 0;

	int x, y, n;
	const int expected_components = 4;
	stbi_set_flip_vertically_on_load(true);
	uint8* data = stbi_load(const_cast<char*>(filename), &x, &y, &n, expected_components);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLenum image_type = GL_RGBA;

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	glTexImage2D(GL_TEXTURE_2D, 0, image_type, x, y, 0, image_type, GL_UNSIGNED_BYTE, data);

	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum error_type = glGetError();
	GLenum tex2D_enabled = glIsEnabled(GL_TEXTURE_2D);

	stbi_image_free(data);

	textures[location].texture_handle = handle; 
	textures[location].w = x;
	textures[location].h = y;
	textures[location].bytes_per_color = n;
}

void Renderer::load_shader(char* vert_file, char* frag_file, ShaderTypes location)
{
	GLuint vert_loc = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_loc = glCreateShader(GL_FRAGMENT_SHADER);

	char* vert_shader = load_text_file(vert_file); 
	char* frag_shader = load_text_file(frag_file); 

	glShaderSource(vert_loc, 1, &vert_shader, 0);
	glShaderSource(frag_loc, 1, &frag_shader, 0);

	GLuint program = glCreateProgram();

	glAttachShader(program, vert_loc);
	glAttachShader(program, frag_loc);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

		GLchar info[4096];
		glGetProgramInfoLog(program, log_length, NULL, info);

		printf("Shader linking error: %s\n", info);
	}

	glDetachShader(program, vert_loc);
	glDetachShader(program, frag_loc);

	glDeleteShader(vert_loc);
	glDeleteShader(frag_loc);

	delete vert_shader;
	delete frag_shader;

	shaders[location].shader_handle = program;
}

void Renderer::draw_sprite(Sprite* sprite, Transform* t)
{
	Vector3 draw_position;
	Dimension screen_dim = {};
	if(main_camera)
	{
		draw_position = main_camera->transform.position;
		screen_dim = main_camera->screen_resolution;
	}

	DrawBufferObject draw_call;
	draw_call.image = sprite->image_file;
	draw_call.shader = sprite->shader_type;
	draw_call.layer = sprite->layer;
	draw_call.tex_rect = sprite->tex_rect;
	draw_call.world_size = sprite->world_size;
	draw_call.world_position = t->position;
	draw_call.draw_angle = sprite->angle;
	// draw_call.camera_position = draw_position;
	// draw_call.camera_size = screen_dim;

	draw_buffer.add(draw_call); 
}

void Renderer::render_draw_buffer()
{
	//TODO(chris): sort draw calls;

	main_camera->update_matrices();

	for(uint32 i = 0; i < draw_buffer.size(); ++i)
	{
		draw_call(draw_buffer[i]);
	}

	// Clear it for the next frame;
	draw_buffer.clear();
}

void Renderer::draw_call(DrawBufferObject data)
{
	glUseProgram(shaders[data.shader].shader_handle);
	glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
	glBindVertexArray(draw_object.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_object.ebo);
	glBindTexture(GL_TEXTURE_2D, textures[data.image].texture_handle);
	// NOTE(chris): Layer is ignored until sorting is implemented;

	int32 tw = textures[data.image].w;
	int32 th = textures[data.image].h;

 	float left  = (float)data.tex_rect.left / tw;
 	float right = (float)(data.tex_rect.left + data.tex_rect.width) / tw;
 	float bot   = (float)(th - (data.tex_rect.top + data.tex_rect.height)) / th; 
 	float top   = (float)(th - data.tex_rect.top) / th;

	//TODO(chris): still need some conversion from pixels to world space

 	Vector3 scale_vec(data.world_size.x, data.world_size.y, 1.0f);

 	// NOTE(cgenova): position is baked into the projection matrix, view matrix is just the identity here.
	Mat4 mvp = ortho_mvp_matrix(scale_vec, data.world_position, data.draw_angle,
 							main_camera->cached_projection_matrix, main_camera->cached_view_matrix); 

	draw_object.tex_coords[0] = Vector2(left, top);
	draw_object.tex_coords[1] = Vector2(right, top);
	draw_object.tex_coords[2] = Vector2(right, bot);
	draw_object.tex_coords[3] = Vector2(left, bot);

	glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
	glBufferData(GL_ARRAY_BUFFER, draw_object.memory_size, draw_object.memory, GL_STREAM_DRAW);

	// NOTE(cgenova): unused, passing time to shaders should be done in separate function, not on every draw call	
	float time = 1.f;
	GLint time_loc = glGetUniformLocation(shaders[data.shader].shader_handle, "time");
	glUniform1f(time_loc, time);

	GLint mat_loc = glGetUniformLocation(shaders[data.shader].shader_handle, "mvp");
	glUniformMatrix4fv(mat_loc, 1, GL_TRUE, (GLfloat*)&mvp);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::build_buffer_object()
{
	uint32 num_vertices = 4;
	uint32 bytes_per_vertex = 3 + 4 + 2; // Position, RGBA, Tex coords
	uint32 memory_size = num_vertices * bytes_per_vertex * sizeof(float); 
	draw_object.memory = new uint8[memory_size];
	draw_object.memory_size = memory_size;

	uint8* mem_loc = draw_object.memory;

	draw_object.vert_positions = (Vector3*) mem_loc;
	mem_loc += sizeof(*draw_object.vert_positions) * num_vertices;
	draw_object.colors = (Vector4*) mem_loc;
	mem_loc += sizeof(*draw_object.colors) * num_vertices;
	draw_object.tex_coords = (Vector2*) mem_loc;

	draw_object.vert_positions[0] = Vector3(-1, 1, -1);//Vector3(-0.5f, 0.5f, 0.0f);
	draw_object.vert_positions[1] = Vector3(1, 1, -1);//Vector3(0.5f, 0.5f, 0.0f); 
	draw_object.vert_positions[2] = Vector3(1, -1, -1);//Vector3(0.5f, -0.5f, 0.0f);
	draw_object.vert_positions[3] = Vector3(-1, -1, -1);//Vector3(-0.5f, -0.5f, 0.0f); 

	draw_object.colors[0] = Vector4(1.0f, 1.0f, 1.0f, 1.0f); 
	draw_object.colors[1] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	draw_object.colors[2] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	draw_object.colors[3] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	draw_object.tex_coords[0] = Vector2(0.0f, 1.0f);
	draw_object.tex_coords[1] = Vector2(1.0f, 1.0f);
	draw_object.tex_coords[2] = Vector2(1.0f, 0.0f);
	draw_object.tex_coords[3] = Vector2(0.0f, 0.0f);

	GLuint indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	void* test = glGenVertexArrays;
	void* test2 = glEnable;

	glGenVertexArrays(1, &draw_object.vao);
	glBindVertexArray(draw_object.vao);

	glGenBuffers(1, &draw_object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
	glBufferData(GL_ARRAY_BUFFER, memory_size, draw_object.memory, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(4 * sizeof(*draw_object.vert_positions)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(4 * ( sizeof(*draw_object.vert_positions) + sizeof(*draw_object.colors))));

	glGenBuffers(1, &draw_object.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_object.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::draw_animation(Animation* animation, Transform* t, float time)
{
	assert(1);
}