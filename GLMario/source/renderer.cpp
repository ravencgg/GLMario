#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\Dependencies\stb_image.h"

char* Renderer::default_vert_shader = "..\\res\\default_vert.glsl";
char* Renderer::default_frag_shader = "..\\res\\default_frag.glsl";
char* Renderer::particle_vert_shader = "..\\res\\particle_vert.glsl";
char* Renderer::particle_frag_shader = "..\\res\\particle_frag.glsl";
char* Renderer::mario_image = "..\\res\\supermariobros.png";
char* Renderer::main_image = "..\\res\\tiles.png";
char* Renderer::text_image = "..\\res\\charmap.png";
char* Renderer::particle_image = "..\\res\\particle.png";
const uint32 Renderer::pixels_to_meters = 16;


Rect sprite_rects[(uint32) SpriteRect::RECT_COUNT] = {};

void initialize_sprite_rects()
{
	sprite_rects[(uint32) SpriteRect::BRICK] = rect( 85, 0, 16, 16 ); 
	sprite_rects[(uint32) SpriteRect::STONE] = { 0, 0, 16, 16 }; 
}

Rect get_sprite_rect(SpriteRect r)
{
	assert((uint32) r < (uint32) SpriteRect::RECT_COUNT);
	Rect result = sprite_rects[(uint32)r];
	return result;
}

Renderer* Renderer::s_instance = nullptr;

glm::mat4 Renderer::proj_matrix;
glm::mat4 Renderer::view_matrix;
glm::mat4 Renderer::vp_matrix;

Renderer::Renderer(Window* w, Vec4 clear_color)
: draw_window(w)
{
	glewExperimental = GL_TRUE;
	GLenum glew_status = glewInit();
	if(glew_status)
	{
		printf("Error initializing GLEW");
	}
	set_clear_color(clear_color);

	// NOTE(chris): also enables textures and blending
	load_image(Renderer::main_image, ImageFiles::MAIN_IMAGE);
	load_image(Renderer::mario_image, ImageFiles::MARIO_IMAGE);
	load_image(Renderer::text_image, ImageFiles::TEXT_IMAGE);
	load_image(Renderer::particle_image, ImageFiles::PARTICLE_IMAGE);
	load_shader(Renderer::default_vert_shader, Renderer::default_frag_shader, ShaderTypes::DEFAULT_SHADER);
	load_shader(Renderer::particle_vert_shader, Renderer::particle_frag_shader, ShaderTypes::PARTICLE_SHADER);

	text_data.chars_per_line = 18;
	text_data.char_size = { 7, 9 };

	build_buffer_object();
}

void Renderer::create_instance(Window* w)
{
	if(s_instance) return;
	initialize_sprite_rects();
	s_instance = new Renderer(w);
}

Renderer* Renderer::get()
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
	frame_resolution = draw_window->get_resolution();
	glViewport(0, 0, frame_resolution.width, frame_resolution.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //TODO(cgenova): disable depth buffer for better 2d rendering simulation


	float width = main_camera->viewport_size.x;
	float height = main_camera->viewport_size.y;

	float depth = (float) DrawLayer::LAYER_COUNT + 1.f;
	proj_matrix = glm::ortho(-width / 2, width / 2, -height / 2, height / 2, 0.1f, depth); 

	Vec3 cp = vec3(main_camera->transform.position, -1.0f);
	glm::vec3 look = glm::vec3(cp.x, cp.y, -1.0f);
	view_matrix = glm::lookAt(glm::vec3(cp.x, cp.y, 1.0f),
		look,
		glm::vec3(0, 1, 0));

	vp_matrix = proj_matrix * view_matrix;
}

