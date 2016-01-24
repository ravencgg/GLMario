#define __GL_IMPLEMENTATION__
#include "win32_gl.h"

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
    Sleep(1);
	SDL_GL_SwapWindow(game_state->window.sdl_window);
}

#define VERTEX_BUFFER_SIZE MEGABYTES(64)
#define MAX_DRAW_COMMANDS (8192 * 2)

Renderer* CreateRenderer(MemoryArena* arena)
{
    Renderer* result = PushStruct(arena, Renderer);

    result->vertex_buffer = CreateSubArena(arena, VERTEX_BUFFER_SIZE);
    result->command_buffer.draw_commands = PushArray(arena, DrawCommand, MAX_DRAW_COMMANDS);
    result->command_buffer.command_array_size = MAX_DRAW_COMMANDS;
    result->command_buffer.num_commands = 0;

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

    // VSync ( 0 is disabled );
    //SDL_GL_SetSwapInterval(0);

	result->text_data.chars_per_line = 18;
	result->text_data.char_size = { 7, 9 };
    result->text_data.texture_size = result->textures[(uint32)ImageFiles::TEXT_IMAGE].size;
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
}

void SetClearColor(Vec4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void ForceColorClear()
{
	glClear(GL_COLOR_BUFFER_BIT); // Would have to put a swap buffer here to notice anything
}

float ViewportWidth(Camera* camera)
{
	float result = camera->viewport_size.x;
	return result;
}

static void LoadImage(Renderer* renderer, char* filename, ImageFiles location)
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
	renderer->textures[(uint32)location].size.x = x;
	renderer->textures[(uint32)location].size.y = y;
	renderer->textures[(uint32)location].bytes_per_color = n;
}

static void LoadShader(Renderer* renderer, char* vert_file, char* frag_file, ShaderTypes location)
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

static Rectf GetTextTextureCoord(TextData* text_data, char c)
{
    // NOTE: this assumes valid characters are being rendered
    uint32 ascii_position = c - ' ';
    assert(c >= ' ' && c <= '~');
    if(c < ' ' || c > '~')
    {
        ascii_position = c - ' ';
    }

    // Char coords
    Vec2i char_pos;
    char_pos.x = ascii_position % text_data->chars_per_line;
    char_pos.x *= text_data->char_size.x;
    char_pos.y = ascii_position / text_data->chars_per_line + 1;
    char_pos.y *= (text_data->char_size.y);

    // Texture coords
    Rectf result;
    result.x = (float)(char_pos.x) / (float)text_data->texture_size.x;
    result.w = (float)(text_data->char_size.x) / (float) text_data->texture_size.x;
    result.y = (float)(text_data->texture_size.y - char_pos.y) / (float)text_data->texture_size.y;
    result.h = (float)(text_data->char_size.y) / (float)text_data->texture_size.y;

    return result;
}

// NOTE: Advances the vertex array
static void DrawCharacter(TextVertex** vertex_array, char c, TextData* text_data, Vec4 color, Rectf pos)
{
    Rectf texture_coords = GetTextTextureCoord(text_data, c);

    float left = texture_coords.left;
    float right = texture_coords.Right();
    float top = texture_coords.Top();
    float bot = texture_coords.bot;

    TextVertex* current_vertex = *vertex_array;
    *current_vertex++ = { pos.left             , pos.bot              , left , bot, color };
    *current_vertex++ = { pos.left             , pos.bot + pos.height , left , top, color };
    *current_vertex++ = { pos.left + pos.width , pos.bot              , right, bot, color };
    *current_vertex++ = { pos.left             , pos.bot + pos.height , left , top, color };
    *current_vertex++ = { pos.left + pos.width , pos.bot              , right, bot, color };
    *current_vertex++ = { pos.left + pos.width , pos.bot + pos.height , right, top, color };

    *vertex_array = current_vertex;
}


