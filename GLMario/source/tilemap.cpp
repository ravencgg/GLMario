#include "tilemap.h"
#include "game_types.h"
#include "input.h"

#include "entity.h"

/**********************************************
 *
 * Tilemap Collision
 *
 ***************/

enum CollisionInfo
{
    TileCollision_NONE,
    TileCollision_Left   = 0x1,
    TileCollision_Top    = 0x2,
    TileCollision_Right  = 0x4,
    TileCollision_Bottom = 0x8,
};




/**********************************************
 *
 * Tilemap Quadtree
 *
 ***************/


static bool Contains(TileGroup* tile_group, Rectf rect)
{
    bool result = Intersects(tile_group->aabb, rect);
    return result;
}

static bool Contains(TileGroup* tile_group, Vec2 point)
{
    bool result = Contains(tile_group->aabb, point);
    return result;
}

static bool IsLeaf(TileGroup* tile_group)
{
    bool result = !tile_group->is_parent;
    return result;
}

// NOTE: could switch to doing this by ID instead of pointer?
// @untested! We currently don't allow the removal of static colliders so this hasn't been used
#if 0
uint32 RemoveTile(TileGroup* node, Tile* to_delete)
{
    uint32 num_removed = 0;

    if(IsLeaf(node))
    {
        for(uint8 i = 0; i < node->contained_colliders; ++i)
        {
            if(node->colliders[i] == to_delete)
            {
                num_removed++;
                auto remaining = (MAX_LEAF_SIZE - 1) - node->contained_colliders;
                if(remaining > 0)
                {
                    memmove(&node->colliders[i], &node->colliders[i + 1], sizeof((uint32)node->colliders[i] * remaining));
                    node->colliders[i + 1] = 0;
                }
                else
                {
                    node->colliders[i] = 0; // This is the last one in the array
                }
            }
        }
    }
    else
    {
        TileGroup* child_node = node->child_nodes;
        for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i)
        {
            num_removed += RemoveTile(child_node, to_delete);
        }
    }

    return num_removed;
}
#endif

// Returns true if it was added to at least one quadtree, false if you are trying to add out of bounds
static bool AddTile(TileGroup* tile_group, MemoryArena* arena, Tile* tile)
{
    if(!Contains(tile_group, tile->aabb))
    {
        // didn't add here, return false;
        return false;
    }

    if(IsLeaf(tile_group))
    {
        if(tile_group->contained_colliders < MAX_LEAF_SIZE)
        {
            tile_group->colliders[tile_group->contained_colliders] = tile;
            ++tile_group->contained_colliders;
        }
        else
        {
            // Split the node into 4
            Tile* temp_colliders[MAX_LEAF_SIZE];
            memcpy(temp_colliders, tile_group->colliders, sizeof(temp_colliders));

            tile_group->child_nodes = PushArray(arena, TileGroup, QUADTREE_CHILDREN);
            tile_group->is_parent = true;

            uint16 new_depth = tile_group->depth + 1;
            TileGroup* new_node = tile_group->child_nodes;

            Rectf r = tile_group->aabb;
            float half_width = r.w / 2.f;
            float half_height = r.h / 2.f;

            Rectf divided_rects[QUADTREE_CHILDREN] = {
                { r.x              , r.y              , half_width, half_height },
                { r.x + half_height, r.y              , half_width, half_height },
                { r.x              , r.y + half_height, half_width, half_height },
                { r.x + half_height, r.y + half_height, half_width, half_height },
            };

            for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++new_node)
            {
                new_node->aabb  = divided_rects[i];
                new_node->depth = new_depth;
                new_node->is_parent = false;
            }

            // Add the original tile that was passed in.
            AddTile(tile_group, arena, tile);

            // re-add the old pointers to the new child nodes.
            for(uint32 i = 0; i < MAX_LEAF_SIZE; ++i)
            {
                AddTile(tile_group, arena, temp_colliders[i]);
            }
        }
    }
    else
    { // Not a leaf, recurse
        TileGroup* child = tile_group->child_nodes;
        for(uint32 j = 0; j < QUADTREE_CHILDREN; ++j, ++child)
        {
            if(Contains(child, tile->aabb))
            {
                AddTile(child, arena, tile);
            }
        }
    }

    // Either added here or in a child
    return true;
}

