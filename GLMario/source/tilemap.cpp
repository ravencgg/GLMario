#include "tilemap.h"
#include "physics.h"

Tilemap::Tilemap(Physics* p)
{
    physics = p;
	init();
}

Tilemap::~Tilemap()
{
}

void Tilemap::MakeCheckerboard(Rect r)
{
	for(int32 y = 0; y < r.height; ++y)
	{
		for(int32 x = 0; x < r.width; ++x)
		{
			if(y & 1)
			{
				if(!(x & 1)) 
                {
                    Rectf position = { (float) r.left + x, (float) r.top + y, 1, 1 };
                    Tile tile = {};
                    tile.position = position; 
                    tile.tile_type = BRICK;
                    tile.collider = physics->AddStaticCollider(position);
                    tiles.push_back(tile);
                }
			}	
			else
			{
				if (x & 1)
				{
                    Rectf position = { (float) r.left + x, (float) r.top + y, 1, 1 };
                    Tile tile = {};
                    tile.position = position; 
                    tile.tile_type = BRICK;
                    tile.collider = physics->AddStaticCollider(position);
                    tiles.push_back(tile);
				}
			}
		}
	}	
}

void Tilemap::MakeWalledRoom(Rect r)
{
	for (int32 y = 0; y < r.height; ++y)
	{
		for (int32 x = 0; x < r.width; ++x)
		{
			if(y == 0 || y == (r.height - 1))
			{
                Rectf position = { (float)r.left + x, (float)r.top + y, 1, 1 };
                Tile tile = {};
                tile.position = position; 
                tile.tile_type = BRICK;
                tile.collider = physics->AddStaticCollider(position);
                tiles.push_back(tile);
			}
			else if(x == 0 || x == (r.width - 1))
			{
                Rectf position = { (float) r.left + x, (float) r.top + y, 1, 1 };
                Tile tile = {};
                tile.position = position; 
                tile.tile_type = BRICK;
                tile.collider = physics->AddStaticCollider(position);
                tiles.push_back(tile);
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

    for(uint32 i = 0; i < tiles.size(); ++i)
    {
        draw_call.sd.world_position = { tiles[i].position.x, tiles[i].position.y };
        ren->push_draw_call(draw_call, DrawLayer::TILEMAP);
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
