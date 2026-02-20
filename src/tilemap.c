#include <stdlib.h>
#include <stdio.h>

#include "tilemap.h"
#include "game.h"


#define TILESET_PATH "assets/tilesets/"
#define TILE_HALF 16
#define TILE_SIZE 32


static void
read_exact(FILE *f, void *dst, const size_t n, const char *label)
{
    if (fread(dst, 1, n, f) != n) {
        TraceLog(LOG_ERROR, "[Tilemap] Failed to read file: %s", label);
        exit(1);
    }
}

static char *read_string(FILE *f)
{
    int len = 0;
    read_exact(f, &len, sizeof(int), "string length");

    char *buf = malloc(len + 1);
    if (!buf) { TraceLog(LOG_ERROR, "[Tilemap] Out of memory"); exit(1); }

    read_exact(f, buf, len, "string data");
    buf[len] = '\0';
    return buf;
}

static int
FindTilesetIndex(const Tilemap *tilemap, const unsigned int gid)
{
    for (int i = tilemap->tilesetCount - 1; i >= 0; i--) {
        if (gid >= (unsigned int)tilemap->tilesets[i].firstGid) return i;
    }

    return -1;
}

static bool
IsTileValid(const int x, const int y, const int width, const int height)
{
    return x >= 0 && x < width && y >= 0 && y < height;
}

static bool
HasTile(const TileLayer *layer, const int x, const int y, const int width, const int height)
{
    return IsTileValid(x, y, width, height) && layer->data[y * width + x] != 0;
}


Tilemap LoadTilemapBinary(const char *jsonPath)
{
    Tilemap tilemap = {0};

    FILE *f = fopen(jsonPath, "rb");
    if (!f) {
        TraceLog(LOG_ERROR, "[Tilemap] Failed to open file: %s", jsonPath);
        exit(1);
    }

    // ── HEADER ───────────────────────────────────────────────────────────────
    read_exact(f, &tilemap.width,       sizeof(int), "width");
    read_exact(f, &tilemap.height,      sizeof(int), "height");
    read_exact(f, &tilemap.tileWidth,   sizeof(int), "tileWidth");
    read_exact(f, &tilemap.tileHeight,  sizeof(int), "tileHeight");
    read_exact(f, &tilemap.tilesetCount,sizeof(int), "tilesetCount");
    read_exact(f, &tilemap.layerCount,  sizeof(int), "layerCount");
    read_exact(f, &tilemap.mapId,      sizeof(int), "mapId");

    // ── MAP PROPERTIES ───────────────────────────────────────────────────────
    read_exact(f, &tilemap.spawnPointX, sizeof(int), "spawnPointX");
    read_exact(f, &tilemap.spawnPointY, sizeof(int), "spawnPointY");

    tilemap.tilesets = (Tileset *)calloc(tilemap.tilesetCount, sizeof(Tileset));
    if (!tilemap.tilesets) { TraceLog(LOG_ERROR, "[Tilemap] Out of memory for tilesets"); exit(1); }

    char pathBuffer[SHORT_STRING] = {0};

    for (int i = 0; i < tilemap.tilesetCount; i++)
    {
        Tileset *ts = &tilemap.tilesets[i];

        read_exact(f, &ts->firstGid,      sizeof(int), "firstGid");
        read_exact(f, &ts->propertyCount, sizeof(int), "propertyCount");
        ts->texturePath = read_string(f);

        ts->properties = (TileProperties *)malloc(ts->propertyCount * sizeof(TileProperties));
        if (!ts->properties) { TraceLog(LOG_ERROR, "[Tilemap] Out of memory for tile properties"); exit(1); }

        for (int j = 0; j < ts->propertyCount; j++) {
            read_exact(f, &ts->properties[j].id,   sizeof(int), "prop.id");
            read_exact(f, &ts->properties[j].type, sizeof(int), "prop.type");
        }

        snprintf(pathBuffer, SHORT_STRING, "%s%s", TILESET_PATH, ts->texturePath);
        ts->texture = LoadTextureFromBin(pathBuffer);
    }

    // ── LAYERS ───────────────────────────────────────────────────────────────
    tilemap.layers = (TileLayer *)calloc(tilemap.layerCount, sizeof(TileLayer));
    if (!tilemap.layers) { TraceLog(LOG_ERROR, "[Tilemap] Out of memory for layers"); exit(1); }

    for (int i = 0; i < tilemap.layerCount; i++)
    {
        TileLayer *layer = &tilemap.layers[i];

        read_exact(f, &layer->width,  sizeof(int), "layer.width");
        read_exact(f, &layer->height, sizeof(int), "layer.height");

        const int cell_count = layer->width * layer->height;
        layer->data = (unsigned int *)malloc(cell_count * sizeof(unsigned int));
        if (!layer->data) { TraceLog(LOG_ERROR, "[Tilemap] Out of memory for layer data"); exit(1); }

        read_exact(f, layer->data, cell_count * sizeof(unsigned int), "layer.data");
    }

    fclose(f);
    return tilemap;
}

