#include "tilemap.h"
#include "physics.h"

Tile MakeTile(Physics* physics, Vec2 position, Vec2 size, TileType tile_type)
{
    assert(physics);
    Tile result = {};
    result.position = position;
    result.size = size; 
    result.tile_type = tile_type;

    Rectf collider = { (float) position.x - size.x / 2.f, (float) position.y - size.y / 2.f, size.x, size.y };
    result.collider = physics->AddStaticCollider(collider);
    return result;
}

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
                    Vec2 p = vec2((float) x, (float) y);
                    Vec2 s = vec2(1, 1);
                    tiles.push_back(MakeTile(physics, p, s));
                }
			}	
			else
			{
				if (x & 1)
				{
                    Vec2 p = vec2((float) x, (float) y);
                    Vec2 s = vec2(1, 1);
                    tiles.push_back(MakeTile(physics, p, s));
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
                Vec2 p = vec2((float) x, (float) y);
                Vec2 s = vec2(1, 1);
                tiles.push_back(MakeTile(physics, p, s));
			}
			else if(x == 0 || x == (r.width - 1))
			{
                Vec2 p = vec2((float) x, (float) y);
                Vec2 s = vec2(1, 1);
                tiles.push_back(MakeTile(physics, p, s));
				tiles.back().collider.data->active = false;
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
        draw_call.sd.world_position = tiles[i].position;
        draw_call.sd.world_size = tiles[i].size;
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
