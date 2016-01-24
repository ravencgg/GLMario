#pragma once

// TODO: Remove this from public visibility
#include "win32_gl.h"

#include "types.h"
#include "mathops.h"
#include "utility.h"

struct Window;
struct GameState;

enum ImageFiles  : uint32 { MAIN_IMAGE, MARIO_IMAGE, TEXT_IMAGE, PARTICLE_IMAGE, IMAGE_COUNT };
enum ShaderTypes : uint32 { Shader_Default, Shader_Text, Shader_Particle, Shader_Line, Shader_Count };
enum DrawLayers  : uint32 { DrawLayer_Background, DrawLayer_PreTilemap, DrawLayer_Tilemap, DrawLayer_PostTilemap, DrawLayer_Player, DrawLayer_Foreground, DrawLayer_UI, DrawLayer_Debug, DrawLayer_Count };

struct Camera
{
    Vec2 position;
    Vec2 viewport_size;
};


#define MAX_SUB_LAYER 0x00FFFFFF
struct DrawLayer
{
    union
    {
        struct
        {
            uint32 sub_layer : 24;
            DrawLayers layer : 8;
        };
        uint32 sort_key;
    };
};

inline DrawLayer SetLayer(DrawLayers layer)
{
    DrawLayer result = {};
    result.layer = layer;
    return result;
}

inline DrawLayer SetLayer(DrawLayers layer, uint32 sub_layer)
{
    DrawLayer result;
    result.layer = layer;
    result.sub_layer = sub_layer;
    return result;
}

enum StringOptions
{
    String_HorAlignLeft   = 0x1,
    String_HorAlignRight  = 0x2,
    String_HorAlignCenter = 0x3, // Both left and right flags!

    String_VerAlignTop    = 0x10,
    String_VerAlignBottom = 0x20,
    String_VerAlignCenter = 0x30,// Both top and bottom flags!

    String_AlignCenter    = String_HorAlignCenter | String_VerAlignCenter,

//    String_NoClipping     = 0x100,
// String_DrawPartial       = 0x200, // Compute the correct location for the full string, but only show part of it?
//                                      Could be used for text boxes
};

enum DrawOptions
{
    Draw_ScreenSpace             = 0x1,       // Uses [0,1] coords instead of world space coordinates

	Draw_TextureWhole            = 0x10, // TODO(cgenova): remove this? pointless with flags
	Draw_TextureRect             = 0x20,

    PrimitiveDraw_Looped         = 0x100,
    PrimitiveDraw_Filled         = 0x200,
    PrimitiveDraw_CustomWidth    = 0x400,// top byte of the uint32 holding these flags data is the size in pixels of the line;
    PrimitiveDraw_Smooth         = 0x800,

    StringColorOptions_Solid     = 0x1000,
    StringColorOptions_Gradient  = 0x2000,
};


struct PrimitiveDrawParams
{
    uint32 line_draw_flags;
    DrawLayer draw_layer;
    uint8 line_width;
};

enum DrawType
{
    DrawType_Unknown,
    DrawType_Sprite,
    DrawType_Primitive,
    DrawType_Particles,
    DrawType_Text,
};


// Pass color in as a uniform if it is desired
struct SpriteVertex
{
    Vec2 position;
    Vec2 uv;
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

struct SimpleVertex
{
    Vec2 position;
    Vec4 color;

    SimpleVertex() {}
    SimpleVertex(Vec2 p, Vec4 c) : position(p), color(c) {}
};

struct DrawCall// Used for drawing to the screen.
{
	DrawType draw_type;
	ShaderTypes shader;
	uint32 draw_options;

	union
	{
        struct
        {
            ImageFiles image;
            Rect tex_rect;
            Vec2 world_size;
            Vec2 world_position;
            Vec4 color_mod;
            float draw_angle;
        }sprite;

        struct
        {
            ImageFiles image;

        } text;
        struct
        {
            uint8 line_width;
            GLuint draw_method;
        } line;

        struct
        {
            ImageFiles image;
            GLuint vao;
            GLuint vbo;
            GLuint draw_method;
            uint32 num_vertices;
        } owning_buffer;
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
    Vec2i texture_size;
};

struct Texture
{
    Vec2i size;
    int32 bytes_per_color;
    GLuint texture_handle;
};

struct Shader
{
    GLuint shader_handle;
};

enum VertexType
{
    VertexType_Null,  //Used for things that draw and own their own VBOs
    VertexType_Sprite,
    VertexType_Line,
    VertexType_Text,
    VertexType_Simple,

    VertexType_Count,
};

struct DrawCommand
{
    VertexType vertex_type;
    DrawCall draw_call;
    DrawLayer layer;
    size_t vertex_buffer_offset;
    uint32 num_vertices;
    Rectf draw_aabb;
};

struct CommandBuffer
{
    DrawCommand* draw_commands;
    size_t num_commands;
    size_t command_array_size;
};

struct Renderer
{
	Vec2i frame_resolution;
	TextData text_data;

    Texture textures[(uint32) ImageFiles::IMAGE_COUNT];
    Shader shaders[Shader_Count];

    MemoryArena vertex_buffer;
    CommandBuffer command_buffer;
};

inline PrimitiveDrawParams DefaultPrimitiveDrawParams()
{
    PrimitiveDrawParams result = {};

    result.line_draw_flags      = 0;
    result.draw_layer.layer     = DrawLayer_UI;
    result.draw_layer.sub_layer = 0;
    result.line_width           = 1;
    return result;
}

void SwapBuffer(GameState*);

Renderer* CreateRenderer(MemoryArena* arena);
void LoadImage(Renderer*, char*, ImageFiles);
void LoadShader(Renderer*, char*, char*, ShaderTypes);

void BeginFrame(Renderer*, Window*);
void Flush(Renderer*, Camera*);
void ForceColorClear();
void SetClearColor(Vec4);

Vec2i GetResolution(Renderer*);
float ViewportWidth(Renderer*);
void RenderDrawBuffer(Renderer*, Camera*);

void DrawStringInRect(Renderer* ren, char* string, Rectf bounds, Vec2 text_size, Vec4 color, uint32 options, DrawLayer draw_layer);

TextDrawResult DrawString(Renderer*, char* string, uint32 string_size, float start_x, float start_y, 
                            StringTextColor* = 0, size_t = 0, DrawLayer draw_layer = SetLayer(DrawLayer_UI));

void PushDrawCommand(Renderer*, DrawCommand);

void DrawSprite(Renderer*, DrawCall, DrawLayer);

void DrawLine(Renderer* ren, SimpleVertex* verts, uint32 num_vertices, PrimitiveDrawParams params = DefaultPrimitiveDrawParams());

void DrawLine(Renderer*, Vec2, Vec2, Vec4, PrimitiveDrawParams params = DefaultPrimitiveDrawParams());

void DrawRect(Renderer*, Rectf, Vec4 color, float rotation = 0, PrimitiveDrawParams params = DefaultPrimitiveDrawParams());