void DrawStringInRect(Renderer* ren, char* string, Rectf bounds, Vec2 text_size, Vec4 color, uint32 options, DrawLayer draw_layer)
{
    const uint32 tab_advance = 3;
    const uint32 verts_per_char = 6; // Could be 4 with element buffers
    Vec2i texture_size = ren->textures[(uint32)ImageFiles::TEXT_IMAGE].size;
    assert(string);
    if (*string == 0) return;

    DrawCommand command = {};
    command.draw_call.draw_type = DrawType_Text;
    command.layer = draw_layer;
    command.draw_call.draw_options |= Draw_ScreenSpace; // NOTE: This could work with world space,
                                                        // with more dynamic character sizes
    command.draw_call.shader = ShaderTypes::Shader_Text;
    command.draw_call.text.image = ImageFiles::TEXT_IMAGE;

    Rectf char_rect;
    char_rect.w = text_size.x;
    char_rect.h = text_size.y;
    char_rect.x = bounds.x;
    char_rect.y = bounds.Top() - char_rect.h;

    float chars_fit_hor_float = bounds.w / char_rect.w;
    uint32 chars_fit_hor = (uint32) chars_fit_hor_float;
    uint32 remaining_space_hor = chars_fit_hor;

    float chars_fit_ver_float = bounds.h / char_rect.h;
    uint32 chars_fit_ver = (uint32) chars_fit_ver_float;
    uint32 remaining_space_ver = chars_fit_ver;

	float y_spacing = char_rect.h * 1.1f;
	uint32 lines_drawn = 0;

    struct Line
    {
        char* start;
        char* end;
        uint32 drawn_chars; // includes non-leading and trailing whitespace
    };

    uint32 used_lines = 0;
    Line lines[128] = { 0 };

    uint32 chars_to_draw = 0;

    while (*string)
    {
        assert(used_lines < ArrayCount(lines));
        Line* current_line = lines + used_lines;

        StrSkipWhitespace(&string);
        current_line->start = string;

        uint32 trailing_whitespace = 0;
        while (*string && remaining_space_hor)
        {
            // Read until the line is full

            if (remaining_space_hor <= 0) {
                break;
            }

            if (*string == '\n')
            {
                string++;
                break;
            }

            size_t token_size;
            char* token_end = StrGetToken(string, &token_size);

            if (token_size > remaining_space_hor)
            {
                if (token_size > (uint32)chars_fit_hor)
                {
                    current_line->drawn_chars += (uint32)trailing_whitespace;
                    string = string + remaining_space_hor;
                    chars_to_draw += remaining_space_hor;
                    current_line->drawn_chars += remaining_space_hor;
                    break;
                }
                else
                {
                    // Needs to go on the next line
                    break;
                }
            }
            else
            {
                current_line->drawn_chars += (uint32)trailing_whitespace;
                current_line->drawn_chars += (uint32)token_size;
                chars_to_draw += (uint32)token_size;
                remaining_space_hor -= (uint32)token_size;
                string += token_size;
            }

            if (*string == '\n') break; // handle on the next iteration

            trailing_whitespace = (uint32)StrSkipWhitespace(&string, tab_advance);

            if (trailing_whitespace >= remaining_space_hor)
            {
                break;
            }
            else
            {
                remaining_space_hor -= (uint32)trailing_whitespace;
            }

        }

        remaining_space_hor = chars_fit_hor;
        current_line->end = string;
        ++used_lines;
    }

    command.vertex_buffer_offset = ren->vertex_buffer.used;
    command.num_vertices = verts_per_char * chars_to_draw; // Remove spaces from calculation?
    TextVertex* base_text_vertex = PushArray(&ren->vertex_buffer, TextVertex, command.num_vertices);
    TextVertex* current_vertex = base_text_vertex;

    float vert_space = used_lines * y_spacing;
    float empty_space = bounds.h - vert_space;

    char_rect.y = bounds.y + bounds.h; // Top aligned by default
    if ((options & String_VerAlignCenter) == String_VerAlignCenter)
    {
        char_rect.y -= empty_space / 2.f;
    }
    else if (options & String_VerAlignBottom)
    {
        char_rect.y -= empty_space;
    }

    foru_iz(used_lines)
    {
        Line* current_line = lines + i;
        size_t len = current_line->end - current_line->start;
        char* c = current_line->start;

        chars_fit_hor_float;

        float line_width = current_line->drawn_chars * char_rect.w;
        assert(bounds.w >= line_width);
        float empty_space = bounds.w - line_width;

        char_rect.x = bounds.x; // Default is left aligned

        if ((options & String_HorAlignCenter) == String_HorAlignCenter)
        {
            char_rect.x += empty_space / 2;
        }
        else if (options & String_HorAlignRight)
        {
            char_rect.x += empty_space;
        }

        char_rect.y -= y_spacing;

        while (c != current_line->end)
        {
            if (*c == '\n')
            {
                // Has to be a new line anyway, so just let it break out
            }
            else if (*c == ' ')
            {
                char_rect.x += char_rect.w;
            }
            else if (*c == '\t')
            {
                char_rect.x += char_rect.w * tab_advance;
            }
            else
            {
                DrawCharacter(&base_text_vertex, *c, &ren->text_data, color, char_rect);
                char_rect.x += char_rect.w;
            }

            ++c;
        }
    }

    PushDrawCommand(ren, command);
}

