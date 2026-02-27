#include "game/tilemap/tilemap_internal.h"

void DrawWallTile(const Tilemap *tilemap, const Tileset *tileset,
             const Rectangle src, const Vector2 pos,
             const u32 x, const u32 y)
{
    const TileType typeN = GetTileType(tilemap, 0, x, y - 1);
    const TileType typeS = GetTileType(tilemap, 0, x, y + 1);
    const TileType typeE = GetTileType(tilemap, 0, x + 1, y);
    const TileType typeW = GetTileType(tilemap, 0, x - 1, y);

    const float sX = src.x;
    const float sY = src.y;
    const float pX = pos.x;
    const float pY = pos.y;

    const float tileSize = (float)tilemap->header.tileWidth;
    const float tileHalf = tileSize * 0.5f;

    // Center fill
    if (typeE == TILE_WALL && typeS == TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX + tileHalf, sY + tileHalf, tileSize, tileSize },
            (Vector2){ pX + tileHalf, pY + tileHalf }, WHITE);
    }

    // Edges
    if (typeN != TILE_WALL && typeE == TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX + tileHalf, sY, tileSize, tileHalf },
            (Vector2){ pX + tileHalf, pY }, WHITE);
    }

    if (typeS != TILE_WALL && typeE == TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX + tileHalf, sY + 48, tileSize, tileHalf },
            (Vector2){ pX + tileHalf, pY + tileHalf }, WHITE);
    }

    if (typeW != TILE_WALL && typeS == TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX, sY + tileHalf, tileHalf, tileSize },
            (Vector2){ pX, pY + tileHalf }, WHITE);
    }

    if (typeE != TILE_WALL && typeS == TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX + 48, sY + tileHalf, tileHalf, tileSize },
            (Vector2){ pX + tileHalf, pY + tileHalf }, WHITE);
    }

    // Corners
    if (typeW != TILE_WALL && typeN != TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX, sY, tileHalf, tileHalf },
            (Vector2){ pX, pY }, WHITE);
    }

    if (typeE != TILE_WALL && typeN != TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX + 48, sY, tileHalf, tileHalf },
            (Vector2){ pX + tileHalf, pY }, WHITE);
    }

    if (typeW != TILE_WALL && typeS != TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX, sY + 48, tileHalf, tileHalf },
            (Vector2){ pX, pY + tileHalf }, WHITE);
    }

    if (typeE != TILE_WALL && typeS != TILE_WALL) {
        DrawTextureRec(tileset->texture,
            (Rectangle){ sX + 48, sY + 48, tileHalf, tileHalf },
            (Vector2){ pX + tileHalf, pY + tileHalf }, WHITE);
    }
}