void Renderer::set_clear_color(Vec4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void Renderer::force_color_clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::end_frame()
{
	render_draw_buffer();
	draw_window->swap_buffer();
	for(uint32 i = 0; i < (uint32)DrawLayer::LAYER_COUNT; ++i)
	{
		draw_buffer[i].clear();
	}
}

Dimension Renderer::get_resolution()
{
	return draw_window->get_resolution();
}

float Renderer::viewport_width()
{
	float result = main_camera->viewport_size.x;
	return result; 
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

//	GLenum error_type = glGetError();
//	GLenum tex2D_enabled = glIsEnabled(GL_TEXTURE_2D);

	stbi_image_free(data);

	textures[(uint32)location].texture_handle = handle; 
	textures[(uint32)location].w = x;
	textures[(uint32)location].h = y;
	textures[(uint32)location].bytes_per_color = n;
}

void Renderer::load_shader(char* vert_file, char* frag_file, ShaderTypes location)
{
	GLuint vert_loc = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_loc = glCreateShader(GL_FRAGMENT_SHADER);

	char* vert_shader = load_text_file(vert_file); 
	char* frag_shader = load_text_file(frag_file); 

	glShaderSource(vert_loc, 1, &vert_shader, 0);
	glShaderSource(frag_loc, 1, &frag_shader, 0);

	glCompileShader(vert_loc);
	glCompileShader(frag_loc);

	//TODO(cgenova): compilation error checking

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

	shaders[(uint32)location].shader_handle = program;
}

// void Renderer::draw_sprite(Sprite* sprite, Vec2 position)
// {
// 	DrawCall draw_info;
// 	draw_info.image = sprite->image_file;
// 	draw_info.shader = sprite->shader_type;
// 	draw_info.sd.tex_rect = sprite->tex_rect;
// 	draw_info.sd.world_size = sprite->world_size;
// 	draw_info.sd.world_position = position;
// 	draw_info.sd.draw_angle = sprite->angle;
// 	draw_info.sd.color_mod = sprite->color_mod;
// 	// draw_info.camera_position = draw_position;
// 	// draw_info.camera_size = screen_dim;
// 	draw_buffer[(uint32)sprite->layer].add(draw_info); 
// }

void Renderer::draw_character(char c, uint32 x, uint32 y)
{
	const float bo_scale = 0.9f;

	uint32 tshader = (uint32) ShaderTypes::DEFAULT_SHADER;
	glUseProgram(shaders[tshader].shader_handle);
	{ // TODO(cgenova): convert to function and pull this and the one in draw_call out
		GLint active_tex = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &active_tex);

		if (active_tex != textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle)
		{
			glBindTexture(GL_TEXTURE_2D, textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle);
		}
	}
	// NOTE(chris): Layer is ignored until sorting is implemented;
	//TODO(chris): still need some conversion from pixels to world space

	Vec3 new_position = { (float)x, (float)y, 0 };
	glm::mat4 proj  = glm::ortho(0.f, (float)frame_resolution.width, 0.f, (float)frame_resolution.height, 0.1f, 10.f);
	glm::mat4 scale = glm::scale(glm::vec3((float) text_data.char_size.width * bo_scale, (float) text_data.char_size.height * bo_scale, 1.0f));
	glm::mat4 trans = glm::translate(glm::vec3(new_position.x, new_position.y, 0.f));//  data.world_position.x, data.world_position.y, 0.0f)); // NOTE(cgenova): everything is at 1.0f in z!

	//glm::mat4 mvp = glm::mat4(1.0f);// vp_matrix * trans * rot * scale;
	glm::mat4 mvp = proj * trans * scale;

	int32 tw = textures[(uint32)ImageFiles::TEXT_IMAGE].w;
	int32 th = textures[(uint32)ImageFiles::TEXT_IMAGE].h;

	uint32 ascii_position = c - ' ';

	Point2 char_pos;
	char_pos.x = ascii_position % text_data.chars_per_line;
	char_pos.x *= text_data.char_size.width;
	char_pos.y = ascii_position / text_data.chars_per_line;
	char_pos.y *= text_data.char_size.height;

	float left = (float)char_pos.x / tw;
	float right = (float)(char_pos.x + text_data.char_size.width) / tw;
	float bot = (float)(th - (char_pos.y + text_data.char_size.height)) / th;
	float top = (float)(th - char_pos.y) / th;

	draw_object.tex_coords[0] = vec2(left, top);
	draw_object.tex_coords[1] = vec2(right, top);
	draw_object.tex_coords[2] = vec2(right, bot);
	draw_object.tex_coords[3] = vec2(left, bot);

	glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
	glBufferData(GL_ARRAY_BUFFER, draw_object.memory_size, draw_object.memory, GL_STREAM_DRAW);
	glBindVertexArray(draw_object.vao);

	GLint mat_loc = glGetUniformLocation(shaders[tshader].shader_handle, "mvp");
	glUniformMatrix4fv(mat_loc, 1, GL_FALSE, (GLfloat*)&mvp);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_object.ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// TODO(cgenova): text drawing styles;

/*  Text drawing TODO(cgenova):
		* precalculate MVP matrix in draw_string as much as possible
		* have console move down for each line
		* Character width is not correct
		* Character height is probably not correct
		* figure out the border color issue
		*/

TextDrawResult Renderer::draw_string(std::string s, uint32 start_x, uint32 start_y)
{
	uint32 x = start_x;
	uint32 y = start_y;
	uint32 ld = 0;
	uint32 y_spacing = 10;
	const uint32 tab_size = 4;
	int32 char_height = text_data.char_size.height;

	auto new_line = [&y, &x, y_spacing, char_height, start_x](){
		y -= (char_height + y_spacing);
		x = start_x;
	};

	for (uint32 i = 0; i < s.length(); ++i)
	{
		if(s[i] == '\n')
		{
			new_line();
			continue;
		}
		else if (s[i] == '\t')
		{
			x += tab_size * text_data.char_size.width;
		}
		if (x + text_data.char_size.width > (uint32)frame_resolution.width)
		{
			x = start_x;
			new_line();
			ld++;
		}		
		draw_character(s[i], x, y);
		x += (uint32)(text_data.char_size.width * 1.2f);
	}

	new_line();
	TextDrawResult result;
	result.bottom_right = { x, y };
	result.lines_drawn = ld;

	return result;
}

void Renderer::render_draw_buffer()
{
	for (uint32 layer = 0; layer < (uint32)DrawLayer::LAYER_COUNT; ++layer)
	{
		for (uint32 i = 0; i < draw_buffer[layer].size(); ++i)
		{
			draw_call(draw_buffer[layer][i]);
		}
	}
}

void Renderer::push_draw_call(DrawCall draw_call, DrawLayer layer)
{
	draw_buffer[(uint32) layer].add(draw_call);
}

void Renderer::draw_call(DrawCall data)
{
	// NOTE(cgenova): The buffer object is (-1, 1) so this conversion factor will return it to a one length object.
	const float bo_scale = 0.5;

	glUseProgram(shaders[(uint32)data.shader].shader_handle); 

	{
		GLint active_tex = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &active_tex);

		if (active_tex != textures[(uint32)data.image].texture_handle)
		{
			glBindTexture(GL_TEXTURE_2D, textures[(uint32)data.image].texture_handle);
		}
	}

	switch(data.draw_type)
	{
		case DrawType::SINGLE_SPRITE:
		{
			// NOTE(chris): Layer is ignored until sorting is implemented;
			// TODO(chris): still need some conversion from pixels to world space
			// TODO(cgenova): handle rotated scaling;
		 	// Vec3 scale_vec(data.world_size.x * 800 * bo_scale, data.world_size.y * 600 * bo_scale, 1.0f);

		 	Vec3 new_position = {data.sd.world_position.x, // * main_camera->viewport_size.x, 
		 						 data.sd.world_position.y, // * main_camera->viewport_size.y,
		 						 0};

			glm::mat4 model_matrix = glm::mat4(1.0f);
			glm::mat4 scale = glm::scale(glm::vec3(data.sd.world_size.x * bo_scale, data.sd.world_size.y * bo_scale, 1.0f));
			glm::mat4 rot = glm::rotate(glm::radians(data.sd.draw_angle), glm::vec3(0, 0, 1.0f));
			glm::mat4 trans = glm::translate(glm::vec3(new_position.x, new_position.y, 0.f));//  data.sprite_data.world_position.x, data.sprite_data.world_position.y, 0.0f)); // NOTE(cgenova): everything is at 1.0f in z!

			//glm::mat4 mvp = glm::mat4(1.0f);// vp_matrix * trans * rot * scale;
			glm::mat4 mvp = vp_matrix * trans * rot * scale;

			int32 tw = textures[(uint32)data.image].w;
			int32 th = textures[(uint32)data.image].h;

		 	float left  = (float)data.sd.tex_rect.left / tw;
		 	float right = (float)(data.sd.tex_rect.left + data.sd.tex_rect.width) / tw;
		 	float bot   = (float)(th - (data.sd.tex_rect.top + data.sd.tex_rect.height)) / th; 
		 	float top   = (float)(th - data.sd.tex_rect.top) / th;

			draw_object.tex_coords[0] = vec2(left, top);
			draw_object.tex_coords[1] = vec2(right, top);
			draw_object.tex_coords[2] = vec2(right, bot);
			draw_object.tex_coords[3] = vec2(left, bot);

			glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
			glBufferData(GL_ARRAY_BUFFER, draw_object.memory_size, draw_object.memory, GL_STREAM_DRAW);
			glBindVertexArray(draw_object.vao);

			// NOTE(cgenova): unused, passing time to shaders should be done in separate function, not on every draw call	
			float time = 1.f;
			GLint time_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "time");
			glUniform1f(time_loc, time);


			GLint color_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "color_in");
			glUniform4f(color_loc, data.sd.color_mod.x, data.sd.color_mod.y, data.sd.color_mod.z, data.sd.color_mod.w);

			GLint mat_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "mvp");
			glUniformMatrix4fv(mat_loc, 1, GL_FALSE, (GLfloat*)&mvp);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_object.ebo);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	

		}break;
		case DrawType::PARTICLE_ARRAY_BUFFER:
		{
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glEnable(GL_POINT_SPRITE);

			glBindBuffer(GL_ARRAY_BUFFER, data.pbd.vbo);
			glBindVertexArray(data.pbd.vao);

			GLint mat_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "mvp");
			glUniformMatrix4fv(mat_loc, 1, GL_FALSE, (GLfloat*)&Renderer::vp_matrix);

			float world_scale = viewport_width();
			GLint scl_loc = glGetUniformLocation(shaders[(uint32)ShaderTypes::PARTICLE_SHADER].shader_handle, "w_scale");
			glUniform1f(scl_loc, world_scale);

			glDrawArrays(data.pbd.draw_method, 0, data.pbd.num_vertices);

		}break;
		default:
			assert(0);
			break;
	}

}