/*  Text drawing TODO(cgenova):
		* figure out the border color issue
        *      -> color not set in shader, though it wasn't being used, so it still doesn't make sense
        *
        *      Read whole words in to judge whether a new line is needed
        * instead of splitting words up as soon as they hit the edge
        * of the screen
		*/

TextDrawResult DrawString(Renderer* renderer, char* string, uint32 string_size, float start_x, float start_y,
                            StringTextColor* text_colors, size_t text_color_size, DrawLayer draw_layer)
{

    const uint32 verts_per_char = 6;

    DrawCommand command = {};
    command.vertex_buffer_offset = renderer->vertex_buffer.used;
    command.num_vertices = verts_per_char * string_size; // Remove spaces from calculation?
    TextVertex* base_text_vertex = PushArray(&renderer->vertex_buffer, TextVertex, command.num_vertices);

    command.draw_call.draw_type = DrawType_Text;
    command.layer = draw_layer;
    command.draw_call.draw_options |= Draw_ScreenSpace;

    command.draw_call.shader = ShaderTypes::Shader_Text;
    command.draw_call.text.image = ImageFiles::TEXT_IMAGE;

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
//    StringTextColor* next_color = &default_color;

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

            int32 tw = renderer->textures[(uint32)ImageFiles::TEXT_IMAGE].size.x;
            int32 th = renderer->textures[(uint32)ImageFiles::TEXT_IMAGE].size.y;

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
            Vec2i char_pos;
            char_pos.x = ascii_position % renderer->text_data.chars_per_line;
            char_pos.x *= renderer->text_data.char_size.x;
            char_pos.y = ascii_position / renderer->text_data.chars_per_line;
            char_pos.y *= renderer->text_data.char_size.y;

            // Texture coords
            float left = (float)char_pos.x / tw;
            float right = (float)(char_pos.x + renderer->text_data.char_size.x) / tw;
            float top = (float)(th - (char_pos.y + renderer->text_data.char_size.y)) / th;
            float bot = (float)(th - char_pos.y) / th;

            TextVertex* current_vertex = base_text_vertex + array_pos;

            *current_vertex++ = { rect.left              , rect.bot               , left , top, start_color };
            *current_vertex++ = { rect.left              , rect.bot + rect.height , left , bot, start_color };
            *current_vertex++ = { rect.left + rect.width , rect.bot               , right, top, end_color   };
            *current_vertex++ = { rect.left              , rect.bot + rect.height , left , bot, start_color };
            *current_vertex++ = { rect.left + rect.width , rect.bot               , right, top, end_color   };
            *current_vertex++ = { rect.left + rect.width , rect.bot + rect.height , right, bot, end_color   };

            array_pos += 6;
            x += char_size.x;
        }
	}

	NEW_LINE();
	TextDrawResult result;
	result.bottom_right = { x, y };
	result.lines_drawn = lines_drawn;

    PushDrawCommand(renderer, command);

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

