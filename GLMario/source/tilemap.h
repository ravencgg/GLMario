#pragma once

#include "types.h"
#include "mathops.h"
#include "game_object.h"
#include "renderer.h"

class Tilemap
{
public:
	Tilemap();
	Tilemap(uint32, uint32);
	~Tilemap();

	enum TileType { EMPTY, BRICK, COUNT };

	struct Tile
	{
		TileType tile_type;
		bool collideable;
	};

	void fill_checkerboard();

	void draw();

private:
	void init();
	uint32 array_loc(uint32, uint32);

	Tile* tiles = nullptr;
	uint32 width, height;
	uint32 tile_width, tile_height;

	Renderer* ren;
};