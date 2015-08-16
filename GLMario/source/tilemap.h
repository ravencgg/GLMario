#pragma once

#include "types.h"
#include "mathops.h"
#include "entity.h"
#include "renderer.h"
#include "console.h"
#include "physics.h"

#include <vector>

enum TileType { EMPTY, BRICK, COUNT };

struct Tile 
{
public:
	TileType tile_type;
    TStaticCollider collider;
};

struct TileBlock
{
	int32 width;
	int32 height;
	std::vector<PhysicsRect> tile_info;
};

class Tilemap
{
public:
	Tilemap();
	Tilemap(int32, int32);
	~Tilemap();

	void fill_checkerboard();
	void fill_walled_room();

	void try_move(Rectf, Vec2);

	void update();
	void draw();

	TileBlock get_tile_block(Point2, Point2);

private:
	void init();
	uint32 array_loc(uint32, uint32);

	Tile* tiles = nullptr;
	int32 width, height;
	int32 tile_width, tile_height;
    DrawCall draw_call;

	Renderer* ren;
};