void PushDrawCommand(Renderer* renderer, DrawCommand command)
{
    DrawCommand* draw_command = renderer->command_buffer.draw_commands + renderer->command_buffer.num_commands++;

    if (renderer->command_buffer.num_commands == renderer->command_buffer.command_array_size)
    {
        assert(!"Ran out of command storage");
        return;
    }

    *draw_command = command;
}

#define TO_OGL(pos, dir) ((((pos + (dir / 2)) / dir) * 2) - 1)
#undef TO_OGL

void DrawSprite(Renderer* renderer, DrawCall draw_call, DrawLayer layer)
{
    DrawCall data = draw_call; // lol
    size_t start_offset = renderer->vertex_buffer.used;
    SpriteVertex* vertex = PushArray(&renderer->vertex_buffer, SpriteVertex, 4);
    assert(renderer->vertex_buffer.used - sizeof(SpriteVertex) * 4 == start_offset);

    DrawCommand command = {};
    command.draw_call = draw_call;
    command.num_vertices = 4;
    command.vertex_type = VertexType_Sprite;
    command.vertex_buffer_offset = start_offset;
    command.layer = layer;

    int32 tw = renderer->textures[(uint32)data.sprite.image].size.x;
    int32 th = renderer->textures[(uint32)data.sprite.image].size.y;

    float tex_left  = (float)data.sprite.tex_rect.left / tw;
    float tex_right = (float)(data.sprite.tex_rect.left + data.sprite.tex_rect.width) / tw;
    float tex_bot   = (float)(th - (data.sprite.tex_rect.bot + data.sprite.tex_rect.height)) / th;
    float tex_top   = (float)(th - data.sprite.tex_rect.bot) / th;

    float width  = data.sprite.world_size.x;
    float height = data.sprite.world_size.y;

    float left = data.sprite.world_position.x - width / 2.f;
    float bot = data.sprite.world_position.y - height / 2.f;

    Rectf draw_rect = { left, bot, width, height };
    Vec2_4 points = RotatedRect(draw_rect, draw_call.sprite.draw_angle, &command.draw_aabb);

    *vertex++ = { points.e[1], { tex_left, tex_bot  } };
    *vertex++ = { points.e[2], { tex_right, tex_bot } };
    *vertex++ = { points.e[0], { tex_left, tex_top  } };
    *vertex++ = { points.e[3], { tex_right, tex_top } };

    DrawRect(renderer, command.draw_aabb, vec4(1, 1, 1, 1));

    PushDrawCommand(renderer, command);
}

static inline void SetImage(Renderer* renderer, ImageFiles image)
{
    {
        GLuint active_tex = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&active_tex);

        if (active_tex != renderer->textures[(uint32) image].texture_handle)
        {
            glBindTexture(GL_TEXTURE_2D, renderer->textures[(uint32)image].texture_handle);
        }
    }
}

