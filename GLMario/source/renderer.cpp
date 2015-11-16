#include "renderer.h"

#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\Dependencies\stb_image.h"

char* default_vert_shader = "..\\res\\default_vert.glsl";
char* default_frag_shader = "..\\res\\default_frag.glsl";
char* particle_vert_shader = "..\\res\\particle_vert.glsl";
char* particle_frag_shader = "..\\res\\particle_frag.glsl";
char* mario_image = "..\\res\\supermariobros.png";
char* main_image = "..\\res\\tiles.png";
char* text_image = "..\\res\\charmap.png";
char* particle_image = "..\\res\\particle.png";

//const uint32 Renderer::pixels_to_meters = 16;

//enum class ShaderTypes : uint32 { DEFAULT_SHADER, TEXT_SHADER, PARTICLE_SHADER, LINE_SHADER, SHADER_COUNT };
struct ShaderLoadData
{
    char* vertexFile;
    char* fragFile;
    ShaderTypes type;
};

ShaderLoadData shaderLoadData[Shader_Count];
Rect sprite_rects[(uint32) SpriteRect::RECT_COUNT] = {};
Renderer* Renderer::s_instance = nullptr;

void initialize_sprite_rects()
{
	sprite_rects[(uint32) SpriteRect::BRICK] = rect( 85, 0, 16, 16 );
	sprite_rects[(uint32) SpriteRect::STONE] = rect( 0, 0, 16, 16 );
}

Rect get_sprite_rect(SpriteRect r)
{
	assert((uint32) r < (uint32) SpriteRect::RECT_COUNT);
	Rect result = sprite_rects[(uint32)r];
	return result;
}

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
	load_image(main_image, ImageFiles::MAIN_IMAGE);
	load_image(mario_image, ImageFiles::MARIO_IMAGE);
	load_image(text_image, ImageFiles::TEXT_IMAGE);
	load_image(particle_image, ImageFiles::PARTICLE_IMAGE);
	load_shader(default_vert_shader, default_frag_shader, Shader_Default);
	load_shader(particle_vert_shader, particle_frag_shader, Shader_Particle);
    load_shader("..\\res\\line_vert.glsl", "..\\res\\line_frag.glsl", Shader_Line);
    load_shader("..\\res\\text_vert.glsl", "..\\res\\text_frag.glsl", Shader_Text);

	text_data.chars_per_line = 18;
	text_data.char_size = { 7, 9 };

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
    // Resolution can't change mid frame, although the camera position can
	frame_resolution = draw_window->get_resolution();
	glViewport(0, 0, frame_resolution.width, frame_resolution.height); // TODO: check to see if it is rendering into the title bar
                                                                       // TODO: add a way to get the client rect from the window system
	glClear(GL_COLOR_BUFFER_BIT);

	line_buffer_loc = 0;
}

