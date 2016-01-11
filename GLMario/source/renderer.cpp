#define __GL_IMPLEMENTATION__
#include "win32_gl.h"
#include "platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\Dependencies\stb_image.h"

#include "renderer.h"

#include "time.h"
#include "particles.h"
#include "console.h"
#include "game_types.h"

#include "SDL.h"

char* default_vert_shader = "..\\res\\default_vert.glsl";
char* default_frag_shader = "..\\res\\default_frag.glsl";
char* particle_vert_shader = "..\\res\\particle_vert.glsl";
char* particle_frag_shader = "..\\res\\particle_frag.glsl";
char* mario_image = "..\\res\\supermariobros.png";
char* main_image = "..\\res\\tiles.png";
char* text_image = "..\\res\\charmap.png";
char* particle_image = "..\\res\\particle.png";

void SwapBuffer(GameState* game_state)
{
	SDL_GL_SwapWindow(game_state->window.sdl_window);
}

Renderer* CreateRenderer(MemoryArena* arena)
{
    Renderer* result = PushStruct(arena, Renderer);
    // TODO: allocate text_array per frame based on how much text there is, or use separate arrays for separate text draw calls
    result->text_array_size = 1024 * 8;
    result->text_array = PushArray(arena, TextVertex, result->text_array_size);

    if(!LoadGLFunctionPointers())
    {
		printf("Error initializing GL");
        return nullptr;
    }

	LoadImage(result, main_image, ImageFiles::MAIN_IMAGE);
	LoadImage(result, mario_image, ImageFiles::MARIO_IMAGE);
	LoadImage(result, text_image, ImageFiles::TEXT_IMAGE);
	LoadImage(result, particle_image, ImageFiles::PARTICLE_IMAGE);
	LoadShader(result, default_vert_shader, default_frag_shader, Shader_Default);
	LoadShader(result, particle_vert_shader, particle_frag_shader, Shader_Particle);
    LoadShader(result, "..\\res\\line_vert.glsl", "..\\res\\line_frag.glsl", Shader_Line);
    LoadShader(result, "..\\res\\text_vert.glsl", "..\\res\\text_frag.glsl", Shader_Text);

	result->text_data.chars_per_line = 18;
	result->text_data.char_size = { 7, 9 };
    return result;
}

static void ActivateTexture(Renderer* ren, ImageFiles i)
{
    glBindTexture(GL_TEXTURE_2D, ren->textures[(uint32) i].texture_handle);
}

static void ActivateShader(Renderer* ren, ShaderTypes s)
{
    glUseProgram(ren->shaders[(uint32)s].shader_handle);
}

void BeginFrame(Renderer* renderer, Window* window)
{
    SDL_GL_GetDrawableSize(window->sdl_window, &renderer->frame_resolution.x, &renderer->frame_resolution.y);

	glViewport(0, 0, renderer->frame_resolution.x, renderer->frame_resolution.y);
	glClear(GL_COLOR_BUFFER_BIT);
	renderer->line_buffer_loc = 0;
}

void SetClearColor(Vec4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void ForceColorClear()
{
	glClear(GL_COLOR_BUFFER_BIT); // Would have to put a swap buffer here to notice anything
}

void Flush(Renderer* renderer, Camera* render_camera)
{
    RenderDrawBuffer(renderer, render_camera);
	for(uint32 i = 0; i < DrawLayer_Count; ++i)
	{
		renderer->draw_buffer[i].Clear();
	}
}

float ViewportWidth(Camera* camera)
{
	float result = camera->viewport_size.x;
	return result;
}

void LoadImage(Renderer* renderer, char* filename, ImageFiles location)
{
	GLuint handle = 0;

	int x, y, n;
	const int expected_components = 4;
	stbi_set_flip_vertically_on_load(true);
    // const_cast?
	uint8* data = stbi_load(const_cast<char*>(filename), &x, &y, &n, expected_components);
    assert(n == expected_components);

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


	stbi_image_free(data);

	renderer->textures[(uint32)location].texture_handle = handle;
	renderer->textures[(uint32)location].w = x;
	renderer->textures[(uint32)location].h = y;
	renderer->textures[(uint32)location].bytes_per_color = n;
}

void LoadShader(Renderer* renderer, char* vert_file, char* frag_file, ShaderTypes location)
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

	renderer->shaders[(uint32)location].shader_handle = program;
}