// TODO: update shader uniforms once per frame
void RenderDrawBuffer(Renderer* renderer, Camera* camera)
{
    uint32 num_drawn = 0;
    uint32 num_culled = 0;

    static GLuint vao = (glGenVertexArrays(1, &vao), vao);
    static GLuint vbo = (glGenBuffers(1, &vbo), vbo);

    Vec2 cam_position = { camera->position.x, camera->position.y };
    Vec2 viewport     = { camera->viewport_size.x, camera->viewport_size.y };

    Rectf view_rect = { cam_position.x - viewport.x / 2.f, cam_position.y - viewport.y / 2.f, viewport.x, viewport.y };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, renderer->vertex_buffer.used, renderer->vertex_buffer.base, GL_STATIC_DRAW);

    // TODO: Stable sorting, on a smaller struct!
    qsort(renderer->command_buffer.draw_commands, renderer->command_buffer.num_commands, sizeof(DrawCommand), [](const void* lhs, const void* rhs)-> int{
        DrawCommand* l = (DrawCommand*)lhs;
        DrawCommand* r = (DrawCommand*)rhs;
        return l->layer.sort_key > r->layer.sort_key;
    });

    // TODO: perform culling at the beginning instead of per type. All commands will have to have the
    // relevant screen_space and aabb information

    for (uint32 command_index = 0; command_index < renderer->command_buffer.num_commands; ++command_index)
    {
        ProfileBeginSection(Profile_RenderFinish);

        DrawCommand* command = renderer->command_buffer.draw_commands + command_index;
        DrawCall data = command->draw_call;

        glUseProgram(renderer->shaders[(uint32)data.shader].shader_handle);

        GLsizei buffer_offset = (GLsizei)command->vertex_buffer_offset;

        switch (data.draw_type)
        {
        case DrawType_Sprite:
        {
            assert(command->vertex_type == VertexType_Sprite);

            SetImage(renderer, data.sprite.image);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);

            if (Intersects(view_rect, command->draw_aabb))
            {
                glBindVertexArray(vao);

                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (GLvoid*)buffer_offset);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (GLvoid*)(buffer_offset + 2 * sizeof(float)));

                GLint cam_pos_loc = glGetUniformLocation(renderer->shaders[(uint32)data.shader].shader_handle, "cam_pos");
                glUniform2f(cam_pos_loc, cam_position.x, cam_position.y);

                GLint viewport_loc = glGetUniformLocation(renderer->shaders[(uint32)data.shader].shader_handle, "viewport");
                glUniform2f(viewport_loc, viewport.x, viewport.y);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                ++num_drawn;
            }
            else
            {
                ++num_culled;
            }
        }break;
        case DrawType_Text:
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBindVertexArray(vao);

            SetImage(renderer, data.text.image);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) buffer_offset);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(buffer_offset + 4 * sizeof(float)));

            assert(data.shader == Shader_Text);
            if (data.draw_options & Draw_ScreenSpace)
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

            glDrawArrays(GL_TRIANGLES, 0, command->num_vertices);
            ++num_drawn;

        }break;
        case DrawType_Primitive:
        {
            Rectf view_bounds = (command->draw_call.draw_options & Draw_ScreenSpace)
                                ? RectFromDimCorner({ 0, 0 }, { 1.f, 1.f }) : view_rect;

            if (Intersects(view_bounds, command->draw_aabb))
            {
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBindVertexArray(vao);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)buffer_offset);
                glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (GLvoid*)(buffer_offset + sizeof(Vec2)));

                if (data.draw_options & PrimitiveDraw_Smooth)
                {
                    glEnable(GL_LINE_SMOOTH);
                    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                }
                else
                {
                    glDisable(GL_LINE_SMOOTH);
                }

                uint8 s = 1;
                if (data.draw_options & PrimitiveDraw_CustomWidth)
                {
                    s = data.line.line_width;
                }
                float size = (float)s;
                glLineWidth(size);

                if (data.draw_options & Draw_ScreenSpace)
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

                // TODO: culling?
                glDrawArrays(data.line.draw_method, 0, (GLsizei)command->num_vertices);
                //glDrawArrays(GL_LINE_LOOP, 0, data.lbd.num_vertices);

                ++num_drawn;
            }
            else
            {
                ++num_culled;
            }

        }break;
        case DrawType_Particles:
        {
            glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
            glEnable(GL_POINT_SPRITE);

            SetImage(renderer, data.owning_buffer.image);

            glBindBuffer(GL_ARRAY_BUFFER, data.owning_buffer.vbo);
            glBindVertexArray(data.owning_buffer.vao);

            float world_scale = ViewportWidth(camera);
            GLint scl_loc = glGetUniformLocation(renderer->shaders[Shader_Particle].shader_handle, "w_scale");
            glUniform1f(scl_loc, world_scale);

            if (data.draw_options & Draw_ScreenSpace)
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

            glDrawArrays(data.owning_buffer.draw_method, 0, data.owning_buffer.num_vertices);

            ++num_drawn;

        }break;
        InvalidDefaultCase;
        }

        ProfileEndSection(Profile_RenderFinish);
    }

    ClearArena(&renderer->vertex_buffer);
    renderer->command_buffer.num_commands = 0;

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

