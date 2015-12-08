#include "tilemap.h"

#include "utility.h"
#include "renderer.h"
#include "containers.h"

void AllocateTileMap(MemoryArena* arena, TileMap* tilemap, uint32 map_width, uint32 map_height)
{
    tilemap->map_width = map_width;
    tilemap->map_adjusted_width = map_width + (TILE_GROUP_SIDE_SIZE - (map_width % TILE_GROUP_SIDE_SIZE));

    tilemap->map_height = map_height;
    tilemap->map_adjusted_height = map_height + (TILE_GROUP_SIDE_SIZE - (map_height % TILE_GROUP_SIDE_SIZE));

    tilemap->hor_groups = tilemap->map_adjusted_width  / TILE_GROUP_SIDE_SIZE;
    tilemap->ver_groups = tilemap->map_adjusted_height / TILE_GROUP_SIDE_SIZE;

    tilemap->tile_groups = PushStructs(arena, TileGroup, tilemap->hor_groups * tilemap->ver_groups);

    TileGroup* group = tilemap->tile_groups;
    for(uint32 y = 0; y < tilemap->ver_groups; ++y)
    {
        for(uint32 x = 0; x < tilemap->hor_groups; ++x)
        {
            group->group_coord_x = x;
            group->group_coord_y = y;

            Tile* tile = group->tiles;
            for(uint32 yy = 0; yy < TILE_GROUP_SIDE_SIZE; ++yy)
            {
                for(uint32 xx = 0; xx < TILE_GROUP_SIDE_SIZE; ++xx)
                {
                    tile->tile_coord_x = x * TILE_GROUP_SIDE_SIZE + xx;
                    tile->tile_coord_y = y * TILE_GROUP_SIDE_SIZE + yy;

                    ++tile;
                }
            }

            ++group;
        }
    }
}

void MakeCheckerboard(TileMap*, Rect)
{

}

void MakeWalledRoom(TileMap*, Rect)
{

}

void AddTile(TileMap*, float, float)
{

}

Tile* FindTile(TileMap* tilemap, uint32 x, uint32 y)
{
    assert(!"probably bogus with the new numbering system");
    assert(x < tilemap->map_width);
    assert(y < tilemap->map_height);

    if(x >= tilemap->map_width || y >= tilemap->map_height)
    {
        return 0;
    }
    uint32 group_x = x / TILE_GROUP_SIDE_SIZE;
    uint32 group_y = y / TILE_GROUP_SIDE_SIZE;

    assert(group_x < tilemap->hor_groups);
    assert(group_y < tilemap->ver_groups);

    uint32 internal_x = x % TILE_GROUP_SIDE_SIZE;;
    uint32 internal_y = y % TILE_GROUP_SIDE_SIZE;;
    uint32 group_loc = group_y * tilemap->hor_groups + group_x;

    assert(group_loc < tilemap->hor_groups * tilemap->ver_groups);
    TileGroup* group = tilemap->tile_groups + group_loc;

    Tile* result = group->tiles + (internal_x + (internal_y * TILE_GROUP_SIDE_SIZE));
    return result;
}

void DrawTileMap(TileMap* tilemap)
{
    uint32 num_groups = tilemap->hor_groups * tilemap->ver_groups;
    uint32 tiles_per_group = TILE_GROUP_SIZE;

    Array<SimpleVertex> line_vertices;
    Vec4 color = vec4(0, 0, 1.0f, 1.0f);

    uint8 color_to_mod = 0;

    line_vertices.AddEmpty(tilemap->map_width * tilemap->map_height);

    uint32 index = 0;

    Renderer* ren = Renderer::get();

    TileGroup* group = tilemap->tile_groups;
    for(uint32 group_num = 0; group_num < num_groups; ++group_num, ++group)
    {
        Tile* tile = group->tiles;

        color.e[color_to_mod] += 0.1f;
        if(color.e[color_to_mod] > 1.f)
        {
            color.e[color_to_mod] = 0;
        }
        color_to_mod ^= 1;

        for(uint32 tile_num = 0; tile_num < tiles_per_group; ++tile_num, ++tile)
        {
            float x = (float)tile->tile_coord_x;
            float y = (float)tile->tile_coord_y;

            Rectf r = { x, y, 1.f, 1.f };
            if(tile->tile_coord_x > tilemap->map_width || tile->tile_coord_y > tilemap->map_height)
            {
                ren->DrawRect(r, 2, DrawLayer_UI, vec4(1.0f, 0, 0, 1.f));
            }
            else
            {
                ren->DrawRect(r, 2, DrawLayer_UI, color);
            }

        }
    }

    const uint8 line_width = 3;
    ren->DrawLine(line_vertices, line_width, DrawLayer_UI, LineDrawOptions::CUSTOM_SIZE);
}


#if 0

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
                    tiles.Add(MakeTile(physics, p, s));
                }
			}
			else
			{
				if (x & 1)
				{
                    Vec2 p = vec2((float) x, (float) y);
                    Vec2 s = vec2(1, 1);
                    tiles.Add(MakeTile(physics, p, s));
				}
			}
		}
	}
}

void Tilemap::MakeWalledRoom(Rect r)
{


	for (int32 y = r.y, j = 0; j < r.height; ++y, ++j)
	{
		for (int32 x = r.x, i = 0; i < r.width; ++x, ++i)
		{
			if(y == r.y || j == (r.height - 1))
			{
                Vec2 p = vec2((float) x, (float) y);
                Vec2 s = vec2(1, 1);
                tiles.Add(MakeTile(physics, p, s));
			}
			else if(x == r.x || i == (r.width - 1))
			{
                Vec2 p = vec2((float) x, (float) y);
                Vec2 s = vec2(1, 1);
                tiles.Add(MakeTile(physics, p, s));
				// tiles.back().collider.data->active = false;
			}
		}
	}
}

void Tilemap::AddTile(float x, float y)
{
    Vec2 p = vec2(x, y);
    Vec2 s = vec2(1, 1);
    tiles.Add(MakeTile(physics, p, s));
    tiles.GetBack().collider->active = true;
}

void Tilemap::update()
{
}

void Tilemap::draw()
{
	static Rect brick_rect = { 85, 0, tile_width, tile_height };
	static Rect ground_rect = { 0, 0, tile_width, tile_height };

    for(uint32 i = 0; i < tiles.Size(); ++i)
    {
        draw_call.sd.world_position = tiles[i].position;
        draw_call.sd.world_size = tiles[i].size;
        ren->push_draw_call(draw_call, DrawLayer_Tilemap);
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
	draw_call.shader = Shader_Default;
	draw_call.options = DrawOptions::TEXTURE_RECT;
	draw_call.sd.tex_rect = get_sprite_rect(SpriteRect::STONE);

	draw_call.sd.world_size = vec2(1.f, 1.f);
	draw_call.sd.world_position = { 0, 0 };
	draw_call.sd.draw_angle = 0;
}

#endif