/*  Text drawing TODO(cgenova):
		* Character width is not correct
		* Character height is probably not correct
		* figure out the border color issue
        *      -> color not set in shader, though it wasn't being used, so it still doesn't make sense
        *
        * No more immediate mode! Make this use the ARRAY_BUFFER path
        *      -> needs dynamic vao/vbo for this to happen
        *
        *      Read whole words in to judge whether a new line is needed
        * instead of splitting words up as soon as they hit the edge
        * of the screen
		*/

TextDrawResult DrawString(Renderer* renderer, char* string, uint32 string_size, float start_x, float start_y,
                             StringTextColor* text_colors, size_t text_color_size)
{
    // TODO: more sane way of storing these
    static GLuint textVBO = (glGenBuffers(1, &textVBO), textVBO);
    static GLuint textVAO = (glGenVertexArrays(1, &textVAO), textVAO);

    const uint32 verts_per_char = 6;
    if(renderer->text_array_size < string_size * verts_per_char)
    {
        size_t new_size = string_size * verts_per_char;
        renderer->text_array = ExpandArray<TextVertex>(renderer->text_array, renderer->text_array_size, new_size);
        renderer->text_array_size = new_size;
    }

    Vec2 char_size = { 0.01f, 0.02f };
    uint32 array_pos = 0;
	float x = start_x;
	float y = start_y;
	float y_spacing = char_size.y * 1.1f;
	uint32 lines_drawn = 0;
    const uint32 tab_advance = 3;
    uint32 chars_drawn_this_line = 0; // NOTE: Unused;

    // Defaults to green text?
    StringTextColor default_color;
    default_color.color_options = StringColorOptions_Solid;
    default_color.start = 0;
    default_color.end   = string_size;
    default_color.c.solid_color = vec4(0, 1.0f, 0, 1.0f);

    StringTextColor* current_color = &default_color;
    StringTextColor* next_color = &default_color;

    size_t text_color_index = 0;

    if(text_colors && text_color_size)
    {
        current_color = text_colors;
    }

#define NEW_LINE()                  \
		y -= y_spacing;             \
		x = start_x;                \
        chars_drawn_this_line = 0;

#define TAB() x += (tab_advance - (chars_drawn_this_line % tab_advance)) * char_size.x

    Vec4 start_color = current_color->c.solid_color;
    Vec4 end_color   = current_color->c.solid_color;

    for (uint32 i = 0; i < string_size; ++i)
    {
        // TODO: Lerp color to blend between color changes
        // TODO: Put lerp information in the string color struct
        // Check for next color;
        if(current_color->end == i)
        {
            if(text_color_index < text_color_size)
            {
                ++text_color_index;
                current_color++;

                // Set up the solid color if it isn't a gradient;
                start_color = current_color->c.solid_color;
                end_color = current_color->c.solid_color;
            }
        }

        if(current_color->color_options == StringColorOptions_Gradient)
        {
            uint32 start        = current_color->start;
            uint32 end          = current_color->end;
            uint32 size         = end - start;
            uint32 relative_pos = i - start;

            assert(size > 0);
            assert(i > 0);
            float start_t = (float) ((float) relative_pos / (float)size);
            float end_t   = (float) (((float)relative_pos + 1.f) / (float)size);
            start_color = lerp(current_color->c.gradient_start, current_color->c.gradient_end, start_t);
            end_color   = lerp(current_color->c.gradient_start, current_color->c.gradient_end, end_t);
        }

        char c = string[i];

		if(c == '\n')
		{
			NEW_LINE();
			continue;
		}
		else if (c == '\t')
		{
            TAB();
            continue;
		}
        else if(c == ' ')
        {
            // No need to submit vertices to draw empty space
            x += char_size.x;
            continue;
        }

		if (x + char_size.x > 1.f)
		{
            NEW_LINE();
			lines_drawn++;
		}

        // Add a character to the array of vertices
        {
            //DrawCharacter(text_array + array_pos, string[i], x, y, char_size);

            int32 tw = renderer->textures[(uint32)ImageFiles::TEXT_IMAGE].w;
            int32 th = renderer->textures[(uint32)ImageFiles::TEXT_IMAGE].h;

            // NOTE: this assumes valid characters are being rendered
            uint32 ascii_position = c - ' ';
            assert(c >= ' ' && c <= '~');
            if(c < ' ' || c > '~')
            {
                ascii_position = c - ' ';
            }

            Rectf rect = {};
            rect.left = x;
            rect.width = char_size.x;
            rect.bot = y;
            rect.height = char_size.y;

            // Char coords
            Point2 char_pos;
            char_pos.x = ascii_position % renderer->text_data.chars_per_line;
            char_pos.x *= renderer->text_data.char_size.x;
            char_pos.y = ascii_position / renderer->text_data.chars_per_line;
            char_pos.y *= renderer->text_data.char_size.y;

            // Texture coords
            float left = (float)char_pos.x / tw;
            float right = (float)(char_pos.x + renderer->text_data.char_size.x) / tw;
            float bot = (float)(th - (char_pos.y + renderer->text_data.char_size.y)) / th;
            float top = (float)(th - char_pos.y) / th;

            TextVertex* current_vertex = renderer->text_array + array_pos;

            *current_vertex++ = { rect.left              , rect.bot               , left , bot, start_color };
            *current_vertex++ = { rect.left              , rect.bot + rect.height , left , top, start_color };
            *current_vertex++ = { rect.left + rect.width , rect.bot               , right, bot, end_color   };
            *current_vertex++ = { rect.left              , rect.bot + rect.height , left , top, start_color };
            *current_vertex++ = { rect.left + rect.width , rect.bot               , right, bot, end_color   };
            *current_vertex++ = { rect.left + rect.width , rect.bot + rect.height , right, top, end_color   };

            array_pos += 6;
            x += char_size.x;
        }
	}

	NEW_LINE();
	TextDrawResult result;
	result.bottom_right = { x, y };
	result.lines_drawn = lines_drawn;

	glUseProgram(renderer->shaders[Shader_Text].shader_handle);

    //GLint color_loc = glGetUniformLocation(shaders[Shader_Text].shader_handle, "color_modifier");
    //glUniform4f(color_loc, color.x, color.y, color.z, color.w);

	{
        // TODO(cgenova): convert to function and pull this and the one in draw_call out
		GLuint active_tex = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&active_tex);
		if (active_tex != renderer->textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle)
		{
			glBindTexture(GL_TEXTURE_2D, renderer->textures[(uint32)ImageFiles::TEXT_IMAGE].texture_handle);
		}
	}

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBindVertexArray(textVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(float)));

	glBufferData(GL_ARRAY_BUFFER, array_pos * sizeof(renderer->text_array[0]), renderer->text_array, GL_STREAM_DRAW);

    GLint cam_pos_loc = glGetUniformLocation(renderer->shaders[Shader_Text].shader_handle, "cam_pos");
    glUniform2f(cam_pos_loc, 0.5f, 0.5f);
    GLint viewport_loc = glGetUniformLocation(renderer->shaders[Shader_Text].shader_handle, "viewport");
    glUniform2f(viewport_loc, 1, 1);

    glDrawArrays(GL_TRIANGLES, 0, array_pos);

	return result;
