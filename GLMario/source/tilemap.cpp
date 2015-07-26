#include "tilemap.h"
#include "physics.h"

Tilemap::Tilemap()
{
	init();
}

Tilemap::Tilemap(int32 w, int32 h)
{
	assert(w > 0 && h > 0);
	init();
	width = w;
	height = h;

	tiles = new Tile[w * h];
	memset(tiles, 0, sizeof(*tiles) * w * h);
}

Tilemap::~Tilemap()
{
	if(tiles) delete[] tiles;
}

void Tilemap::fill_checkerboard()
{
	for(int32 y = 0; y < height; ++y)
	{
		for(int32 x = 0; x < width; ++x)
		{
			uint32 loc = array_loc(x, y);

			if(y & 1)
			{
				if(x & 1) tiles[loc].tile_type = EMPTY;
				else tiles[loc].tile_type = BRICK;
			}	
			else
			{
				if(x & 1) tiles[loc].tile_type = BRICK;
				else tiles[loc].tile_type = EMPTY;
			}
		}
	}	
}

void Tilemap::fill_walled_room()
{
	for (int32 y = 0; y < height; ++y)
	{
		for (int32 x = 0; x < width; ++x)
		{
			uint32 loc = array_loc(x, y);

			if(y == 0 || y == (height - 1))
			{
				tiles[loc].tile_type = BRICK;
			}
			else if(x == 0 || x == (width - 1))
			{
				tiles[loc].tile_type = BRICK;
			}
			else
			{
				tiles[loc].tile_type = EMPTY;
			}
		}
	}
}

void Tilemap::update()
{
}

void Tilemap::draw()
{
	static Rect brick_rect = { 85, 0, tile_width, tile_height };
	static Rect ground_rect = { 0, 0, tile_width, tile_height }; 

	Vec2 position;

	for(int32 y = 0; y < height; ++y)
	{
		for(int32 x = 0; x < width; ++x)
		{
			uint32 loc = array_loc(x, y);

			position.x = (float)x;
			position.y = (float)y;

			//position.x = (float) (rand() % 20 - 10);
			//position.y = (float) (rand() % 20 - 10);


			if(tiles[loc].tile_type != EMPTY)
			{
				draw_call.sd.world_position = { position.x, position.y };
				ren->push_draw_call(draw_call, DrawLayer::TILEMAP);
			}
		}
	}
}

void Tilemap::init()
{
	ren = Renderer::get();	
	tile_width = 16;
	tile_height = 16;

	draw_call = {};
	draw_call.draw_type = DrawType::SINGLE_SPRITE;
	draw_call.image = ImageFiles::MAIN_IMAGE; 
	draw_call.shader = ShaderTypes::DEFAULT_SHADER;
	draw_call.options = DrawOptions::TEXTURE_RECT;
	draw_call.sd.tex_rect = get_sprite_rect(SpriteRect::STONE);
	
	draw_call.sd.world_size = vec2(1.f, 1.f);
	draw_call.sd.world_position = { 0, 0 };
	draw_call.sd.draw_angle = 0;
}

uint32 Tilemap::array_loc(uint32 x, uint32 y) 
{
	uint32 result = y * width + x; 
	return result;
}
