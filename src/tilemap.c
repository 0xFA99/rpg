#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tilemap.h"
#include "cJSON.h"
#include "game.h"
#include "raylib.h"
#include "helper.h"


#define MAX_PATH_LENGTH 256
#define MAX_NAME_LENGTH 64
#define TILESET_PATH_PREFIX "../assets/tilesets/"


static bool LoadLayers(Tilemap* tilemap, const cJSON* layers_node);
static bool LoadTilesets(Tilemap *map, const cJSON *tilesets);
static bool IsTilePresent(const TileLayer* layer, int x, int y, int W, int H);
static void RenderBaseTiles(const Tilemap* map);
static void RenderAutotileEdges(const Tilemap* map, const TileLayer* layer, int W, int H, float TW, float TH);
static void RenderAutotileCorners(const Tilemap* map, const TileLayer* layer, int W, int H, float TW, float TH);
static uint8_t GetTileBitmask(const TileLayer *layer, int x, int y, int W, int H);

static bool
LoadLayers(Tilemap* tilemap, const cJSON* layers_node)
{
    const int total = cJSON_GetArraySize(layers_node);
    if (total <= 0) {
        fprintf(stderr, "[WARNING] No layers found\n");
        return false;
    }

    tilemap->layers = calloc(total, sizeof(TileLayer));
    if (!tilemap->layers) {
        fprintf(stderr, "[ERROR] Memory allocation failed for layers\n");
        return false;
    }

    tilemap->layerCount = 0;

    const cJSON* item = NULL;
    cJSON_ArrayForEach(item, layers_node)
    {
        if (tilemap->layerCount >= total) break;

        TileLayer* layer = &tilemap->layers[tilemap->layerCount];

        const cJSON* data_arr   = cJSON_GetObjectItem(item, "data");
        const cJSON* name       = cJSON_GetObjectItem(item, "name");
        const cJSON* id_obj     = cJSON_GetObjectItem(item, "id");
        const cJSON* width_obj  = cJSON_GetObjectItem(item, "width");
        const cJSON* height_obj = cJSON_GetObjectItem(item, "height");

        if (!cJSON_IsNumber(id_obj) || !cJSON_IsNumber(width_obj) ||
            !cJSON_IsNumber(height_obj) || !cJSON_IsArray(data_arr)) {
            fprintf(stderr, "[WARNING] Invalid layer entry\n");
            continue;
        }

        layer->width    = width_obj->valueint;
        layer->height   = height_obj->valueint;
        layer->visible  = cJSON_IsTrue(cJSON_GetObjectItem(item, "visible"));
        layer->name     = name && name->valuestring ? strdup(name->valuestring) : NULL;

        const int dataSize = cJSON_GetArraySize(data_arr);
        if (dataSize > 0) {
            layer->data = malloc(sizeof(unsigned int) * dataSize);
            if (!layer->data) {
                fprintf(stderr, "[ERROR] Memory allocation failed for layer data\n");
                if (layer->name) free(layer->name);
                continue;
            }

            int i = 0;
            const cJSON* val = NULL;
            cJSON_ArrayForEach(val, data_arr) {
                if (i >= dataSize) break;
                layer->data[i++] = (unsigned int)val->valueint;
            }
        }

        tilemap->layerCount++;
    }

    return tilemap->layerCount > 0;
}

static bool
LoadTilesets(Tilemap *map, const cJSON *tilesets)
{
    if (!tilesets) return false;

    const int total = cJSON_GetArraySize(tilesets);
    if (total <= 0) return false;

    map->tilesets = calloc(total, sizeof(Tileset));
    if (!map->tilesets) return false;

    map->tilesetCount = 0;
    const cJSON *item = NULL;

    cJSON_ArrayForEach(item, tilesets)
    {
        const cJSON *firstGid = cJSON_GetObjectItem(item, "firstgid");
        const cJSON *source   = cJSON_GetObjectItem(item, "source");

        if (!cJSON_IsNumber(firstGid) || !cJSON_IsString(source)) continue;

        const char *fullPath = source->valuestring;
        const char *fileName = strrchr(fullPath, '/');
        fileName = !fileName ? fullPath : fileName + 1;

        char baseName[SHORT_STRING] = {0};
        size_t len = strcspn(fileName, ".");
        if (len >= SHORT_STRING) len = SHORT_STRING - 1;
        strncpy(baseName, fileName, len);

        char finalPath[MAX_STRING];
        snprintf(finalPath, sizeof(finalPath), "%s%s.png", TILESET_PATH_PREFIX, baseName);

        Tileset *ts = &map->tilesets[map->tilesetCount];
        const Texture2D tex = LoadTexture(finalPath);

        if (tex.id > 0)
        {
            ts->firstGid = firstGid->valueint;
            ts->source   = strdup(finalPath);
            ts->texture  = tex;
            map->tilesetCount++;
        } else {
            fprintf(stderr, "[ERROR] Failed to load tileset texture: %s\n", finalPath);
        }
    }

    return map->tilesetCount > 0;
}