#undef NEW_LINE
#undef TAB
}

inline Vec2 ScreenToOpenGL(Vec2 input, Vec2 viewport)
{
#define TO_OGL(pos, dir) ((((pos + (dir / 2)) / dir) * 2) - 1)
    Vec2 result;
    result.x = TO_OGL(input.x, viewport.x);
    result.y = TO_OGL(input.y, viewport.y);
    return result;
#undef TO_OGL
}

void PushDrawCall(Renderer* renderer, DrawCall draw_call, DrawLayer layer)
{
	renderer->draw_buffer[(uint32) layer].AddBack(draw_call);
}

void RenderDrawBuffer(Renderer* renderer, Camera* camera)
{
    const Rectf ogl_screen_rect = { -1.f, -1.f, 2.0f, 2.0f };

    uint32 num_drawn = 0;
    uint32 num_culled = 0;

	for (uint32 layer = 0; layer < DrawLayer_Count; ++layer)
	{
		for (uint32 i = 0; i < renderer->draw_buffer[layer].Size(); ++i)
		{
            DrawCall& data = renderer->draw_buffer[layer][i];

            ProfileBeginSection(Profile_RenderFinish);
            // TODO: set up the shaders once per frame (or once per draw surface)
            // TODO: don't have every draw type specify a shader e.g. lines only have the line shader
            // and text will ony have the text shader. Oh, and TODO: Batch text draws into a buffer.

            static GLuint vao = (glGenVertexArrays(1, &vao), vao);
            static GLuint vbo = (glGenBuffers(1, &vbo), vbo);

            Vec2 cam_position = { camera->position.x, camera->position.y };
            Vec2 viewport     = { camera->viewport_size.x, camera->viewport_size.y };

            glUseProgram(renderer->shaders[(uint32)data.shader].shader_handle);

            {
                GLuint active_tex = 0;
                glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&active_tex);

                if (active_tex != renderer->textures[(uint32)data.image].texture_handle)
                {
                    glBindTexture(GL_TEXTURE_2D, renderer->textures[(uint32)data.image].texture_handle);
                }
            }

            switch(data.draw_type)
            {
                case DrawType::SINGLE_SPRITE:
                    {
                        // TODO(cgenova): handle rotated scaling;

                        int32 tw = renderer->textures[(uint32)data.image].w;
                        int32 th = renderer->textures[(uint32)data.image].h;

                        float tex_left  = (float)data.sd.tex_rect.left / tw;
                        float tex_right = (float)(data.sd.tex_rect.left + data.sd.tex_rect.width) / tw;
                        float tex_bot   = (float)(th - (data.sd.tex_rect.top + data.sd.tex_rect.height)) / th;
                        float tex_top   = (float)(th - data.sd.tex_rect.top) / th;

                        float width  = data.sd.world_size.x;
                        float height = data.sd.world_size.y;

                        float x_pos = (data.sd.world_position.x - cam_position.x) - width / 2;
                        float y_pos = (data.sd.world_position.y - cam_position.y) - height / 2;

#define TO_OGL(pos, dir) ((((pos + (dir / 2)) / dir) * 2) - 1)

                        Rectf rect = { x_pos, y_pos, width, height };
                        Vec2_4 positions = RotatedRect(rect, data.sd.draw_angle);

                        Vec2 left_top = ScreenToOpenGL(positions.e[0], viewport);
                        Vec2 left_bot = ScreenToOpenGL(positions.e[1], viewport);
                        Vec2 right_bot = ScreenToOpenGL(positions.e[2], viewport);
                        Vec2 right_top = ScreenToOpenGL(positions.e[3], viewport);

                        x_pos = TO_OGL(x_pos, viewport.x);
                        y_pos = TO_OGL(y_pos, viewport.y);
                        width = TO_OGL(width, viewport.x);
                        height = TO_OGL(height, viewport.y);

#undef TO_OGL

#if 0
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
#else
                        SpriteVertex sprite_vertices[4] = {
                            {
                                left_bot,                    // World position
                                { tex_left, tex_bot }                // UV coords
                            },
                            {
                                right_bot,            // World position
                                { tex_right, tex_bot }               // UV coords
                            },
                            {
                                left_top,           // World position
                                { tex_left, tex_top }                // UV coords
                            },
                            {
                                right_top,   // World position
                                { tex_right, tex_top }               // UV coords
                            },
                        };
#endif

                        Rectf sprite_rect = { x_pos, y_pos, width, height };
                        Rectf msum = MinkowskiSum(ogl_screen_rect, sprite_rect);

                        if(Intersects(ogl_screen_rect, sprite_rect))
                        {
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

                            ++num_drawn;
                        }
                        else
                        {
                            ++num_culled;
                        }

                    }break;
                case DrawType::ARRAY_BUFFER:
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, data.abd.vbo);
                        glBindVertexArray(data.abd.vao);
                        glEnableVertexAttribArray(0);
                        glEnableVertexAttribArray(1);
                        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
                        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(sizeof(Vec2)));

                        GLint cam_pos_loc = glGetUniformLocation(renderer->shaders[(uint32)data.shader].shader_handle, "cam_pos");
                        glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);

                        GLint viewport_loc = glGetUniformLocation(renderer->shaders[(uint32)data.shader].shader_handle, "viewport");
                        glUniform2f(viewport_loc, viewport.x, viewport.y);

                        glDrawArrays(data.abd.draw_method, 0, data.abd.num_vertices);
                        ++num_drawn;

                    }break;
                case DrawType::LINE_BUFFER:
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, data.lbd.vbo);
                        glBindVertexArray(data.lbd.vao);
                        glEnableVertexAttribArray(0);
                        glEnableVertexAttribArray(1);
                        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
                        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(sizeof(Vec2)));

                        if(data.lbd.line_draw_flags & LineDrawOptions::SMOOTH)
                        {
                            glEnable(GL_LINE_SMOOTH);
                            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                        }
                        else
                        {
                            glDisable(GL_LINE_SMOOTH);
                        }

                        uint8 s = ((data.lbd.line_draw_flags & (0xFF << 24)) >> 24);
                        float size = (float) s;
                        glLineWidth(size);

                        if(data.lbd.line_draw_flags & LineDrawOptions::SCREEN_SPACE)
                        {
                            // Screen space drawing is in range [0,1];
                            GLint cam_pos_loc = glGetUniformLocation(renderer->shaders[data.shader].shader_handle, "cam_pos");
                            glUniform2f(cam_pos_loc, 0.5f, 0.5f);
                            GLint viewport_loc = glGetUniformLocation(renderer->shaders[data.shader].shader_handle, "viewport");
                            glUniform2f(viewport_loc, 1, 1);
                        }
                        else
                        {
                            GLint cam_pos_loc = glGetUniformLocation(renderer->shaders[data.shader].shader_handle, "cam_pos");
                            glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);
                            GLint viewport_loc = glGetUniformLocation(renderer->shaders[data.shader].shader_handle, "viewport");
                            glUniform2f(viewport_loc, viewport.x, viewport.y);
                        }

                        glDrawArrays(data.lbd.draw_method, 0, data.lbd.num_vertices);
                        //glDrawArrays(GL_LINE_LOOP, 0, data.lbd.num_vertices);

                        ++num_drawn;

                    }break;
                case DrawType::PARTICLE_ARRAY_BUFFER:
                    {
                        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                        glEnable(GL_POINT_SPRITE);

                        glBindBuffer(GL_ARRAY_BUFFER, data.abd.vbo);
                        glBindVertexArray(data.abd.vao);


                        float world_scale = ViewportWidth(camera);
                        GLint scl_loc = glGetUniformLocation(renderer->shaders[Shader_Particle].shader_handle, "w_scale");
                        glUniform1f(scl_loc, world_scale);


                        GLint cam_pos_loc = glGetUniformLocation(renderer->shaders[(uint32)data.shader].shader_handle, "cam_pos");
                        glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);

                        GLint viewport_loc = glGetUniformLocation(renderer->shaders[(uint32)data.shader].shader_handle, "viewport");
                        glUniform2f(viewport_loc, viewport.x, viewport.y);


                        //glDrawArrays(data.pbd.draw_method, 0, data.pbd.num_vertices);
                        //glDrawArrays(GL_TRIANGLE_FAN, 0, data.pbd.num_vertices);
                        glDrawArrays(data.abd.draw_method, 0, data.abd.num_vertices);

                        ++num_drawn;

                    }break;
                default:
                    assert(0);
                    break;
            }

            ProfileEndSection(Profile_RenderFinish);
		}
	}

    DebugPrintPushColor(vec4(0.8f, 0.2f, 0.2f, 1.0f));
    DebugPrintf("Draw calls drawn: %d/%d", num_drawn, (num_drawn + num_culled));
    DebugPrintPopColor();
}