Tilemap LoadTilemapById(int mapId)
{
    char path[SHORT_STRING];
    snprintf(path, SHORT_STRING, "assets/tilemaps/map_%d.bin", mapId);
    return LoadTilemapBinary(path);
}

void UnloadTilemap(Tilemap *tilemap)
{
    if (!tilemap) return;

    // Unload tilesets
    for (int i = 0; i < tilemap->tilesetCount; i++) {
        UnloadTexture(tilemap->tilesets[i].texture);
        free(tilemap->tilesets[i].texturePath);
        free(tilemap->tilesets[i].properties);
    }
    free(tilemap->tilesets);

    // Unload layers
    for (int i = 0; i < tilemap->layerCount; i++) {
        free(tilemap->layers[i].data);
    }
    free(tilemap->layers);

    // Reset tilemap
    *tilemap = (Tilemap){0};
}

static void
DrawTilePart(const Texture2D *texture, const float srcX, const float srcY,
                                       const float srcW, const float srcH,
                                       const float dstX, const float dstY)
{
    DrawTextureRec(
        *texture,
        (Rectangle){ srcX, srcY, srcW, srcH },
        (Vector2){ dstX, dstY },
        WHITE
    );
}

static void
DrawWallTile(const Tilemap *tilemap, const Tileset *tileset,
             const Rectangle src, const Vector2 pos,
             const int x, const int y)
{
    const TileType typeN = GetTileType(tilemap, 0, x, y - 1);
    const TileType typeS = GetTileType(tilemap, 0, x, y + 1);
    const TileType typeE = GetTileType(tilemap, 0, x + 1, y);
    const TileType typeW = GetTileType(tilemap, 0, x - 1, y);

    const float sX = src.x;
    const float sY = src.y;
    const float pX = pos.x;
    const float pY = pos.y;

    // Center fill
    if (typeE == TILE_WALL && typeS == TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX + TILE_HALF, sY + TILE_HALF, TILE_SIZE, TILE_SIZE,
            pX + TILE_HALF, pY + TILE_HALF);
    }

    // Edges
    if (typeN != TILE_WALL && typeE == TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX + TILE_HALF, sY, TILE_SIZE, TILE_HALF,
            pX + TILE_HALF, pY);
    }

    if (typeS != TILE_WALL && typeE == TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX + TILE_HALF, sY + 48, TILE_SIZE, TILE_HALF,
            pX + TILE_HALF, pY + TILE_HALF);
    }

    if (typeW != TILE_WALL && typeS == TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX, sY + TILE_HALF, TILE_HALF, TILE_SIZE,
            pX, pY + TILE_HALF);
    }

    if (typeE != TILE_WALL && typeS == TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX + 48, sY + TILE_HALF, TILE_HALF, TILE_SIZE,
            pX + TILE_HALF, pY + TILE_HALF);
    }

    // Corners
    if (typeW != TILE_WALL && typeN != TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX, sY, TILE_HALF, TILE_HALF, pX, pY);
    }

    if (typeE != TILE_WALL && typeN != TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX + 48, sY, TILE_HALF, TILE_HALF,
            pX + TILE_HALF, pY);
    }

    if (typeW != TILE_WALL && typeS != TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX, sY + 48, TILE_HALF, TILE_HALF,
            pX, pY + TILE_HALF);
    }

    if (typeE != TILE_WALL && typeS != TILE_WALL) {
        DrawTilePart(&tileset->texture,
            sX + 48, sY + 48, TILE_HALF, TILE_HALF,
            pX + TILE_HALF, pY + TILE_HALF);
    }
}

