#include "game/tilemap/tilemap_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


Tilemap *LoadTilemapById(const u32 id)
{
    char path[PATH_LEN] = {0};
    snprintf(path, PATH_LEN, "%s/map_%d.bin", TILEMAP_PATH, id);

    FILE *file = fopen(path, "rb");
    assert(file && "[ERROR] Failed to open file!");

    Tilemap *tilemap = malloc(sizeof(Tilemap));
    assert(tilemap && "[ERROR] Failed to allocate memory tilemap!");

    LoadTilemapHeader(file, tilemap);
    LoadTilesets(file, tilemap);
    LoadLayers(file, tilemap);
    BuildTileTable(tilemap);

    fclose(file);
    return tilemap;
}

void UnloadTilemap(Tilemap *tilemap)
{
    if (!tilemap) return;

    if (tilemap->tilesets) {
        for (u32 i = 0; i < tilemap->header.tilesetCount; i++) {
            UnloadTexture(tilemap->tilesets[i].texture);

            free(tilemap->tilesets[i].texturePath);
            free(tilemap->tilesets[i].properties);
        }
        free(tilemap->tilesets);
    }

    if (tilemap->layers) {
        for (u32 i = 0; i < tilemap->header.layerCount; i++) {
            free(tilemap->layers[i].data);
        }
        free(tilemap->layers);
    }

    free(tilemap->tileDrawInfoTable);
    free(tilemap->tilesetIndexTable);
    free(tilemap->tileTypeTable);

    free(tilemap);
}

void DrawTilemap(const Tilemap *tilemap)
{
    assert(tilemap && "[ERROR] Tilemap not found!");

    DrawNonBorderTiles(tilemap);
    DrawBorderTiles(tilemap);
}
