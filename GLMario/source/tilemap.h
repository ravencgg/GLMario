#pragma once

#include "types.h"
#include "renderer.h"

#define TILE_GROUP_SIDE_SIZE 4
#define TILE_GROUP_SIZE (TILE_GROUP_SIDE_SIZE * TILE_GROUP_SIDE_SIZE)

enum TileCollision : uint8
{
    TileCollision_None,
    TileCollision_Solid,

    TileCollision_LeftTriangle,
    TileCollision_RightTriangle,

    TileCollision_HalfRect,
    TileCollision_HalfLeftTriangle,
    TileCollision_HalfRightTriangle,
};

struct TileTypeInfo
{
    uint16 tex_coord_x;
    uint16 tex_coord_y;
    uint8 collision_type;

    // 3 bytes free?
};

struct Tile2
{
    TileTypeInfo tile_info;

    uint16 tile_coord_x;
    uint16 tile_coord_y;
};

struct TileGroup
{
    uint16 group_coord_x;
    uint16 group_coord_y;
    Tile2 tiles[TILE_GROUP_SIZE];
};

struct TileMap
{
    TileGroup* tile_groups;

    uint32 hor_groups;
    uint32 ver_groups;
    uint32 map_width;
    uint32 map_height;

// Corrects for non multiple of TILE_GROUP_SIDE_SIZE map sizes
    uint32 map_adjusted_width;
    uint32 map_adjusted_height;
};

struct MemoryArena;

void AllocateTileMap(MemoryArena* arena, TileMap* tilemap, uint32 map_width, uint32 map_height);

void MakeCheckerboard(TileMap*, Rect);
void MakeWalledRoom(TileMap*, Rect);
void AddTile(TileMap*, float, float);
Tile2* FindTile(TileMap* tilemap, uint32 x, uint32 y);

void DrawTileMap(TileMap*);

#if 1

#include "physics.h"

enum TileType : uint32 { EMPTY, BRICK, COUNT };

struct Tile
{
    Vec2 position;
    Vec2 size;
	TileType tile_type;
    RArrayRef<StaticCollider> collider;

    Tile()
    : collider() {}
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
	void draw(GameState*);

    Array<Tile> tiles;

private:
	void init();

	int32 tile_width, tile_height;
    DrawCall draw_call;

    Physics* physics;
};
#endif
