#include "game/tilemap/tilemap_internal.h"

void DrawCarpetTile(const Tilemap *tilemap, const Tileset *tileset,
                    const Rectangle src, const Vector2 pos,
                    const u32 x, const u32 y)
{
    const TileType typeN = GetTileType(tilemap, 1, x, y - 1);
    const TileType typeS = GetTileType(tilemap, 1, x, y + 1);
    const TileType typeE = GetTileType(tilemap, 1, x + 1, y);
    const TileType typeW = GetTileType(tilemap, 1, x - 1, y);

    const float sX = src.x;
    const float sY = src.y;
    const float pX = pos.x;
    const float pY = pos.y;

    const float tileSize = (float)tilemap->header.tileWidth;
    const float tileHalf = tileSize * 0.5f;
    const Texture2D tex = tileset->texture;

    // --- 1. Center Fill / Base ---
    if (typeE == TILE_CARPET) {
        DrawTextureRec(tex, (Rectangle){ sX + tileHalf, sY + tileSize + tileHalf, tileSize, tileSize }, (Vector2){ pX, pY }, WHITE);
    }

    // --- 2. Edges ---
    // Right Edge
    if (typeE != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX + tileSize, sY + tileSize + tileHalf, tileSize, tileSize }, (Vector2){ pX, pY }, WHITE);

    // Left Edge
    if (typeW != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX, sY + tileSize + tileHalf, tileSize, tileSize }, (Vector2){ pX, pY }, WHITE);

    // Top Edge
    if (typeN != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX + tileHalf, sY + tileSize, tileSize, tileHalf }, (Vector2){ pX, pY }, WHITE);

    // Bottom Edge
    if (typeS != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX + tileHalf, sY + tileSize * 2 + tileHalf, tileSize, tileHalf }, (Vector2){ pX + 0, pY + tileHalf }, WHITE);

    // --- 3. Corners ---
    // Top-Right Corner
    if (typeN != TILE_CARPET && typeE != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX + tileSize + tileHalf, sY + tileSize, tileHalf, tileHalf }, (Vector2){ pX + tileHalf, pY }, WHITE);

    // Top-Left Corner
    if (typeN != TILE_CARPET && typeW != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX, sY + tileSize, tileHalf, tileHalf }, (Vector2){ pX, pY }, WHITE);

    // Bottom-Right Corner
    if (typeS != TILE_CARPET && typeE != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX + tileSize + tileHalf, sY + 2 * tileSize + tileHalf, tileHalf, tileHalf }, (Vector2){ pX + tileHalf, pY + tileHalf }, WHITE);

    // Bottom-Left Corner
    if (typeS != TILE_CARPET && typeW != TILE_CARPET)
        DrawTextureRec(tex, (Rectangle){ sX, sY + 2 * tileSize + tileHalf, tileHalf, tileHalf }, (Vector2){ pX, pY + tileHalf }, WHITE);
}