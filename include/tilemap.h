#ifndef TILEMAP_H
#define TILEMAP_H

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
    int          id;
    TileType     type;
} TileProp;

typedef struct {
    Texture2D    texture;
    char        *texturePath;
    TileProp    *properties;
    int          firstGid;
    int          propertyCount;
} Tileset;

typedef struct {
    int         *data;
    int          width;
    int          height;
} TileLayer;

typedef struct {
    Tileset     *tilesets;
    TileLayer   *layers;

    int          width;
    int          height;
    int          tileWidth;
    int          tileHeight;

    int          tilesetCount;
    int          layerCount;
    int          mapId;

    int          spawnPointX;
    int          spawnPointY;
    int          eventGotoMapId;
    int          eventGotoMapTileX;
    int          eventGotoMapTileY;
} Tilemap;

typedef struct {
    Rectangle        src;
    Vector2          pos;
    TileType         type;
    const Tileset   *tileset;
} TileDrawInfo;

Tilemap *LoadTilemapBinary(const char *binPath);
Tilemap *LoadTilemapById(int mapId);

void     UnloadTilemap(Tilemap *tilemap);
void     DrawTilemap(const Tilemap *tilemap);

TileType GetTileType(const Tilemap *tilemap, int layerIndex, int x, int y);

#endif