static bool
IsTilePresent(const TileLayer* layer, const int x, const int y, const int W, const int H)
{
    return x >= 0 && x < W && y >= 0 && y < H && layer->data[y * W + x] != 0;
}

static uint8_t
GetTileBitmask(const TileLayer *layer, int x, int y, int W, int H)
{
    uint8_t mask = 0;

    if (IsTilePresent(layer, x, y - 1, W, H)) mask |= NEIGHBOR_N;
    if (IsTilePresent(layer, x, y + 1, W, H)) mask |= NEIGHBOR_S;
    if (IsTilePresent(layer, x + 1, y, W, H)) mask |= NEIGHBOR_E;
    if (IsTilePresent(layer, x - 1, y, W, H)) mask |= NEIGHBOR_W;

    if (IsTilePresent(layer, x - 1, y - 1, W, H)) mask |= NEIGHBOR_NW;
    if (IsTilePresent(layer, x + 1, y - 1, W, H)) mask |= NEIGHBOR_NE;
    if (IsTilePresent(layer, x - 1, y + 1, W, H)) mask |= NEIGHBOR_SW;
    if (IsTilePresent(layer, x + 1, y + 1, W, H)) mask |= NEIGHBOR_SE;

    return mask;
}

static void
RenderBaseTiles(const Tilemap* map)
{
    const TileLayer* layer  = &map->layers[0];
    const float TW          = (float)map->tileWidth;
    const float TH          = (float)map->tileHeight;
    const int W             = (int)layer->width;
    const int H             = (int)layer->height;

    const Texture2D atlas   = map->tilesets[0].texture;
    const int atlasCol      = atlas.width / (int)TW;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            const unsigned int gid = layer->data[y * W + x];
            if (gid == 0) continue;

            const int localId       = (int)gid - map->tilesets[0].firstGid;
            const Rectangle gSrc    = { (float)(localId % atlasCol) * TW, (float)(localId / atlasCol) * TH, TW, TH};
            const Vector2 gPos      = { (float)x * TW, (float)y * TH };

            DrawTextureRec(atlas, gSrc, gPos, WHITE);
        }
    }
}

static void
RenderAutotileEdges(const Tilemap* map, const TileLayer* layer, const int W, const int H, const float TW, const float TH)
{
    const float hW = TW / 2.0f;
    const float hH = TH / 2.0f;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (layer->data[y * W + x] == 0) continue;

            const uint8_t mask = GetTileBitmask(layer, x, y, W, H);
            const Vector2 gPos = { (float)x * TW, (float)y * TH };

            if (!(mask & NEIGHBOR_S)) DrawTextureRec(map->autoTile->texture, map->autoTile->ground.edges[EDGE_TOP],    (Vector2){ gPos.x, gPos.y + TH }, WHITE);
            if (!(mask & NEIGHBOR_W)) DrawTextureRec(map->autoTile->texture, map->autoTile->ground.edges[EDGE_RIGHT],  (Vector2){ gPos.x - hW, gPos.y }, WHITE);
            if (!(mask & NEIGHBOR_E)) DrawTextureRec(map->autoTile->texture, map->autoTile->ground.edges[EDGE_LEFT],   (Vector2){ gPos.x + TW, gPos.y }, WHITE);
            if (!(mask & NEIGHBOR_N)) DrawTextureRec(map->autoTile->texture, map->autoTile->ground.edges[EDGE_BOTTOM], (Vector2){ gPos.x, gPos.y - hH }, WHITE);
        }
    }
}

