#pragma once

#include "types.h"
#include "renderer.h"

#define TILE_GROUP_SIDE_SIZE 4
#define TILE_GROUP_SIZE (TILE_GROUP_SIDE_SIZE * TILE_GROUP_SIDE_SIZE)

struct Tile
{
    Rectf aabb;
};

#define QUADTREE_CHILDREN 4
#define MAX_LEAF_SIZE 8

struct TileGroup
{
    uint16 depth;
    uint8 contained_colliders;
    bool is_parent;
    Rectf aabb;

    union
    {
        // TODO: fix straddling tiles. Allocate tiles at the end of the arena and use pointers to them again?
        // TODO: extra memory for max-depth groups
        Tile* colliders[MAX_LEAF_SIZE]; // Colliders are NOT contiguous in memory
        TileGroup* child_nodes;         // Children are contiguous in memory
    };
};

struct TileMap
{
    Vec2 size;
    Tile* tiles;
    uint32 num_tiles;
    uint32 max_tiles;

    TileGroup* tile_quadtree;
    MemoryArena quadtree_memory;
};

struct MemoryArena;
void AllocateTileMap(MemoryArena* arena, TileMap* tilemap, Vec2 map_size, uint32 max_tiles);

void AddTileToMap(TileMap* tilemap, Rectf tile_rect);
void AddTileToMap(TileMap* tilemap, Vec2 position);
void DrawTileMap(GameState*, TileMap*);

#if 0 // TODO: Renames
StaticCollider** GetPotentialColliders(PhysicsNode* node, Rectf aabb, StaticCollider** collision_list, uint32* list_size);
void DrawBoundingBoxes(PhysicsNode*, Renderer*);
bool Contains(PhysicsNode*, Rectf rect);
bool Contains(PhysicsNode*, Vec2 point);
bool IsLeaf(PhysicsNode*);
#endif

