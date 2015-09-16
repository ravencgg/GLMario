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
    Vec2 position;
    Vec2 size;
	TileType tile_type;
    RStaticCollider collider;
};

Tile MakeTile(Physics* physics, Vec2 position, Vec2 size, TileType tile_type = BRICK);

class Tilemap
{
public:
	Tilemap(Physics*);
	~Tilemap();

	void MakeCheckerboard(Rect);
	void MakeWalledRoom(Rect);
    void AddTile(float, float);
	void update();
	void draw();

private:
	void init();

    std::vector<Tile> tiles;
	int32 tile_width, tile_height;
    DrawCall draw_call;

	Renderer* ren;
    Physics* physics;
};
