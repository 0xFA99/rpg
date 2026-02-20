#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "collusion.h"
#include "game.h"

static bool
IsSolidTile(const Tilemap *tilemap, const int layerIndex, const int x, const int y)
{
    const TileLayer *layer = &tilemap->layers[layerIndex];
    if (x < 0 || x >= layer->width || y < 0 || y >= layer->height) return false;

    if (layer->data[y * layer->width + x] <= 0) return false;

    const TileType type = GetTileType(tilemap, layerIndex, x, y);
    return type == TILE_BORDER || type == TILE_WALL || type == TILE_COLLUSION || type == TILE_TABLE;
}

Collusion InitCollusion(const Tilemap *tilemap, const int layerIndex)
{
    assert(tilemap != NULL && "[ERROR] Tilemap not found!\n");
    const TileLayer *layer = &tilemap->layers[layerIndex];

    Collusion collusion = {0};
    collusion.rect = NULL;
    collusion.rectCount = 0;

    bool *visited = calloc(layer->width * layer->height, sizeof(bool));

    for (int y = 0; y < layer->height; y++) {
        for (int x = 0; x < layer->width; x++) {
            if (IsSolidTile(tilemap, layerIndex, x, y) && !visited[y * layer->width + x]) {
                int currentW = 1;
                while (x + currentW < layer->width &&
                       IsSolidTile(tilemap, layerIndex, x + currentW, y) &&
                       !visited[y * layer->width + (x + currentW)]) {
                    currentW++;
                }

                int currentH = 1;
                while (y + currentH < layer->height) {
                    bool canExpand = true;
                    for (int i = 0; i < currentW; i++) {
                        if (!IsSolidTile(tilemap, layerIndex, x + i, y + currentH) ||
                            visited[(y + currentH) * layer->width + (x + i)]) {
                            canExpand = false;
                            break;
                        }
                    }
                    if (canExpand) currentH++;
                    else break;
                }

                for (int row = 0; row < currentH; row++) {
                    for (int col = 0; col < currentW; col++) {
                        visited[(y + row) * layer->width + (x + col)] = true;
                    }
                }

                collusion.rectCount++;
            }
        }
    }

    collusion.rect = malloc(collusion.rectCount * sizeof(Rectangle));
    assert(collusion.rect != NULL && "[ERROR] Failed alloc collusion!\n");

    memset(visited, 0, layer->width * layer->height * sizeof(bool));

    int rectIdx = 0;
    for (int y = 0; y < layer->height; y++) {
        for (int x = 0; x < layer->width; x++) {
            if (IsSolidTile(tilemap, layerIndex, x, y) && !visited[y * layer->width + x]) {

                int currentW = 1;
                while (x + currentW < layer->width &&
                       IsSolidTile(tilemap, layerIndex, x + currentW, y) &&
                       !visited[y * layer->width + (x + currentW)]) {
                    currentW++;
                }

                int currentH = 1;
                while (y + currentH < layer->height) {
                    bool canExpand = true;
                    for (int i = 0; i < currentW; i++) {
                        if (!IsSolidTile(tilemap, layerIndex, x + i, y + currentH) ||
                            visited[(y + currentH) * layer->width + (x + i)]) {
                            canExpand = false;
                            break;
                        }
                    }
                    if (canExpand) currentH++;
                    else break;
                }

                for (int row = 0; row < currentH; row++) {
                    for (int col = 0; col < currentW; col++) {
                        visited[(y + row) * layer->width + (x + col)] = true;
                    }
                }

                collusion.rect[rectIdx].x = (float)x * TILE_SIZE;
                collusion.rect[rectIdx].y = (float)y * TILE_SIZE;
                collusion.rect[rectIdx].width = (float)currentW * TILE_SIZE;
                collusion.rect[rectIdx].height = (float)currentH * TILE_SIZE;
                rectIdx++;
            }
        }
    }

    free(visited);
    return collusion;
}

void DestroyCollusion(Collusion *collusion)
{
    if (collusion->rect) free(collusion->rect);
    collusion->rect = NULL;
    collusion->rectCount = 0;
}