void Renderer::build_buffer_object()
{
	uint32 num_vertices = 4;
	uint32 bytes_per_vertex = 3 + 4 + 2; // Position, RGBA, Tex coords
	uint32 memory_size = num_vertices * bytes_per_vertex * sizeof(float); 
	draw_object.memory = new uint8[memory_size];
	draw_object.memory_size = memory_size;

	uint8* mem_loc = draw_object.memory;

	draw_object.vert_positions = (Vec3*) mem_loc;
	mem_loc += sizeof(*draw_object.vert_positions) * num_vertices;
	draw_object.tex_coords = (Vec2*) mem_loc;

	draw_object.vert_positions[0] = vec3(-1, 1, -1);//Vec3(-0.5f, 0.5f, 0.0f);
	draw_object.vert_positions[1] = vec3(1, 1, -1);//Vec3(0.5f, 0.5f, 0.0f); 
	draw_object.vert_positions[2] = vec3(1, -1, -1);//Vec3(0.5f, -0.5f, 0.0f);
	draw_object.vert_positions[3] = vec3(-1, -1, -1);//Vec3(-0.5f, -0.5f, 0.0f); 

	draw_object.tex_coords[0] = vec2(0.0f, 1.0f);
	draw_object.tex_coords[1] = vec2(1.0f, 1.0f);
	draw_object.tex_coords[2] = vec2(1.0f, 0.0f);
	draw_object.tex_coords[3] = vec2(0.0f, 0.0f);

	GLuint indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	glGenVertexArrays(1, &draw_object.vao);
	glBindVertexArray(draw_object.vao);

	glGenBuffers(1, &draw_object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
	glBufferData(GL_ARRAY_BUFFER, memory_size, draw_object.memory, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(4 * sizeof(*draw_object.vert_positions)));

	glGenBuffers(1, &draw_object.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_object.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::draw_animation(Animation* animation, Transform* t, float time)
{
	assert(animation && t && time > 1);
}