static void ConvertToGLRect(Rectf* rect)
{
    rect->x = rect->x * 2.f - 1.f;
    rect->y = rect->y * 2.f - 1.f;
    rect->w *= 2.f;
    rect->h *= 2.f;
}

void DrawLine(Renderer* renderer, Vec2 start, Vec2 end, Vec4 color, LineDrawParams* params)
{
    LineDrawParams default_params;
    LineDrawParams* using_params = params ? params : &default_params;

    Array<SimpleVertex> v(2);
    v.AddEmpty(2);
    v[0].position = start;
    v[0].color = color;
    v[1].position = end;
    v[1].color = color;
    DrawLine(renderer, v, params);
}

void DrawLine(Renderer* renderer, Array<SimpleVertex>& vertices, LineDrawParams* params)
{
    LineDrawParams default_params;
    LineDrawParams* using_params = params ? params : &default_params;

	DrawCall dc = {};
	dc.draw_type = DrawType::LINE_BUFFER;
    dc.shader = Shader_Line;

    if(renderer->line_buffer_loc == renderer->line_buffer.size())
    {
		LineBufferData a = {};
		glGenBuffers(1, &a.vbo);
		glGenVertexArrays(1, &a.vao);
		glBindVertexArray(a.vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(sizeof(Vec2)));

        renderer->line_buffer.push_back(a);
    }

    LineBufferData* lbd = &renderer->line_buffer[renderer->line_buffer_loc++];
    lbd->num_vertices = vertices.Size();

    lbd->draw_method = (using_params->line_draw_flags & LineDraw_Looped) ? GL_LINE_LOOP : GL_LINE_STRIP;
    lbd->line_draw_flags = using_params->line_draw_flags;

    if(using_params->line_width)
    {
        lbd->line_draw_flags |= LineDraw_CustomWidth;
        lbd->line_draw_flags |= ((uint32)using_params->line_width << 24);
    }

    glBindBuffer(GL_ARRAY_BUFFER, lbd->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.Size() * sizeof(SimpleVertex), &vertices[0], GL_STREAM_DRAW);
    dc.lbd = *lbd;
    PushDrawCall(renderer, dc, using_params->draw_layer);
}