static void
RenderAutotileCorners(const Tilemap* map, const TileLayer* layer, const int W, const int H, const float TW, const float TH)
{
    const float hW = TW / 2.0f;
    const float hH = TH / 2.0f;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (layer->data[y * W + x] == 0) continue;

            const uint8_t mask = GetTileBitmask(layer, x, y, W, H);
            const Vector2 gPos = { (float)x * TW, (float)y * TH };

            if (!(mask & NEIGHBOR_W) && !(mask & NEIGHBOR_N))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersOut[CORNER_BOTTOM_RIGHT],
                              (Vector2){ gPos.x - hW, gPos.y - hH }, WHITE);
            if (!(mask & NEIGHBOR_E) && !(mask & NEIGHBOR_N))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersOut[CORNER_BOTTOM_LEFT],
                              (Vector2){ gPos.x + TW, gPos.y - hH }, WHITE);
            if (!(mask & NEIGHBOR_W) && !(mask & NEIGHBOR_S))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersOut[CORNER_TOP_RIGHT],
                              (Vector2){ gPos.x - hW, gPos.y + TH }, WHITE);
            if (!(mask & NEIGHBOR_E) && !(mask & NEIGHBOR_S))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersOut[CORNER_TOP_LEFT],
                              (Vector2){ gPos.x + TW, gPos.y + TH }, WHITE);

            if (mask & NEIGHBOR_N && mask & NEIGHBOR_W && !(mask & NEIGHBOR_NW))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersIn[CORNER_BOTTOM_RIGHT],
                              (Vector2){ gPos.x - hW, gPos.y - hH }, WHITE);
            if (mask & NEIGHBOR_N && mask & NEIGHBOR_E && !(mask & NEIGHBOR_NE))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersIn[CORNER_BOTTOM_LEFT],
                              (Vector2){ gPos.x + TW, gPos.y - hH }, WHITE);
            if (mask & NEIGHBOR_S && mask & NEIGHBOR_W && !(mask & NEIGHBOR_SW))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersIn[CORNER_TOP_LEFT],
                              (Vector2){ gPos.x - hW, gPos.y + TH }, WHITE);
            if (mask & NEIGHBOR_S && mask & NEIGHBOR_E && !(mask & NEIGHBOR_SE))
                DrawTextureRec(map->autoTile->texture, map->autoTile->ground.cornersIn[CORNER_TOP_RIGHT],
                              (Vector2){ gPos.x + TW, gPos.y + TH }, WHITE);
        }
    }
}

Tilemap LoadTilemap(const char *jsonPath)
{
    Tilemap tilemap = {0};

    char *jsonString = ReadFile2String(jsonPath);
    if (!jsonString) return tilemap;

    cJSON* root = cJSON_Parse(jsonString);
    if (!root)
    {
        fprintf(stderr, "[ERROR] JSON Parse Error\n");
        free(jsonString);
        return tilemap;
    }

    tilemap.width       = cJSON_GetObjectItem(root, "width")->valueint;
    tilemap.height      = cJSON_GetObjectItem(root, "height")->valueint;
    tilemap.tileWidth   = cJSON_GetObjectItem(root, "tilewidth")->valueint;
    tilemap.tileHeight  = cJSON_GetObjectItem(root, "tileheight")->valueint;

    const cJSON* tilesets_node  = cJSON_GetObjectItem(root, "tilesets");
    const cJSON* layers_node    = cJSON_GetObjectItem(root, "layers");

    const bool tilesets_ok  = LoadTilesets(&tilemap, tilesets_node);
    const bool layers_ok    = LoadLayers(&tilemap, layers_node);

    if (!tilesets_ok || !layers_ok) UnloadTilemap(&tilemap);

    cJSON_Delete(root);
    free(jsonString);

    return tilemap;
}

void UnloadTilemap(const Tilemap *tilemap)
{
    for (int i = 0; i < tilemap->tilesetCount; i++) {
        if (tilemap->tilesets[i].source) free(tilemap->tilesets[i].source);
        UnloadTexture(tilemap->tilesets[i].texture);
    }
    free(tilemap->tilesets);

    for (int i = 0; i < tilemap->layerCount; i++) {
        free(tilemap->layers[i].data);
        if (tilemap->layers[i].name) free(tilemap->layers[i].name);
    }
    free(tilemap->layers);

    if (tilemap->autoTile) {
        UnloadTexture(tilemap->autoTile->texture);
        free(tilemap->autoTile);
    }
}

