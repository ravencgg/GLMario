#pragma once

#include "types.h"
#include "mathops.h"
#include "game_object.h"
#include "renderer.h"
#include "console.h"

class Tilemap
{
public:
	Tilemap();
	Tilemap(int32, int32);
	~Tilemap();

	DrawCall draw_call;

	enum TileType { EMPTY, BRICK, COUNT };

	struct Tile
	{
		TileType tile_type;
		bool collideable;
	};

	void fill_checkerboard();
	void fill_walled_room();

	void update();
	void draw();

private:
	void init();
	uint32 array_loc(uint32, uint32);

	Tile* tiles = nullptr;
	int32 width, height;
	int32 tile_width, tile_height;

	Renderer* ren;
};
