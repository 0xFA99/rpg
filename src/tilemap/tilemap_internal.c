#include "game/tilemap/tilemap_internal.h"
#include "game/utils.h"

#include <stdlib.h>
#include <assert.h>

#define EXTRA_GID 256

// TILEMAP HELPER
// TODO: feat: Using hash-map
u32 FindMaxGidInMap(const Tilemap *tilemap)
{
    u32 max = 0;

    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const u32 totalCells = tilemap->layers[l].width * tilemap->layers[l].height;

        for (u32 i = 0; i < totalCells; i++) {
            if (tilemap->layers[l].data[i] > max) {
                max = tilemap->layers[l].data[i];
            }
        }
    }

    return max;
}

void BuildTileTable(Tilemap *tilemap)
{
    const u32 maxGid = FindMaxGidInMap(tilemap);
    tilemap->maxGid = maxGid + EXTRA_GID;

    tilemap->tileTypeTable      = calloc(tilemap->maxGid + 1, sizeof(u8));
    tilemap->tilesetIndexTable  = calloc(tilemap->maxGid + 1, sizeof(u8));
    tilemap->tileDrawInfoTable  = calloc(tilemap->maxGid + 1, sizeof(TileDrawInfo));

    for (u32 tsIdx = 0; tsIdx < tilemap->header.tilesetCount; tsIdx++) {
        const Tileset *ts = &tilemap->tilesets[tsIdx];

        for (u32 p = 0; p < ts->propertyCount; p++) {
            const u32 globalGid = ts->firstGid + ts->properties[p].id;

            if (globalGid > tilemap->maxGid) continue;

            tilemap->tileTypeTable[globalGid] = (u8)ts->properties[p].type;
            tilemap->tilesetIndexTable[globalGid] = (u8)tsIdx;

            const u32 localId = ts->properties[p].id;
            const u32 tilesPerRow = ts->texture.width / 32;

            tilemap->tileDrawInfoTable[globalGid] = (TileDrawInfo) {
                .src = (Rectangle) {
                    .x      = (float)(localId % tilesPerRow) * 32.0f,
                    .y      = (float)(localId / tilesPerRow) * 32.0f,
                    .width  = 32.0f,
                    .height = 32.0f
                },
                .type = ts->properties[p].type,
                .tileset = ts
            };
        }
    }
}

int FindTilesetIndex(const Tilemap *tilemap, const u32 gid)
{
    for (int i = (int)tilemap->header.tilesetCount - 1; i >= 0; i--)
    {
        if (gid >= tilemap->tilesets[i].firstGid) return i;
    }

    return -1;
}

TileType GetTileType(const Tilemap *tilemap, const u32 layerIndex, const u32 x, const u32 y)
{
    if (x >= tilemap->header.width || y >= tilemap->header.height) return TILE_NONE;
    if (layerIndex >= tilemap->header.layerCount) return TILE_NONE;

    const u32 gid = tilemap->layers[layerIndex].data[y * tilemap->header.width + x];

    if (gid > 0 && gid <= tilemap->maxGid) return tilemap->tileTypeTable[gid];

    return TILE_NONE;
}

// TILEMAP SETUP
void LoadTilemapHeader(FILE *file, Tilemap *tilemap)
{
    ReadExact(file, &tilemap->header, sizeof(u32) * (sizeof(TilemapHeader) / sizeof(u32)));
}

void LoadTilesets(FILE *file, Tilemap *tilemap)
{
    tilemap->tilesets = malloc(sizeof(Tileset) * tilemap->header.tilesetCount);
    assert(tilemap->tilesets && "[ERROR] Failed to allocate memory tilesets!");

    char pathBuffer[PATH_LEN] = {0};

    for (u32 i = 0; i < tilemap->header.tilesetCount; i++)
    {
        Tileset *tilesetIndex = &tilemap->tilesets[i];

        // Read firstGid & propertyCount
        ReadExact(file, &tilesetIndex->firstGid, sizeof(u32));
        ReadExact(file, &tilesetIndex->propertyCount, sizeof(u32));

        // Get Texture Path
        tilesetIndex->texturePath = ReadString(file);

        tilesetIndex->properties = malloc(tilesetIndex->propertyCount * sizeof(TileProp));
        assert(tilesetIndex->properties && "[ERROR] Failed to allocate memory tile properties!");

        ReadExact(file, tilesetIndex->properties, sizeof(TileProp) * tilesetIndex->propertyCount);

        snprintf(pathBuffer, PATH_LEN, "%s/%s", TILESET_PATH, (const char *)tilesetIndex->texturePath);
        tilesetIndex->texture = LoadTextureFromBin(pathBuffer);
    }
}

