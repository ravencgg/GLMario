#pragma once
#include <vector>

#include "glew.h"
#include "types.h"
#include "mathops.h"
#include "window.h"
#include "IDrawer.h"
#include "helper.h"
#include "dynamic_array.h"

// GLM includes
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/transform.hpp>
#include "glm/mat4x4.hpp"

class Camera;

enum class ImageFiles  : uint32 { MAIN_IMAGE, MARIO_IMAGE, TEXT_IMAGE, PARTICLE_IMAGE, IMAGE_COUNT }; 
enum class ShaderTypes : uint32 { DEFAULT_SHADER, PARTICLE_SHADER, SHADER_COUNT }; // TODO(cgenova): text shader -> simple, with color option
enum class DrawLayer   : uint32 { BACKGROUND, PRE_TILEMAP, TILEMAP, POST_TILEMAP, PLAYER, FOREGROUND, UI, LAYER_COUNT };

enum class SpriteRect  : uint32 { UNINITIALIZED, STONE, BRICK, MARIO, RECT_COUNT };

Rect sprite_rects[];

void initialize_sprite_rects();
Rect get_sprite_rect(SpriteRect);

namespace DrawOptions
{
	enum : uint32  {
		WHOLE_TEXTURE = 0x1, // TODO(cgenova): remove this? pointless with flags
		TEXTURE_RECT = 0x2,
	};
}
 
namespace DrawType
{
	enum Type : uint32 { UNINITIALIZED, SINGLE_SPRITE, ARRAY_BUFFER, PARTICLE_ARRAY_BUFFER, DRAW_TYPE_COUNT};
}

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
};
//
//struct ArrayBufferData
//{
//	GLuint vao;
//	GLuint vbo;
//	GLuint draw_method;
//	uint32 num_vertices;
//};

struct DrawCall// Used for drawing to the screen.
{
	DrawType::Type draw_type;

	ImageFiles image;
	ShaderTypes shader;
	uint32 options;

	union 
	{
		SpriteData sd;
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
	Point2 bottom_right;
};

class Renderer
{
public:
	static glm::mat4 proj_matrix;
	static glm::mat4 view_matrix;
	static glm::mat4 vp_matrix;

	Renderer(Window* w, Vec4 clear_color = vec4(0, 0, 0, 1));
	virtual ~Renderer() {};

	static void create_instance(Window*);
	static Renderer* get();

	void set_camera(Camera*);
	void begin_frame();
	void end_frame();
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

	void draw_character(char, uint32, uint32, glm::mat4&, glm::mat4&);
	TextDrawResult draw_string(std::string, uint32 x, uint32 y);

	static char* default_frag_shader;
	static char* default_vert_shader;
	static char* particle_vert_shader;
	static char* particle_frag_shader;
	static char* main_image;
	static char* mario_image;
	static char* text_image;
	static char* particle_image;
	static const uint32 pixels_to_meters;

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

	Texture textures[(uint32) ImageFiles::IMAGE_COUNT];
	Shader shaders[(uint32) ShaderTypes::SHADER_COUNT];

	void push_draw_call(DrawCall, DrawLayer);
	void draw_call(DrawCall);
	void draw_line(std::vector<Vec2> points, DrawLayer layer);
	void draw_line(Vec2&, Vec2&, DrawLayer dl = DrawLayer::UI);
	void draw_rect(Rectf&, DrawLayer dl = DrawLayer::UI);
private:
	struct DrawObject
	{
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;
		uint8* memory = nullptr;
		uint32 memory_size = 0;
		Vec3* vert_positions = nullptr;
		Vec2* tex_coords = nullptr;

		~DrawObject() { if(memory) delete[] memory; }
	};

	void build_buffer_object();

	Dimension frame_resolution;

	TextData text_data;

	DynamicArray<DrawCall> draw_buffer[(uint32)DrawLayer::LAYER_COUNT];
	DrawObject draw_object;

	Camera* main_camera;
	Window* draw_window;

	static Renderer* s_instance;
};
	