void Renderer::set_clear_color(Vec4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void Renderer::force_color_clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Flush()
{
	render_draw_buffer();
	for(uint32 i = 0; i < DrawLayer_Count; ++i)
	{
		draw_buffer[i].Clear();
	}
}

void Renderer::SwapBuffer()
{
	draw_window->swap_buffer();
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

void Renderer::draw_character(char c, int32 x, int32 y)
{
    static GLuint textVBO = (glGenBuffers(1, &textVBO), textVBO);
    static GLuint textVAO = (glGenVertexArrays(1, &textVAO), textVAO);

    uint32 tshader = Shader_Text;

    int32 tw = textures[(uint32)ImageFiles::TEXT_IMAGE].w;
    int32 th = textures[(uint32)ImageFiles::TEXT_IMAGE].h;

    uint32 ascii_position = c - ' ';

    Dimension screen_size = this->draw_window->get_resolution();

    Rectf rect = {};
    rect.left = (float)(x / (float)screen_size.width) * 2.0f - 1.0f;
    rect.width = (float)(tw / (float)screen_size.width) / 4.0f;
    rect.top = (float)y / (float)screen_size.height * 2.0f - 1.0f;
    rect.height = (float)(th / (float)screen_size.height) / 2.0f;

    Point2 char_pos;
    char_pos.x = ascii_position % text_data.chars_per_line;
    char_pos.x *= text_data.char_size.width;
    char_pos.y = ascii_position / text_data.chars_per_line;
    char_pos.y *= text_data.char_size.height;

    float left = (float)char_pos.x / tw;
    float right = (float)(char_pos.x + text_data.char_size.width) / tw;
    float bot = (float)(th - (char_pos.y + text_data.char_size.height)) / th;
    float top = (float)(th - char_pos.y) / th;

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBindVertexArray(textVAO);

    struct
    {
        float x, y, tx, ty;

    } vertices[4] = {

        { rect.left, rect.top, left, bot },
        { rect.left, rect.top + rect.height, left, top },
        { rect.left + rect.width, rect.top, right, bot },
        { rect.left + rect.width, rect.top + rect.height, right, top },
    };

	glUseProgram(shaders[Shader_Text].shader_handle);
	{ // TODO(cgenova): convert to function and pull this and the one in draw_call out
		GLint active_tex = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &active_tex);

		if (active_tex != textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle)
		{
			glBindTexture(GL_TEXTURE_2D, textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle);
		}
	}

    glEnableVertexArrayAttrib(textVAO, 0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vertices), vertices, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

	glUseProgram(shaders[Shader_Text].shader_handle);
	{ // TODO(cgenova): convert to function and pull this and the one in draw_call out
		GLint active_tex = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &active_tex);

		if (active_tex != textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle)
		{
			glBindTexture(GL_TEXTURE_2D, textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, draw_object.vbo);
	glBindVertexArray(draw_object.vao);

	float bo_scale = 0.9f;

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
		x += (uint32)(text_data.char_size.width * 1.5f);
	}

	new_line();
	TextDrawResult result;
	result.bottom_right = { x, y };
	result.lines_drawn = ld;

	return result;
}

void Renderer::render_draw_buffer()
{
	for (uint32 layer = 0; layer < DrawLayer_Count; ++layer)
	{
		for (uint32 i = 0; i < draw_buffer[layer].Size(); ++i)
		{
			draw_call(draw_buffer[layer][i]);
		}
	}
}

void Renderer::push_draw_call(DrawCall draw_call, DrawLayer layer)
{
	draw_buffer[(uint32) layer].AddBack(draw_call);
}

void Renderer::draw_call(DrawCall data)
{
    // TODO: set up the shaders once per frame (or once per draw surface)
    // TODO: don't have every draw type specify a shader e.g. lines only have the line shader
    // and text will ony have the text shader. Oh, and TODO: Batch text draws into a buffer.

	static GLuint vao = (glGenVertexArrays(1, &vao), vao);
	static GLuint vbo = (glGenBuffers(1, &vbo), vbo);


    Vec2 cam_position = { main_camera->transform.position.x, main_camera->transform.position.y };
    Vec2 viewport     = { main_camera->viewport_size.x, main_camera->viewport_size.y };

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
			// TODO(cgenova): handle rotated scaling;

			int32 tw = textures[(uint32)data.image].w;
			int32 th = textures[(uint32)data.image].h;

		 	float tex_left  = (float)data.sd.tex_rect.left / tw;
		 	float tex_right = (float)(data.sd.tex_rect.left + data.sd.tex_rect.width) / tw;
		 	float tex_bot   = (float)(th - (data.sd.tex_rect.top + data.sd.tex_rect.height)) / th;
		 	float tex_top   = (float)(th - data.sd.tex_rect.top) / th;

            float width  = data.sd.world_size.x;
            float height = data.sd.world_size.y;

            float x_pos = (data.sd.world_position.x - cam_position.x) - width / 2;
            float y_pos = (data.sd.world_position.y - cam_position.y) - height / 2;

#define TO_OGL(pos, dir) ((((pos + (dir / 2)) / dir) * 2) - 1)

            x_pos = TO_OGL(x_pos, viewport.x);
            y_pos = TO_OGL(y_pos, viewport.y);
            width = TO_OGL(width, viewport.x);
            height = TO_OGL(height, viewport.y);

#undef TO_OGL

            SpriteVertex sprite_vertices[4] = {
                {
                    { x_pos, y_pos },                    // World position
                    { tex_left, tex_bot }                // UV coords
                },
                {
                    { x_pos + width, y_pos },            // World position
                    { tex_right, tex_bot }               // UV coords
                },
                {
                    { x_pos, y_pos + height },           // World position
                    { tex_left, tex_top }                // UV coords
                },
                {
                    { x_pos + width, y_pos + height },   // World position
                    { tex_right, tex_top }               // UV coords
                },
            };

            // TODO: Culling

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices, GL_STREAM_DRAW);
			glBindVertexArray(vao);

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)) );

			// NOTE(cgenova): unused, passing time to shaders should be done in separate function, not on every draw call
			//float time = 1.f;
			//GLint time_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "time");
			//glUniform1f(time_loc, time);

			//GLint color_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "color_in");
			//glUniform4f(color_loc, data.sd.color_mod.x, data.sd.color_mod.y, data.sd.color_mod.z, data.sd.color_mod.w);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		}break;
        case DrawType::ARRAY_BUFFER:
        {
			glBindBuffer(GL_ARRAY_BUFFER, data.abd.vbo);
			glBindVertexArray(data.abd.vao);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(sizeof(Vec2)));

			GLint cam_pos_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "cam_pos");
			glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);

			GLint viewport_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "viewport");
			glUniform2f(viewport_loc, viewport.x, viewport.y);

			glDrawArrays(data.abd.draw_method, 0, data.abd.num_vertices);

        }break;
        case DrawType::LINE_BUFFER:
        {
			glBindBuffer(GL_ARRAY_BUFFER, data.lbd.vbo);
			glBindVertexArray(data.lbd.vao);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(sizeof(Vec2)));


            if(data.lbd.line_draw_options & LineDrawOptions::SMOOTH)
            {
                glEnable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            }
            else
            {
                glDisable(GL_LINE_SMOOTH);
            }

            if(data.lbd.line_draw_options & LineDrawOptions::CUSTOM_SIZE)
            {
                uint8 s = ((data.lbd.line_draw_options & (0xFF << 24)) >> 24);
                float size = (float) s;
                glLineWidth(size);
            }
            else
            {
                glLineWidth(4.f);
            }

            if(data.lbd.line_draw_options & LineDrawOptions::SCREEN_SPACE)
            {
                GLint cam_pos_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "cam_pos");
                glUniform2f(cam_pos_loc, (float)frame_resolution.width / 2.f, (float)frame_resolution.height / 2.f);

                GLint viewport_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "viewport");
                glUniform2f(viewport_loc, (float)frame_resolution.width, (float)frame_resolution.height);
            }
            else
            {
                GLint cam_pos_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "cam_pos");
                glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);

                GLint viewport_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "viewport");
                glUniform2f(viewport_loc, viewport.x, viewport.y);
            }

			glDrawArrays(data.lbd.draw_method, 0, data.lbd.num_vertices);
			//glDrawArrays(GL_LINE_LOOP, 0, data.lbd.num_vertices);

        }break;
		case DrawType::PARTICLE_ARRAY_BUFFER:
		{
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			glEnable(GL_POINT_SPRITE);

			glBindBuffer(GL_ARRAY_BUFFER, data.abd.vbo);
			glBindVertexArray(data.abd.vao);

			float world_scale = viewport_width();
			GLint scl_loc = glGetUniformLocation(shaders[Shader_Particle].shader_handle, "w_scale");
			glUniform1f(scl_loc, world_scale);


			GLint cam_pos_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "cam_pos");
			glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);

			GLint viewport_loc = glGetUniformLocation(shaders[(uint32)data.shader].shader_handle, "viewport");
			glUniform2f(viewport_loc, viewport.x, viewport.y);


			//glDrawArrays(data.pbd.draw_method, 0, data.pbd.num_vertices);
			//glDrawArrays(GL_TRIANGLE_FAN, 0, data.pbd.num_vertices);
			glDrawArrays(data.abd.draw_method, 0, data.abd.num_vertices);

		}break;
		default:
			assert(0);
			break;
	}

}