static Tile** GetPotentialCollidingTiles(TileGroup* node, Rectf aabb, Tile** collision_list, uint32* list_size)
{
    Tile** index = collision_list;

    if(Contains(node, aabb))
    {
        if(IsLeaf(node))
        {
            uint8 col_count = node->contained_colliders;
            if(col_count > 0)
            {
                size_t copy_size = col_count * sizeof(node->colliders[0]);
                memcpy(index, node->colliders, copy_size);

                index += col_count;
                *list_size += col_count;
            }
        }
        else
        {
            TileGroup* child = node->child_nodes;
            for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++child)
            {
                index = GetPotentialCollidingTiles(child, aabb, index, list_size);
            }
        }
    }
    return index;
}

static void DrawBoundingBoxes(TileGroup* tile_group, Renderer* ren)
{
    const float blue  = (float)tile_group->depth / 10.f;
    const float green = Contains(tile_group, MouseWorldPosition()) ? 0.8f : 0.1f;

    Vec4 color = { 0, green, blue, 0.5f };

    if(!IsLeaf(tile_group))
    {
        TileGroup* node = tile_group->child_nodes;
        for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++node)
        {
            DrawBoundingBoxes(node, ren);
        }
    }

    PrimitiveDrawParams params = {};
    params.line_width = 2;
    DrawRect(ren, tile_group->aabb, color, 0, params);
}


/**********************************************
 *
 * Tilemap
 *
 ***************/

void AllocateTileMap(MemoryArena* arena, TileMap* tilemap, Vec2 map_size, uint32 max_tiles)
{
    tilemap->size = map_size;

// TODO: accurate allocation of sub arena

    tilemap->tiles = PushArray(arena, Tile, max_tiles);
    tilemap->max_tiles = max_tiles;
    tilemap->num_tiles = 0;

    tilemap->quadtree_memory = CreateSubArena(arena, sizeof(TileGroup) * max_tiles);
    tilemap->tile_quadtree = PushStruct(&tilemap->quadtree_memory, TileGroup);
    tilemap->tile_quadtree->aabb = { 0, 0, map_size.x, map_size.y };
}

void AddTileToMap(TileMap* tilemap, Rectf tile_rect)
{
    Tile* tile = tilemap->tiles + tilemap->num_tiles;
    tile->aabb = tile_rect;
    ++tilemap->num_tiles;

    if(!AddTile(tilemap->tile_quadtree, &tilemap->quadtree_memory, tile))
    {
        // Didn't add the tile to anything, reuse this for a future allocation
        --tilemap->num_tiles;
        memset(tile, 0, sizeof(Tile));
    }
}

void AddTileToMap(TileMap* tilemap, Vec2 position)
{
    AddTileToMap(tilemap, RectFromDimCenter(position, {1.f, 1.f}));
}

void DrawTileMap(GameState* game_state, TileMap* tilemap)
{
    for(uint32 index = 0;
        index < tilemap->num_tiles;
        ++index)
    {
        Tile* tile = tilemap->tiles + index;
        float green = Contains(tile->aabb, MouseWorldPosition()) ? 0.8f : 0.1f;
        const Vec4 tile_color = { 1.f, green, 0, 0.5f };

        DrawRect(game_state->renderer, tile->aabb, tile_color);
    }

    Vec2 mouse_pos = MouseWorldPosition();
    Rectf mouse_rect = {mouse_pos.x, mouse_pos.y, 0.01f, 0.01f};
    Tile** active_tiles;
    PushArrayScoped(active_tiles, &game_state->temporary_memory, Tile*, tilemap->num_tiles);
    uint32 num_active_tiles = 0;
    GetPotentialCollidingTiles(tilemap->tile_quadtree, mouse_rect, active_tiles, &num_active_tiles);

    for(uint32 index = 0;
        index < num_active_tiles;
        ++index)
    {
        Tile* tile = active_tiles[index];
        const Vec4 tile_color = { 1.f, 1.0f, 0, 0.5f };
        DrawRect(game_state->renderer, tile->aabb, tile_color);
    }

    DrawBoundingBoxes(tilemap->tile_quadtree, game_state->renderer);
}

Tile** GetPotentialCollidingTiles(MemoryArena* arena, TileMap* tilemap, Rectf rect_plus_velocity, uint32* num_tiles_found)
{
    Tile** result = PushArray(arena, Tile*, tilemap->num_tiles);
//    PushArrayScoped(active_tiles, &game_state->temporary_memory, Tile*, tilemap->num_tiles);

    uint32 num_active_tiles = 0;
    Tile** active_tiles = result;
    GetPotentialCollidingTiles(tilemap->tile_quadtree, rect_plus_velocity, active_tiles, &num_active_tiles);

    if (num_tiles_found)
    {
        *num_tiles_found = num_active_tiles;
    }

    return result;
}
