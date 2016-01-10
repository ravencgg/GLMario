#pragma once

#include "win32_gl.h"

#include "types.h"
#include "containers.h"
#include "mathops.h"
#include "utility.h"

#include <vector>

struct Window;
struct GameState;

enum ImageFiles  : uint32 { MAIN_IMAGE, MARIO_IMAGE, TEXT_IMAGE, PARTICLE_IMAGE, IMAGE_COUNT };
enum ShaderTypes : uint32 { Shader_Default, Shader_Text, Shader_Particle, Shader_Line, Shader_Count };
enum DrawLayer   : uint32 { DrawLayer_Background, DrawLayer_PreTilemap, DrawLayer_Tilemap, DrawLayer_PostTilemap, DrawLayer_Player, DrawLayer_Foreground, DrawLayer_UI, DrawLayer_Debug, DrawLayer_Count };

struct Camera
{
    Vec2 position;
    Vec2 viewport_size;
};

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

enum LineDrawFlags : uint32
{
    LineDraw_Default      = 0x0,
    LineDraw_Looped       = 0x1,
    LineDraw_ScreenSpace  = 0x2,// Uses [0,1] coords instead of world space coordinates
    LineDraw_CustomWidth  = 0x4,// top byte of the uint32 holding these flags data is the size in pixels of the line;
    LineDraw_Smooth       = 0x8,
};

struct LineDrawParams
{
    uint32 line_draw_flags = LineDraw_Default;
    DrawLayer draw_layer   = DrawLayer_UI;
    uint8 line_width       = 3;
};

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

// TODO: Change this to a system that stores indices in an array of line buffer data.
struct LineBufferData
{
	GLuint vao;
	GLuint vbo;
	GLuint draw_method;
    uint32 line_draw_flags;
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

struct TextDrawResult
{
	uint32 lines_drawn;
	Vec2 bottom_right;
};

struct TextData
{
    uint32 chars_per_line;
    Vec2i char_size;
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

struct Renderer
{
	Vec2i frame_resolution;
	TextData text_data;
    TextVertex* text_array;
    size_t text_array_size;
    uint32 line_buffer_loc = 0;

    Texture textures[(uint32) ImageFiles::IMAGE_COUNT];
    Shader shaders[Shader_Count];

    // TODO: remove this, make a single buffer for lines and draw out of that with draw commands
    std::vector<LineBufferData> line_buffer;
    // TODO: switch to frame buffer, then sort before drawing?
	Array<DrawCall> draw_buffer[DrawLayer_Count];

    Camera* camera;
};

void SwapBuffer(GameState*);

Renderer* CreateRenderer(MemoryArena* arena);
void LoadImage(Renderer*, char*, ImageFiles);
void LoadShader(Renderer*, char*, char*, ShaderTypes);

void BeginFrame(Renderer*, Window*);
void Flush(Renderer*);
void ForceColorClear();
void SetClearColor(Vec4);

Vec2i GetResolution(Renderer*);
float ViewportWidth(Renderer*);
void RenderDrawBuffer(Renderer*);

TextDrawResult DrawString(Renderer*, char* string, uint32 string_size, float start_x, float start_y, StringTextColor* = 0, size_t = 0);
void PushDrawCall(Renderer*, DrawCall, DrawLayer);
//void DrawCall(Renderer*, DrawCall);
void DrawLine(Renderer*, Vec2, Vec2, Vec4, LineDrawParams* params = nullptr);
void DrawLine(Renderer*, Array<SimpleVertex>& vertices, LineDrawParams* params = nullptr);
void DrawRect(Renderer*, const Rectf&, Vec4 color, LineDrawParams* params = nullptr);
void DrawRotatedRect(Renderer*, const Rectf&, float rotation, Vec4 color, LineDrawParams* params = nullptr);