void Renderer::DrawLine(Vec2 start, Vec2 end, Vec4 color, uint8 line_width, DrawLayer dl, uint32 line_draw_options)
{
    std::vector<SimpleVertex> v;
    SimpleVertex s;
    s.position = start;
    s.color = color;
    v.push_back(s);
    s.position = end;
    v.push_back(s);
    DrawLine(v, line_width, dl, line_draw_options);
}

void Renderer::DrawLine(std::vector<SimpleVertex>& vertices, uint8 line_width, DrawLayer dl, uint32 line_draw_options)
{
	DrawCall dc = {};
	dc.draw_type = DrawType::LINE_BUFFER;
    dc.shader = Shader_Line;

    if(line_buffer_loc == line_buffer.size())
    {
		LineBufferData a = {};
		glGenBuffers(1, &a.vbo);
		glGenVertexArrays(1, &a.vao);
		glBindVertexArray(a.vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(sizeof(Vec2)));

        line_buffer.push_back(a);
    }

    LineBufferData* lbd = &line_buffer[line_buffer_loc++];
    lbd->num_vertices = vertices.size();

    lbd->draw_method = (line_draw_options & LineDrawOptions::LOOPED) ? GL_LINE_LOOP : GL_LINE_STRIP;
    lbd->line_draw_options = line_draw_options;

    if(line_width)
    {
        lbd->line_draw_options |= LineDrawOptions::CUSTOM_SIZE;

        lbd->line_draw_options |= ((uint32)line_width << 24);

    }

    glBindBuffer(GL_ARRAY_BUFFER, lbd->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SimpleVertex), &vertices[0], GL_STREAM_DRAW);
    dc.lbd = *lbd;
    push_draw_call(dc, dl);
}

void Renderer::DrawRect(Rectf& rect, uint8 line_width, DrawLayer layer, Vec4 color)
{
    std::vector<SimpleVertex> verts;
    verts.reserve(4);

    SimpleVertex v;
	v.color = color;
	v.position = vec2(rect.left, rect.top);
    verts.push_back(v);

	v.position = vec2(rect.left + rect.width, rect.top);
	verts.push_back(v);

	v.position = vec2(rect.left + rect.width, rect.top + rect.height);
	verts.push_back(v);

	v.position = vec2(rect.left, rect.top + rect.height);
	verts.push_back(v);

	DrawLine(verts, line_width, layer, LineDrawOptions::LOOPED);
}

void Renderer::draw_animation(Animation* animation, Transform* t, float time)
{
	assert(animation && t && time > 1);
}