static void
DrawCarpetTile(const Tilemap *tilemap, const Tileset *tileset,
             const Rectangle src, const Vector2 pos,
             const int x, const int y)
{
    const TileType typeN = GetTileType(tilemap, 1, x, y - 1);
    const TileType typeS = GetTileType(tilemap, 1, x, y + 1);
    const TileType typeE = GetTileType(tilemap, 1, x + 1, y);
    const TileType typeW = GetTileType(tilemap, 1, x - 1, y);

    const float sX = src.x;
    const float sY = src.y;
    const float pX = pos.x;
    const float pY = pos.y;

    // Center fill
    if (typeE == TILE_CARPET)
        DrawTilePart(&tileset->texture, sX + TILE_HALF, sY + TILE_SIZE + TILE_HALF, TILE_SIZE, TILE_SIZE, pX, pY);

    // Edges
    if (typeE != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX + TILE_SIZE, sY + TILE_SIZE + TILE_HALF, TILE_SIZE, TILE_SIZE, pX, pY);

    if (typeW != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX, sY + TILE_SIZE + TILE_HALF, TILE_SIZE, TILE_SIZE, pX, pY);

    if (typeN != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX + TILE_HALF, sY + TILE_SIZE, TILE_SIZE, TILE_HALF, pX, pY);

    if (typeS != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX + TILE_HALF, sY + TILE_SIZE * 2 + TILE_HALF, TILE_SIZE, TILE_HALF, pX, pY + TILE_HALF);

    // Corners
    if (typeN != TILE_CARPET && typeE != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX + TILE_SIZE + TILE_HALF, sY + TILE_SIZE, TILE_HALF, TILE_HALF, pX + TILE_HALF, pY);

    if (typeN != TILE_CARPET && typeW != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX, sY + TILE_SIZE, TILE_HALF, TILE_HALF, pX, pY);

    if (typeS != TILE_CARPET && typeE != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX + TILE_SIZE + TILE_HALF, sY + 2 * TILE_SIZE + TILE_HALF, TILE_HALF, TILE_HALF, pX + TILE_HALF, pY + TILE_HALF);

    if (typeS != TILE_CARPET && typeW != TILE_CARPET)
        DrawTilePart(&tileset->texture, sX, sY + 2 * TILE_SIZE + TILE_HALF, TILE_HALF, TILE_HALF, pX, pY + TILE_HALF);
}

static void
DrawTableTile(const Tilemap *tilemap, const Tileset *tileset,
             const Rectangle src, const Vector2 pos,
             const int x, const int y)
{
    const TileType typeN = GetTileType(tilemap, 1, x, y - 1);
    const TileType typeS = GetTileType(tilemap, 1, x, y + 1);
    const TileType typeE = GetTileType(tilemap, 1, x + 1, y);
    const TileType typeW = GetTileType(tilemap, 1, x - 1, y);

    const float sX = src.x;
    const float sY = src.y;
    const float pX = pos.x;
    const float pY = pos.y;

    // Center fill
    if (typeE == TILE_TABLE)
        DrawTilePart(&tileset->texture, sX + TILE_HALF, sY + TILE_SIZE * 2.0f - 8.0f, TILE_SIZE, TILE_SIZE, pX + TILE_HALF, pY);

    // Edges
    if (typeE != TILE_TABLE)
        DrawTilePart(&tileset->texture, sX + TILE_SIZE + TILE_HALF, sY + TILE_SIZE * 2.0f - 8.0f, TILE_HALF, TILE_SIZE, pX + TILE_HALF, pY);

    if (typeW != TILE_TABLE)
        DrawTilePart(&tileset->texture, sX, sY + TILE_SIZE * 2.0f - 8.0f, TILE_HALF, TILE_SIZE, pX, pY);

    // if (typeN != TILE_TABLE)
    //     DrawTilePart(&tileset->texture, sX + TILE_HALF, sY + TILE_SIZE, TILE_SIZE, TILE_HALF, pX, pY - TILE_HALF);

    if (typeS != TILE_TABLE)
        DrawTilePart(&tileset->texture, sX + TILE_HALF, sY + TILE_SIZE * 3.0f - 8.0f, TILE_SIZE, 8.0f, pX, pY + TILE_SIZE);

    // Corners
    // if (typeN != TILE_TABLE && typeE != TILE_TABLE)
    //     DrawTilePart(&tileset->texture, sX + TILE_SIZE + TILE_HALF, sY + TILE_SIZE, TILE_HALF, TILE_HALF, pX + TILE_HALF, pY);

    // if (typeN != TILE_TABLE && typeW != TILE_TABLE)
    //     DrawTilePart(&tileset->texture, sX, sY + TILE_SIZE, TILE_HALF, TILE_HALF, pX, pY);

    if (typeS != TILE_TABLE && typeE != TILE_TABLE)
        DrawTilePart(&tileset->texture, sX + TILE_SIZE * 2.0f - 8.0f, sY + TILE_SIZE * 3.0f - 8.0f, 8.0f, 8.0f, pX + TILE_SIZE - 8.0f, pY + TILE_SIZE);

    if (typeS != TILE_TABLE && typeW != TILE_TABLE)
        DrawTilePart(&tileset->texture, sX, sY + TILE_SIZE * 3.0f - 8.0f, 8.0f, 8.0f, pX, pY + TILE_SIZE);
}

