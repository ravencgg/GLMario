#include "tilemap.h"

Tilemap::Tilemap()
{
	init();
	width = 0;
	height = 0; 
}

Tilemap::Tilemap(uint32 w, uint32 h)
{
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
	for(uint32 y = 0; y < height; ++y)
	{
		for(uint32 x = 0; x < width; ++x)
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

void Tilemap::update()
{
}

void Tilemap::draw()
{
	static Rect brick_rect = { 85, 0, tile_width, tile_height };
	static Rect ground_rect = { 0, 0, tile_width, tile_height }; 
	static Sprite sprite = { ImageFiles::MAIN_IMAGE,
							 ShaderTypes::DEFAULT_SHADER,
							 DrawLayer::BACKGROUND,
							 Vector2(1.0f, 1.0f),
							 0,
							 brick_rect, 
							 Vector4(1, 1, 1, 1)};
	Vector2 position;

	for(uint32 y = 0; y < height; ++y)
	{
		for(uint32 x = 0; x < width; ++x)
		{
			uint32 loc = array_loc(x, y);

			sprite.image_file = ImageFiles::MAIN_IMAGE;
			position.x = (float)x;
			position.y = (float)y;

			//position.x = (float) (rand() % 20 - 10);
			//position.y = (float) (rand() % 20 - 10);


			if(tiles[loc].tile_type != EMPTY)
			{
				ren->draw_sprite(&sprite, position);
			}
		}
	}


	Console* console = Console::get();
	console->log_message(std::string("Log this, bitch"));
}

void Tilemap::init()
{
	ren = Renderer::get();	
	tile_width = 16;
	tile_height = 16;
}

uint32 Tilemap::array_loc(uint32 x, uint32 y) 
{
	uint32 result = y * width + x; 
	return result;
}
