#include "game/tilemap/tilemap_internal.h"

void DrawTableTile(const Tilemap *tilemap, const Tileset *tileset,
                   const Rectangle src, const Vector2 pos,
                   const u32 x, const u32 y)
{
    // TODO: Finish Auto Tile Table for vertical!
    // const TileType typeN = GetTileType(tilemap, 0, x, y - 1);
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

    // Center fill
    if (typeE == TILE_TABLE)
    {
        DrawTextureRec(tex,
            (Rectangle){ sX + tileHalf, sY + tileSize * 2.0f - 8.0f, tileSize, tileSize },
            (Vector2){ pX + tileHalf, pY }, WHITE);
    }

    // Edges
    if (typeE != TILE_TABLE)
    {
        DrawTextureRec(tex,
            (Rectangle){ sX + tileSize + tileHalf, sY + tileSize * 2.0f - 8.0f, tileHalf, tileSize },
            (Vector2){ pX + tileHalf, pY }, WHITE);
    }

    if (typeW != TILE_TABLE)
    {
        DrawTextureRec(tex,
            (Rectangle){ sX, sY + tileSize * 2.0f - 8.0f, tileHalf, tileSize },
            (Vector2){ pX, pY }, WHITE);
    }

    // if (typeN != TILE_TABLE)
    //     DrawTextureRec(tex, (Rectangle){ sX + tileHalf, sY + tileSize, tileSize, tileHalf }, (Vector2){ pX, pY - tileHalf }, WHITE);

    if (typeS != TILE_TABLE)
    {
        DrawTextureRec(tex,
            (Rectangle){ sX + tileHalf, sY + tileSize * 3.0f - 8.0f, tileSize, 8.0f },
            (Vector2){ pX, pY + tileSize }, WHITE);
    }

    // Corners
    // if (typeN != TILE_TABLE && typeE != TILE_TABLE)
    //     DrawTextureRec(tex, (Rectangle){ sX + tileSize + tileHalf, sY + tileSize, tileHalf, tileHalf }, (Vector2){ pX + tileHalf, pY }, WHITE);

    // if (typeN != TILE_TABLE && typeW != TILE_TABLE)
    //     DrawTextureRec(tex, (Rectangle){ sX, sY + tileSize, tileHalf, tileHalf }, (Vector2){ pX, pY }, WHITE);

    if (typeS != TILE_TABLE && typeE != TILE_TABLE)
    {
        DrawTextureRec(tex,
            (Rectangle){ sX + tileSize * 2.0f - 8.0f, sY + tileSize * 3.0f - 8.0f, 8.0f, 8.0f },
            (Vector2){ pX + tileSize - 8.0f, pY + tileSize }, WHITE);
    }

    if (typeS != TILE_TABLE && typeW != TILE_TABLE)
    {
        DrawTextureRec(tex,
            (Rectangle){ sX, sY + tileSize * 3.0f - 8.0f, 8.0f, 8.0f },
            (Vector2){ pX, pY + tileSize }, WHITE);
    }
}