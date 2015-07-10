#pragma once

#include "glew.h"
#include "types.h"
#include "mathops.h"
#include "window.h"
#include "IDrawer.h"
#include "helper.h"
#include "camera.h"
#include "dynamic_array.h"

// GLM includes
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/transform.hpp>
#include "glm/mat4x4.hpp"

enum class ImageFiles  : uint32 { MAIN_IMAGE, MARIO_IMAGE, TEXT_IMAGE, PARTICLE_IMAGE, IMAGE_COUNT }; 
enum class ShaderTypes : uint32 { DEFAULT_SHADER, PARTICLE_SHADER, SHADER_COUNT }; // TODO(cgenova): text shader -> simple, with color option
enum class DrawLayer   : uint32 { FOREGROUND, BACKGROUND, LAYER_COUNT };

struct Sprite
{
	ImageFiles image_file;
	ShaderTypes shader_type;
	DrawLayer layer;
	Vector2 world_size;
	float angle;
	Rect tex_rect;
	Vector4 color_mod;
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

class Renderer : public IDrawer
{
public:
	static glm::mat4 proj_matrix;
	static glm::mat4 view_matrix;
	static glm::mat4 vp_matrix;


	Renderer(Window* w, Vector4 clear_color = Vector4(0, 0, 0, 1));
	virtual ~Renderer() {};

	static void create_instance(Window*);
	static Renderer* get();

	void set_camera(Camera*);
	void begin_frame();
	void end_frame();
	void set_clear_color(Vector4);
	void force_color_clear();
	void load_image(char*, ImageFiles);
	void load_shader(char*, char*, ShaderTypes);

	Dimension get_resolution();

	// virtual void draw(Sprite*);
	// virtual void draw(Animation*);

	void render_draw_buffer();
	virtual void draw_sprite(Sprite*, Vector2);
	virtual void draw_animation(Animation*, Transform*, float time);

	void activate_texture(ImageFiles i) { glBindTexture(GL_TEXTURE_2D, textures[(uint32) i].texture_handle); }
	void activate_shader(ShaderTypes s) { glUseProgram(shaders[(uint32)s].shader_handle); }

	void draw_character(char, uint32, uint32);
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

private:
	struct DrawBufferObject // Used for drawing to the screen.
	{
		ImageFiles image;
		ShaderTypes shader;
		DrawLayer layer;

		Rect tex_rect;
		Vector2 world_size;
		Vector3 world_position;
		Vector4 color_mod;
		float draw_angle;
	};

	struct DrawObject
	{
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;
		uint8* memory = nullptr;
		uint32 memory_size = 0;
		Vector3* vert_positions = nullptr;
		Vector2* tex_coords = nullptr;

		~DrawObject() { if(memory) delete[] memory; }
	};

	void draw_call(DrawBufferObject);
	void build_buffer_object();


	// NOTE(chris): should this be multiple different draw buffers for each layer?
	// DynamicArray<DrawBufferObject> draw_buffer[LAYER_COUNT];
	Dimension frame_resolution;

	TextData text_data;

	DynamicArray<DrawBufferObject> draw_buffer;
	DrawObject draw_object;

	Camera* main_camera;
	Window* draw_window;

	static Renderer* s_instance;
};