#include <assert.h>
#include <stdlib.h>

#include "collusion.h"
#include "tilemap.h"

static bool
IsSolidTile(const Tilemap *tilemap, const int layerIndex, const int x, const int y)
{
    const TileLayer *layer = &tilemap->layers[layerIndex];
    if (x < 0 || x >= layer->width || y < 0 || y >= layer->height) return false;

    if (layer->data[y * layer->width + x] <= 0) return false;

    const TileType type = GetTileType(tilemap, layerIndex, x, y);
    return type == TILE_BORDER || type == TILE_WALL || type == TILE_COLLUSION;
}

Collusion CreateCollusion(const Tilemap *tilemap, const int layerIndex)
{
    assert(tilemap != NULL && "[ERROR] Tilemap not found!\n");
    const TileLayer *layer = &tilemap->layers[layerIndex];

    bool *visited = calloc(layer->width * layer->height, sizeof(bool));
    unsigned int totalRect = 0;

    for (int y = 0; y < layer->height; y++) {
        for (int x = 0; x < layer->width; x++) {
            if (IsSolidTile(tilemap, layerIndex, x, y) && !visited[y * layer->width + x]) {
                totalRect++;

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
            }
        }
    }

    free(visited);

    Collusion collusion = {0};
    collusion.rectCount = totalRect;
    collusion.rect = malloc(totalRect * sizeof(Rectangle));
    assert(collusion.rect != NULL && "[ERROR] Failed alloc collusion!\n");

    return collusion;
}

void SetupCollusion(Collusion *collusion, const Tilemap *tilemap, const int layerIndex)
{
    if (!collusion || !tilemap) return;
    const TileLayer *layer = &tilemap->layers[layerIndex];

    bool *visited = calloc(layer->width * layer->height, sizeof(bool));
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

                if (rectIdx < collusion->rectCount) {
                    collusion->rect[rectIdx].x = (float)x * 32;
                    collusion->rect[rectIdx].y = (float)y * 32;
                    collusion->rect[rectIdx].width = (float)currentW * 32;
                    collusion->rect[rectIdx].height = (float)currentH * 32;
                    rectIdx++;
                }
            }
        }
    }
    free(visited);
}

void DestroyCollusion(Collusion *collusion)
{
    if (collusion->rect) free(collusion->rect);
    collusion->rect = 0;
    collusion->width = 0;
    collusion->height = 0;
}