void DrawRect(Renderer* renderer, const Rectf& rect, Vec4 color, LineDrawParams* params)
{
    Array<SimpleVertex> verts(4);

    SimpleVertex v;
	v.color = color;
	v.position = vec2(rect.left, rect.bot);
    verts.Add(v);

	v.position = vec2(rect.left + rect.width, rect.bot);
	verts.Add(v);

	v.position = vec2(rect.left + rect.width, rect.bot + rect.height);
	verts.Add(v);

	v.position = vec2(rect.left, rect.bot + rect.height);
	verts.Add(v);

    LineDrawParams default_params;
    LineDrawParams* using_params = params ? params : &default_params;
    using_params->line_draw_flags |= LineDraw_Looped;

	DrawLine(renderer, verts, using_params);
}

void DrawRotatedRect(Renderer* renderer, const Rectf& rect, float rotation, Vec4 color, LineDrawParams* params)
{
    Vec2_4 points = RotatedRect(rect, rotation);

    Array<SimpleVertex> verts(4);

    SimpleVertex v;
	v.color = color;
	v.position = points.e[0];
    verts.Add(v);

	v.position = points.e[1];
	verts.Add(v);

	v.position = points.e[2];
	verts.Add(v);

	v.position = points.e[3];
	verts.Add(v);

    LineDrawParams default_params;
    LineDrawParams* using_params = params ? params : &default_params;
    using_params->line_draw_flags |= LineDraw_Looped;

	DrawLine(renderer, verts, using_params);
}
