#pragma once

#include "types.h"
#include "mathops.h"
#include "entity.h"
#include "renderer.h"
#include "console.h"
#include "physics.h"
#include "containers.h"

#include <vector>

enum TileType : uint32 { EMPTY, BRICK, COUNT };

struct Tile
{
    Vec2 position;
    Vec2 size;
	TileType tile_type;
    RArrayRef<StaticCollider> collider;
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

    Array<Tile> tiles;

private:
	void init();

	int32 tile_width, tile_height;
    DrawCall draw_call;

	Renderer* ren;
    Physics* physics;
};
