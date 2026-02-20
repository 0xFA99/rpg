#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdbool.h>
#include "raylib.h"

typedef enum {
    TILE_NONE = 0,
    TILE_GROUND,
    TILE_WALL,
    TILE_BORDER,
    TILE_COLLUSION,
    TILE_CARPET,
    TILE_TABLE
} TileType;

typedef struct {
    int id;
    TileType type;
} TileProperties;

typedef struct {
    Texture2D texture;
    char *texturePath;
    int firstGid;
    TileProperties *properties;
    int propertyCount;
} Tileset;

typedef struct {
    unsigned int *data;
    int width;
    int height;
} TileLayer;

typedef struct {
    int width;
    int height;
    int tileWidth;
    int tileHeight;

    Tileset *tilesets;
    int tilesetCount;

    TileLayer *layers;
    int layerCount;

    int mapId;
    int spawnPointX;
    int spawnPointY;
} Tilemap;

Tilemap LoadTilemapBinary(const char *binPath);
Tilemap LoadTilemapById(int mapId);

void UnloadTilemap(Tilemap *tilemap);

void DrawTilemap(const Tilemap *tilemap);

TileType GetTileType(const Tilemap *tilemap, int layerIndex, int x, int y);

#endif
