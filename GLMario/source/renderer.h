#pragma once
#include <vector>

#include "glew.h"
#include "mathops.h"
#include "types.h"
#include "game_types.h"
#include "utility.h"
#include "containers.h"

class Camera;
struct Sprite;
struct Animation;
struct Transform;


enum ImageFiles  : uint32 { MAIN_IMAGE, MARIO_IMAGE, TEXT_IMAGE, PARTICLE_IMAGE, IMAGE_COUNT };
 // TODO(cgenova): text shader -> simple, with color option
enum ShaderTypes : uint32 { Shader_Default, Shader_Text, Shader_Particle, Shader_Line, Shader_Count };
enum DrawLayer   : uint32 { DrawLayer_Background, DrawLayer_PreTilemap, DrawLayer_Tilemap, DrawLayer_PostTilemap, DrawLayer_Player, DrawLayer_Foreground, DrawLayer_UI, DrawLayer_Count };
enum class SpriteRect  : uint32 { UNINITIALIZED, STONE, BRICK, MARIO, RECT_COUNT };

Rect sprite_rects[];

void initialize_sprite_rects();
Rect get_sprite_rect(SpriteRect);

namespace DrawOptions
{
	enum : uint32  {
		WHOLE_TEXTURE = 0x1, // TODO(cgenova): remove this? pointless with flags
		TEXTURE_RECT  = 0x2,
	};
}

namespace LineDrawOptions
{
    enum : uint32 {
        NONE          = 0x0,
        LOOPED        = 0x1,
        SCREEN_SPACE  = 0x2, // Uses [0,1] coords instead of world space coordinates
        CUSTOM_SIZE   = 0x4, // top byte of the uint32 holding these flags data is the size in pixels of the line;
        SMOOTH        = 0x8,
    };
}

namespace DrawType
{
	enum Type : uint32 { UNINITIALIZED, SINGLE_SPRITE, LINE_BUFFER, ARRAY_BUFFER, PARTICLE_ARRAY_BUFFER, DRAW_TYPE_COUNT};
}

// Pass color in as a uniform if it is desired
struct SpriteVertex
{
    Vec2 position;
    Vec2 uv;
};

enum StringColorOptions
{
    StringColorOptions_Solid,
    StringColorOptions_Gradient,
};

struct StringTextColor
{
    uint32 color_options;
    uint32 start;
    uint32 end;

    union
    {
        struct
        {
            Vec4 solid_color; // = { 1.0f, 1.0f, 1.0f, 1.0f };
        };
        struct
        {
            Vec4 gradient_start;
            Vec4 gradient_end;
        };
    } c;
};

struct TextVertex
{
    float x, y, tx, ty;
    Vec4 color;
};

struct Vertex
{
    Vec2 position;
    Vec2 uv;
    Vec4 color;
};

struct SimpleVertex
{
    Vec2 position;
    Vec4 color;

    SimpleVertex() {}
    SimpleVertex(Vec2 p, Vec4 c) : position(p), color(c) {}
};

struct SpriteData
{
	Rect tex_rect;
	Vec2 world_size;
	Vec2 world_position;
	Vec4 color_mod;
	float draw_angle;
};

struct ArrayBufferData
{
	GLuint vao;
	GLuint vbo;
	GLuint draw_method;
	uint32 num_vertices;
    bool screen_space;  // @cleanup Combine array and line buffer drawing? Possibly
                        // even particle buffer stuff, use flags to control it all
};

struct LineBufferData
{
	GLuint vao;
	GLuint vbo;
	GLuint draw_method;
    uint32 line_draw_options;
	uint32 num_vertices;
};

struct DrawCall// Used for drawing to the screen.
{
	DrawType::Type draw_type;

	ImageFiles image;
	ShaderTypes shader;
	uint32 options;

	union
	{
		SpriteData sd;
        LineBufferData lbd;
		ArrayBufferData abd;
	};
};

struct Sprite
{
	ImageFiles image_file;
	ShaderTypes shader_type;
	DrawLayer layer;
	Vec2 world_size;
	float angle;
	Rect tex_rect;
	Vec4 color_mod;
};

enum class AnimationMode { LOOP, PING_PONG, STOP };
struct Animation
{
	ImageFiles image_file;
	Dimension size;
	DrawLayer layer;
	Point2 start_loc;

	uint32 frame_time;
	uint32 num_frames;
	AnimationMode animation_mode;
};

struct TextDrawResult
{
	uint32 lines_drawn;
	Vec2 bottom_right;
};

struct TextData
{
    uint32 chars_per_line;
    Dimension char_size;
};

struct Texture
{
    int32 w, h, bytes_per_color;
    GLuint texture_handle;
};

struct Shader
{
    GLuint shader_handle;
};

// TODO: break this up into logical components
//  -> Text drawing
//  -> Particles?
class Renderer
{
public:
	Renderer(Window* w, Vec4 clear_color = vec4(0.1f, 0.1f, 0.1f, 1));
	virtual ~Renderer() {};

	static void create_instance(Window*);
	static Renderer* get();

	void set_camera(Camera*);
	void begin_frame();
	void Flush();
    void SwapBuffer();
	void set_clear_color(Vec4);
	void force_color_clear();
	void load_image(char*, ImageFiles);
	void load_shader(char*, char*, ShaderTypes);

	Dimension get_resolution();
	float viewport_width();

	void render_draw_buffer();
	// void draw_sprite(Sprite*, Vec2);
	void draw_animation(Animation*, Transform*, float time);

	void activate_texture(ImageFiles i) { glBindTexture(GL_TEXTURE_2D, textures[(uint32) i].texture_handle); }
	void activate_shader(ShaderTypes s) { glUseProgram(shaders[(uint32)s].shader_handle); }

	TextDrawResult DrawString(char* string, uint32 string_size, float start_x, float start_y,
                                StringTextColor* = 0, size_t = 0);


	Texture textures[(uint32) ImageFiles::IMAGE_COUNT];
	Shader shaders[Shader_Count];

	void push_draw_call(DrawCall, DrawLayer);
	void draw_call(DrawCall);
    void DrawLine(Vec2, Vec2, Vec4, uint8 line_width, DrawLayer dl = DrawLayer_UI, uint32 line_draw_options = 0);
    // TODO: Get rid of the std::vector version of this and just use the Array one.
    void DrawLine(std::vector<SimpleVertex>& vertices, uint8 line_width, DrawLayer dl, uint32 line_draw_options = 0);

    void DrawLine(Array<SimpleVertex>& vertices, uint8 line_width, DrawLayer dl, uint32 line_draw_options = 0);
	void DrawRect(Rectf&, uint8 line_width = 4, DrawLayer dl = DrawLayer_UI, Vec4 color = vec4(1, 1, 1, 1), uint32 line_draw_options = 0);

private:

	Dimension frame_resolution;

	TextData text_data;

    TextVertex* text_array;
    size_t text_array_size;

    uint32 line_buffer_loc = 0;
    std::vector<LineBufferData> line_buffer;
	Array<DrawCall> draw_buffer[DrawLayer_Count];

	Camera* main_camera;
	Window* draw_window;

	static Renderer* s_instance;
};

void SwapBuffer(GameState*);