void DrawLine(Renderer* renderer, Vec2 start, Vec2 end, Vec4 color, PrimitiveDrawParams params)
{
    SimpleVertex v[2];
    v[0].position = start;
    v[0].color = color;
    v[1].position = end;
    v[1].color = color;


    DrawLine(renderer, v, 2, params);
}

static void DrawPrimitive(Renderer* renderer, size_t vertex_buffer_start, uint32 num_vertices, PrimitiveDrawParams params, Rectf aabb)
{
    DrawCommand command = {};
    command.vertex_type = VertexType_Simple;
    command.draw_call.draw_type = DrawType_Primitive;
    command.draw_call.shader = Shader_Line;
    command.vertex_buffer_offset = vertex_buffer_start;
    command.num_vertices = num_vertices;
    command.layer = params.draw_layer;
    command.draw_aabb = aabb;

    if (params.line_draw_flags & PrimitiveDraw_Filled)
    {
        command.draw_call.line.draw_method = GL_TRIANGLE_STRIP;
    }
    else
    {
        command.draw_call.line.draw_method = (params.line_draw_flags & PrimitiveDraw_Looped) ? GL_LINE_LOOP : GL_LINE_STRIP;
    }

    command.draw_call.draw_options |= params.line_draw_flags;

    if(params.line_width)
    {
        command.draw_call.draw_options |= PrimitiveDraw_CustomWidth;
        command.draw_call.line.line_width = params.line_width;
    }

    command.draw_call.draw_options |= params.line_draw_flags & Draw_ScreenSpace;

    PushDrawCommand(renderer, command);
}

void DrawLine(Renderer* ren, SimpleVertex* verts, uint32 num_vertices, PrimitiveDrawParams params)
{
    size_t buffer_start = ren->vertex_buffer.used;
    SimpleVertex* v = PushArray(&ren->vertex_buffer, SimpleVertex, num_vertices);
    memcpy(v, verts, sizeof(SimpleVertex) * num_vertices);

    Vec2 min = { verts[0].position.x, verts[0].position.y };
    Vec2 max = min;
    foru_in(1, num_vertices)
    {
        SimpleVertex* vert = verts + i;
        min.x = Minimum(min.x, vert->position.x);
        min.y = Minimum(min.y, vert->position.y);
        max.x = Maximum(max.x, vert->position.x);
        max.y = Maximum(max.y, vert->position.y);
    }

    Rectf aabb;
    aabb.x = min.x;
    aabb.y = min.y;
    aabb.w = max.x - min.x;
    aabb.h = max.y - min.y;

    DrawPrimitive(ren, buffer_start, num_vertices, params, aabb);
}

void DrawRect(Renderer* renderer, Rectf rect, Vec4 color, float rotation, PrimitiveDrawParams params)
{
    Rectf aabb;
    Vec2_4 points = RotatedRect(rect, rotation, &aabb);

    size_t start_offset = renderer->vertex_buffer.used;
    SimpleVertex* verts = PushArray(&renderer->vertex_buffer, SimpleVertex, 4);

    if (params.line_draw_flags & PrimitiveDraw_Filled)
    {
        SimpleVertex v;
        v.color = color;

        v.position = points.e[0];
        *verts++ = v;

        v.position = points.e[1];
        *verts++ = v;

        v.position = points.e[3];
        *verts++ = v;

        v.position = points.e[2];
        *verts = v;
    }
    else
    {
        SimpleVertex v;
        v.color = color;
        v.position = points.e[0];
        *verts++ = v;

        v.position = points.e[1];
        *verts++ = v;

        v.position = points.e[2];
        *verts++ = v;

        v.position = points.e[3];
        *verts = v;

        params.line_draw_flags |= PrimitiveDraw_Looped;
    }

    DrawPrimitive(renderer, start_offset, 4, params, aabb);
}