static void
DrawBorderTile(const Tilemap *tilemap, const Tileset *tileset,
               const Rectangle src, const Vector2 pos,
               const int x, const int y)
{
    const TileLayer *layer = &tilemap->layers[0]; // LAYER_GROUND
    const Texture2D texture = tileset->texture;

    const bool N  = HasTile(layer, x, y - 1, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x, y - 1) != TILE_BORDER;
    const bool S  = HasTile(layer, x, y + 1, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x, y + 1) != TILE_BORDER;
    const bool W  = HasTile(layer, x - 1, y, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x - 1, y) != TILE_BORDER;
    const bool E  = HasTile(layer, x + 1, y, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x + 1, y) != TILE_BORDER;

    const bool NW = HasTile(layer, x - 1, y - 1, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x - 1, y - 1) != TILE_BORDER;
    const bool NE = HasTile(layer, x + 1, y - 1, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x + 1, y - 1) != TILE_BORDER;
    const bool SW = HasTile(layer, x - 1, y + 1, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x - 1, y + 1) != TILE_BORDER;
    const bool SE = HasTile(layer, x + 1, y + 1, tilemap->width, tilemap->height) && GetTileType(tilemap, 0, x + 1, y + 1) != TILE_BORDER;

    // DEBUG
    // DrawRectangleLines((int)pos.x, (int)pos.y, 32, 32, RED);

    // EDGES
    if (N) DrawTextureRec(texture, (Rectangle){ src.x + TILE_HALF, src.y + TILE_SIZE, TILE_SIZE, TILE_HALF }, pos, WHITE);
    if (S) DrawTextureRec(texture, (Rectangle){ src.x + TILE_HALF, src.y + TILE_SIZE * 2.0f + TILE_HALF, TILE_SIZE, TILE_HALF }, (Vector2){ pos.x, pos.y + TILE_HALF }, WHITE);
    if (W) DrawTextureRec(texture, (Rectangle){ src.x, src.y + TILE_SIZE + TILE_HALF, TILE_HALF, TILE_SIZE }, pos, WHITE);
    if (E) DrawTextureRec(texture, (Rectangle){ src.x + TILE_SIZE + TILE_HALF, src.y + TILE_SIZE + TILE_HALF, TILE_HALF, TILE_SIZE }, (Vector2){ pos.x + TILE_HALF, pos.y }, WHITE);

    // CORNER IN
    if (N && W) DrawTextureRec(texture, (Rectangle){ src.x, src.y, TILE_HALF, TILE_HALF }, pos, WHITE);
    if (N && E) DrawTextureRec(texture, (Rectangle){ src.x + TILE_HALF, src.y, TILE_HALF, TILE_HALF }, (Vector2){ pos.x + TILE_HALF, pos.y }, WHITE);
    if (S && W) DrawTextureRec(texture, (Rectangle){ src.x, src.y + TILE_HALF, TILE_HALF, TILE_HALF }, (Vector2){ pos.x, pos.y + TILE_HALF }, WHITE);
    if (S && E) DrawTextureRec(texture, (Rectangle){ src.x + TILE_HALF, src.y + TILE_HALF, TILE_HALF, TILE_HALF }, (Vector2){ pos.x + TILE_HALF, pos.y + TILE_HALF }, WHITE);

    // CORNER OUT
    if (!N && !W && NW) DrawTextureRec(texture, (Rectangle){ src.x + TILE_SIZE, src.y, TILE_HALF, TILE_HALF }, pos, WHITE);
    if (!N && !E && NE) DrawTextureRec(texture, (Rectangle){ src.x + TILE_SIZE + TILE_HALF, src.y, TILE_HALF, TILE_HALF }, (Vector2){ pos.x + TILE_HALF, pos.y }, WHITE);
    if (!S && !W && SW) DrawTextureRec(texture, (Rectangle){ src.x + TILE_SIZE, src.y + TILE_HALF, TILE_HALF, TILE_HALF }, (Vector2){ pos.x, pos.y + TILE_HALF }, WHITE);
    if (!S && !E && SE) DrawTextureRec(texture, (Rectangle){ src.x + TILE_SIZE + TILE_HALF, src.y + TILE_HALF, TILE_HALF, TILE_HALF }, (Vector2){ pos.x + TILE_HALF, pos.y + TILE_HALF }, WHITE);
}

