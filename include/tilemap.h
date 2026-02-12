#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdint.h>

#include "raylib.h"

#define TILE_SIZE   32
#define SUB_TILE    16

typedef enum {
    LAYER_GROUND = 0,
    LAYER_COLLUSION
} LayerType;

typedef enum {
    CORNER_TOP_LEFT = 0,
    CORNER_TOP_RIGHT,
    CORNER_BOTTOM_RIGHT,
    CORNER_BOTTOM_LEFT,
    CORNER_MAX_SIZE
} CornerSide;

typedef enum {
    EDGE_TOP = 0,
    EDGE_RIGHT,
    EDGE_BOTTOM,
    EDGE_LEFT,
    EDGE_MAX_SIZE
} EdgeSide;

typedef enum {
    NEIGHBOR_N  = 1 << 0,   // 1
    NEIGHBOR_S  = 1 << 1,   // 2
    NEIGHBOR_E  = 1 << 2,   // 4
    NEIGHBOR_W  = 1 << 3,   // 8
    NEIGHBOR_NW = 1 << 4,   // 16
    NEIGHBOR_NE = 1 << 5,   // 32
    NEIGHBOR_SW = 1 << 6,   // 64
    NEIGHBOR_SE = 1 << 7    // 128
} TileNeighbor;

typedef enum {
    AUTOTILE_GROUND,
    AUTOTILE_WALL
} AutoTileType;

typedef struct {
    Texture2D texture;
    Vector2 position;
    AutoTileType type;
    union {
        struct {
            Rectangle cornersIn[CORNER_MAX_SIZE];
            Rectangle cornersOut[CORNER_MAX_SIZE];
            Rectangle edges[EDGE_MAX_SIZE];
        } ground;

        struct {
            Rectangle edges[EDGE_MAX_SIZE];
        } wall;
    };
} AutoTile;

typedef struct {
    Texture2D texture;
    int firstGid;
    char *source;
} Tileset;

typedef struct {
    char *name;
    uint32_t *data;
    uint32_t width;
    uint32_t height;
    float offsetX;
    float offsetY;
    bool visible;
} TileLayer;

typedef struct {
    Rectangle *bounds;
    unsigned count;
} CollusionData;

typedef struct {
    int width, height;
    int tileWidth, tileHeight;

    Tileset *tilesets;
    int tilesetCount;

    TileLayer *layers;
    int layerCount;

    CollusionData collusion;
    AutoTile *autoTile;
} Tilemap;


Tilemap LoadTilemap(const char *jsonPath);
void    UnloadTilemap(const Tilemap *tilemap);
bool    LoadAutoTile(Tilemap *tilemap, const char *texturePath, Vector2 position);
void    DrawAutoTile(const Tilemap *tilemap);

RenderTexture2D GenerateTilemapRenderTexture(const Tilemap *tilemap);


#endif