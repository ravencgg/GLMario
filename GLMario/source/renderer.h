#pragma once

#include "glew.h"
#include "types.h"
#include "mathops.h"
#include "window.h"
#include "IDrawer.h"
#include "helper.h"
#include "camera.h"
#include "dynamic_array.h"

enum ImageFiles  { MAIN_IMAGE, IMAGE_COUNT }; 
enum ShaderTypes { DEFAULT_SHADER, SHADER_COUNT };
enum DrawLayer   { FOREGROUND, BACKGROUND, LAYER_COUNT };

struct Sprite
{
	ImageFiles image_file;
	ShaderTypes shader_type;
	DrawLayer layer;
	Vector2 world_size;
	float angle;
	Rect tex_rect;
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

class Renderer : public IDrawer
{
public:

	Renderer(Window* w, Vector4 clear_color = Vector4(0, 0, 0, 1));

	void set_camera(Camera*);
	void begin_frame();
	void end_frame();
	void set_clear_color(Vector4);
	void load_image(char*, ImageFiles);
	void load_shader(char*, char*, ShaderTypes);

	// virtual void draw(Sprite*);
	// virtual void draw(Animation*);

	virtual void draw_sprite(Sprite*, Transform*);
	virtual void draw_animation(Animation*, Transform*, float time);

	static char* default_frag_shader;
	static char* default_vert_shader;
	static char* main_image;
	static uint32 pixels_to_meters;

private:

	struct Texture
	{
		int32 w, h, bytes_per_color;
		GLuint texture_handle;
	};

	struct Shader
	{
		GLuint shader_handle;
	};

	struct DrawBufferObject
	{
		ImageFiles image;
		ShaderTypes shader;
		DrawLayer layer;

		Rect tex_rect;
		Vector2 world_size;
		Vector3 world_position;
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
		Vector4* colors = nullptr;
		Vector2* tex_coords = nullptr;

		~DrawObject() { if(memory) delete[] memory; }
	};

	void render_draw_buffer();
	void draw_call(DrawBufferObject);
	void build_buffer_object();

	Texture textures[ImageFiles::IMAGE_COUNT];
	Shader shaders[ShaderTypes::SHADER_COUNT];

	// NOTE(chris): should this be multiple different draw buffers for each layer?
	// DynamicArray<DrawBufferObject> draw_buffer[LAYER_COUNT];
	DynamicArray<DrawBufferObject> draw_buffer;
	DrawObject draw_object;

	Camera* main_camera;

	Window* draw_window;
};