void DrawTilemap(const Tilemap *tilemap)
{
    for (int l = 0; l < tilemap->layerCount; l++) {
        const TileLayer *layer = &tilemap->layers[l];

        for (int y = 0; y < layer->height; y++) {
            for (int x = 0; x < layer->width; x++) {
                const unsigned int gid = layer->data[y * layer->width + x];
                if (gid == 0) continue;

                const int tsIndex = FindTilesetIndex(tilemap, gid);
                if (tsIndex == -1) continue;

                const Tileset *ts = &tilemap->tilesets[tsIndex];
                const int localId = (int)gid - ts->firstGid;
                const int tilesPerRow = ts->texture.width / tilemap->tileWidth;

                const Rectangle src = {
                    .x = (float)(localId % tilesPerRow) * (float)tilemap->tileWidth,
                    .y = localId / tilesPerRow * (float)tilemap->tileHeight,
                    .width  = (float)tilemap->tileWidth,
                    .height = (float)tilemap->tileHeight
                };

                const Vector2 pos = {
                    .x = (float)(x * tilemap->tileWidth),
                    .y = (float)(y * tilemap->tileHeight)
                };

                TileType type = TILE_GROUND;
                if (ts->properties && ts->propertyCount > 0) {
                    for (int i = 0; i < ts->propertyCount; i++) {
                        if (ts->properties[i].id == localId) {
                            type = ts->properties[i].type;
                            break;
                        }
                    }
                }

                if (type == TILE_BORDER) continue;

                switch (type)
                {
                    case TILE_WALL: DrawWallTile(tilemap, ts, src, pos, x, y); break;
                    case TILE_CARPET: DrawCarpetTile(tilemap, ts, src, pos, x, y); break;
                    case TILE_TABLE: DrawTableTile(tilemap, ts, src, pos, x, y); break;
                    default: DrawTextureRec(ts->texture, src, pos, WHITE); break;
                }
            }
        }
    }

    for (int l = 0; l < tilemap->layerCount; l++) {
        const TileLayer *layer = &tilemap->layers[l];

        for (int y = 0; y < layer->height; y++) {
            for (int x = 0; x < layer->width; x++) {
                const unsigned int gid = layer->data[y * layer->width + x];
                if (gid == 0) continue;

                const int tsIndex = FindTilesetIndex(tilemap, gid);
                if (tsIndex == -1) continue;

                const Tileset *ts = &tilemap->tilesets[tsIndex];
                const int localId = (int)gid - ts->firstGid;
                const int tilesPerRow = ts->texture.width / tilemap->tileWidth;

                const Rectangle src = {
                    .x = (float)(localId % tilesPerRow) * (float)tilemap->tileWidth,
                    .y = localId / tilesPerRow * (float)tilemap->tileHeight,
                    .width  = (float)tilemap->tileWidth,
                    .height = (float)tilemap->tileHeight
                };

                const Vector2 pos = {
                    .x = (float)(x * tilemap->tileWidth),
                    .y = (float)(y * tilemap->tileHeight)
                };

                TileType type = TILE_GROUND;
                if (ts->properties && ts->propertyCount > 0) {
                    for (int i = 0; i < ts->propertyCount; i++) {
                        if (ts->properties[i].id == localId) {
                            type = ts->properties[i].type;
                            break;
                        }
                    }
                }

                if (type == TILE_BORDER) {
                    DrawBorderTile(tilemap, ts, src, pos, x, y);
                }
            }
        }
    }
}

TileType GetTileType(const Tilemap *tilemap, const int layerIndex,
                     const int x, const int y)
{
    if (!IsTileValid(x, y, tilemap->width, tilemap->height)) return TILE_NONE;
    if (layerIndex < 0 || layerIndex >= tilemap->layerCount) return TILE_NONE;

    const TileLayer *layer = &tilemap->layers[layerIndex];
    const unsigned int gid = layer->data[y * layer->width + x];

    if (gid == 0) return TILE_NONE;

    const int tsIndex = FindTilesetIndex(tilemap, gid);
    if (tsIndex == -1) return TILE_NONE;

    const Tileset *ts = &tilemap->tilesets[tsIndex];
    const int localId = (int)gid - ts->firstGid;

    if (ts->properties && ts->propertyCount > 0) {
        for (int i = 0; i < ts->propertyCount; i++) {
            if (ts->properties[i].id == localId) {
                return ts->properties[i].type;
            }
        }
    }

    return TILE_NONE;
}


