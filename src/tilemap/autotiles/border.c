
#include "game/tilemap/tilemap_internal.h"

void DrawBorderTile(const Tilemap *tilemap, const Tileset *tileset, const Rectangle src, const Vector2 pos, const u32 x, const u32 y)
{
    const Layer *layer = &tilemap->layers[0];
    const Texture2D texture = tileset->texture;

    const u32 mapWidth = layer->width;
    const u32 mapHeight = layer->height;

    const float tileSize = (float)tilemap->header.tileWidth;
    const float tileHalf = tileSize / 2.0f;

    const bool N  = HAS_TILE(layer, x, y - 1, mapWidth, mapHeight) && GetTileType(tilemap, 0, x, y - 1) != TILE_BORDER;
    const bool S  = HAS_TILE(layer, x, y + 1, mapWidth, mapHeight) && GetTileType(tilemap, 0, x, y + 1) != TILE_BORDER;
    const bool W  = HAS_TILE(layer, x - 1, y, mapWidth, mapHeight) && GetTileType(tilemap, 0, x - 1, y) != TILE_BORDER;
    const bool E  = HAS_TILE(layer, x + 1, y, mapWidth, mapHeight) && GetTileType(tilemap, 0, x + 1, y) != TILE_BORDER;

    const bool NW = HAS_TILE(layer, x - 1, y - 1, mapWidth, mapHeight) && GetTileType(tilemap, 0, x - 1, y - 1) != TILE_BORDER;
    const bool NE = HAS_TILE(layer, x + 1, y - 1, mapWidth, mapHeight) && GetTileType(tilemap, 0, x + 1, y - 1) != TILE_BORDER;
    const bool SW = HAS_TILE(layer, x - 1, y + 1, mapWidth, mapHeight) && GetTileType(tilemap, 0, x - 1, y + 1) != TILE_BORDER;
    const bool SE = HAS_TILE(layer, x + 1, y + 1, mapWidth, mapHeight) && GetTileType(tilemap, 0, x + 1, y + 1) != TILE_BORDER;

    // EDGES
    if (N) DrawTextureRec(texture, (Rectangle){ src.x + tileHalf, src.y + tileSize, tileSize, tileHalf }, pos, WHITE);
    if (S) DrawTextureRec(texture, (Rectangle){ src.x + tileHalf, src.y + tileSize * 2.0f + tileHalf, tileSize, tileHalf }, (Vector2){ pos.x, pos.y + tileHalf }, WHITE);
    if (W) DrawTextureRec(texture, (Rectangle){ src.x, src.y + tileSize + tileHalf, tileHalf, tileSize }, pos, WHITE);
    if (E) DrawTextureRec(texture, (Rectangle){ src.x + tileSize + tileHalf, src.y + tileSize + tileHalf, tileHalf, tileSize }, (Vector2){ pos.x + tileHalf, pos.y }, WHITE);

    // CORNER IN
    if (N && W) DrawTextureRec(texture, (Rectangle){ src.x, src.y, tileHalf, tileHalf }, pos, WHITE);
    if (N && E) DrawTextureRec(texture, (Rectangle){ src.x + tileHalf, src.y, tileHalf, tileHalf }, (Vector2){ pos.x + tileHalf, pos.y }, WHITE);
    if (S && W) DrawTextureRec(texture, (Rectangle){ src.x, src.y + tileHalf, tileHalf, tileHalf }, (Vector2){ pos.x, pos.y + tileHalf }, WHITE);
    if (S && E) DrawTextureRec(texture, (Rectangle){ src.x + tileHalf, src.y + tileHalf, tileHalf, tileHalf }, (Vector2){ pos.x + tileHalf, pos.y + tileHalf }, WHITE);

    // CORNER OUT
    if (!N && !W && NW) DrawTextureRec(texture, (Rectangle){ src.x + tileSize, src.y, tileHalf, tileHalf }, pos, WHITE);
    if (!N && !E && NE) DrawTextureRec(texture, (Rectangle){ src.x + tileSize + tileHalf, src.y, tileHalf, tileHalf }, (Vector2){ pos.x + tileHalf, pos.y }, WHITE);
    if (!S && !W && SW) DrawTextureRec(texture, (Rectangle){ src.x + tileSize, src.y + tileHalf, tileHalf, tileHalf }, (Vector2){ pos.x, pos.y + tileHalf }, WHITE);
    if (!S && !E && SE) DrawTextureRec(texture, (Rectangle){ src.x + tileSize + tileHalf, src.y + tileHalf, tileHalf, tileHalf }, (Vector2){ pos.x + tileHalf, pos.y + tileHalf }, WHITE);
}