bool LoadAutoTile(Tilemap *tilemap, const char *texturePath, const Vector2 position)
{
    if (!tilemap) {
        fprintf(stderr, "[ERROR] NULL tilemap provided\n");
        return false;
    }

    if (tilemap->autoTile) {
        UnloadTexture(tilemap->autoTile->texture);
        free(tilemap->autoTile);
        tilemap->autoTile = NULL;
    }

    tilemap->autoTile = malloc(sizeof(AutoTile));
    if (!tilemap->autoTile) {
        fprintf(stderr, "[ERROR] Memory allocation failed for autotile\n");
        return false;
    }

    tilemap->autoTile->texture = LoadTexture(texturePath);
    if (tilemap->autoTile->texture.id <= 0) {
        fprintf(stderr, "[ERROR] Failed to load autotile texture: %s\n", texturePath);
        free(tilemap->autoTile);
        tilemap->autoTile = NULL;
        return false;
    }

    const float x = position.x;
    const float y = position.y;

    tilemap->autoTile->ground.cornersIn[CORNER_TOP_LEFT]       = (Rectangle){ x + SUB_TILE, y, SUB_TILE, SUB_TILE };
    tilemap->autoTile->ground.cornersIn[CORNER_TOP_RIGHT]      = (Rectangle){ x, y, SUB_TILE, SUB_TILE };
    tilemap->autoTile->ground.cornersIn[CORNER_BOTTOM_LEFT]    = (Rectangle){ x, y + SUB_TILE, SUB_TILE, SUB_TILE };
    tilemap->autoTile->ground.cornersIn[CORNER_BOTTOM_RIGHT]   = (Rectangle){ x + SUB_TILE, y + SUB_TILE, SUB_TILE, SUB_TILE };

    tilemap->autoTile->ground.cornersOut[CORNER_TOP_LEFT]      = (Rectangle){ x + TILE_SIZE, y, SUB_TILE, SUB_TILE };
    tilemap->autoTile->ground.cornersOut[CORNER_TOP_RIGHT]     = (Rectangle){ x + TILE_SIZE + SUB_TILE, y, SUB_TILE, SUB_TILE };
    tilemap->autoTile->ground.cornersOut[CORNER_BOTTOM_LEFT]   = (Rectangle){ x + TILE_SIZE, y + SUB_TILE, SUB_TILE, SUB_TILE };
    tilemap->autoTile->ground.cornersOut[CORNER_BOTTOM_RIGHT]  = (Rectangle){ x + TILE_SIZE + SUB_TILE, y + SUB_TILE, SUB_TILE, SUB_TILE };

    tilemap->autoTile->ground.edges[EDGE_TOP]      = (Rectangle){ x + SUB_TILE, y + TILE_SIZE, TILE_SIZE, SUB_TILE };
    tilemap->autoTile->ground.edges[EDGE_RIGHT]    = (Rectangle){ x + TILE_SIZE + SUB_TILE, y + TILE_SIZE + SUB_TILE, SUB_TILE, TILE_SIZE };
    tilemap->autoTile->ground.edges[EDGE_BOTTOM]   = (Rectangle){ x + SUB_TILE, y + TILE_SIZE * 2.0f + SUB_TILE, TILE_SIZE, SUB_TILE };
    tilemap->autoTile->ground.edges[EDGE_LEFT]     = (Rectangle){ x, y + TILE_SIZE + SUB_TILE, SUB_TILE, TILE_SIZE };

    return true;
}

RenderTexture2D GenerateTilemapRenderTexture(const Tilemap *tilemap)
{
    if (!tilemap || tilemap->layerCount <= 0 || tilemap->tilesetCount <= 0)
    {
        fprintf(stderr, "[ERROR] Invalid tiletilemap for rendering\n");
        return (RenderTexture2D){0};
    }

    const RenderTexture2D canvas = LoadRenderTexture(tilemap->width * tilemap->tileWidth, tilemap->height * tilemap->tileHeight);

    BeginTextureMode(canvas);
    ClearBackground(BLANK);

    RenderBaseTiles(tilemap);

    if (tilemap->autoTile) {
        const TileLayer* layer = &tilemap->layers[0];
        const int W     = (int)layer->width;
        const int H     = (int)layer->height;
        const float TW  = (float)tilemap->tileWidth;
        const float TH  = (float)tilemap->tileHeight;

        RenderAutotileEdges(tilemap, layer, W, H, TW, TH);
        RenderAutotileCorners(tilemap, layer, W, H, TW, TH);
    }

    EndTextureMode();
    return canvas;
}