void LoadLayers(FILE *file, Tilemap *tilemap)
{
    tilemap->layers = malloc(tilemap->header.layerCount * sizeof(Layer));
    assert(tilemap->layers && "[ERROR] Failed to allocate memory for layers!");

    for (u32 i = 0; i < tilemap->header.layerCount; i++)
    {
        Layer *layer = &tilemap->layers[i];

        // Read width & height
        ReadExact(file, &layer->width, sizeof(u32) * 2);

        const u32 cellCount = layer->width * layer->height;

        layer->data = malloc(sizeof(u32) * cellCount);
        assert(layer->data && "[ERROR] Failed to allocate memory for layer data!");

        ReadExact(file, layer->data, cellCount * sizeof(u32));
    }
}

void DrawTileById(const Tilemap *tilemap, const TileDrawInfo *info, const u32 x, const u32 y)
{
    switch (info->type)
    {
        case TILE_WALL:     DrawWallTile(tilemap, info->tileset, info->src, info->pos, x, y); break;
        case TILE_CARPET:   DrawCarpetTile(tilemap, info->tileset, info->src, info->pos, x, y); break;
        case TILE_TABLE:    DrawTableTile(tilemap, info->tileset, info->src, info->pos, x, y); break;
        case TILE_BORDER:   DrawBorderTile(tilemap, info->tileset, info->src, info->pos, x, y); break;

        default:            DrawTextureRec(info->tileset->texture, info->src, info->pos, WHITE); break;
    }
}

TileDrawInfo
GetTileDrawInfo(const Tilemap *tilemap, const Layer *layer, const u32 x, const u32 y)
{
    const u32 gid = layer->data[y * layer->width + x];
    if (gid == 0 || gid > tilemap->maxGid) return (TileDrawInfo){0};

    TileDrawInfo info = tilemap->tileDrawInfoTable[gid];

    if (info.tileset == NULL || info.tileset->texture.id == 0) {
        const int tsIdx = FindTilesetIndex(tilemap, gid);
        if (tsIdx < 0) return (TileDrawInfo){0};

        const Tileset *ts = &tilemap->tilesets[tsIdx];
        const u32 localId = gid - ts->firstGid;
        const u32 tilesPerRow = ts->texture.width / tilemap->header.tileWidth;

        info = (TileDrawInfo) {
            .src = (Rectangle) {
                .x      = (float)(localId % tilesPerRow) * (float)tilemap->header.tileWidth,
                .y      = (float)(localId / tilesPerRow) * (float)tilemap->header.tileHeight,
                .width  = (float)tilemap->header.tileWidth,
                .height = (float)tilemap->header.tileHeight
            },
            .type = TILE_GROUND,
            .tileset = ts
        };

        for (u32 p = 0; p < ts->propertyCount; p++) {
            if (ts->properties[p].id == localId) {
                info.type = ts->properties[p].type;
                break;
            }
        }
    }

    info.pos = (Vector2) {
        .x = (float)x * (float)tilemap->header.tileWidth,
        .y = (float)y * (float)tilemap->header.tileHeight
    };

    return info;
}

void DrawNonBorderTiles(const Tilemap *tilemap)
{
    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const Layer *layer = &tilemap->layers[l];

        for (u32 y = 0; y < layer->height; y++) {
            for (u32 x = 0; x < layer->width; x++)
            {
                TileDrawInfo info = GetTileDrawInfo(tilemap, layer, x, y);
                if (info.type == TILE_NONE || info.type == TILE_BORDER) continue;

                DrawTileById(tilemap, &info, x, y);
            }
        }
    }
}

void DrawBorderTiles(const Tilemap *tilemap)
{
    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const Layer *layer = &tilemap->layers[l];

        for (u32 y = 0; y < layer->height; y++) {
            for (u32 x = 0; x < layer->width; x++)
            {
                TileDrawInfo info = GetTileDrawInfo(tilemap, layer, x, y);
                if (info.type != TILE_BORDER) continue;
                DrawTileById(tilemap, &info, x, y);
            }
        }
    }
}

