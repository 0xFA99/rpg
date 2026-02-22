#include <assert.h>
#include <stdlib.h>

#include "collision.h"
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

static void
ComputeCollisionRects(const Tilemap *tilemap, const int layerIndex,
                      RectInfo **outRects, int *outCount, int *outCapacity)
{
    const TileLayer *layer  = &tilemap->layers[layerIndex];
    bool *visited           = calloc(layer->width * layer->height, sizeof(bool));
    int rectCount           = 0;
    int rectCapacity        = 0;
    RectInfo *rects         = NULL;

    for (int y = 0; y < layer->height; y++)
    {
        for (int x = 0; x < layer->width; x++)
        {
            if (IsSolidTile(tilemap, layerIndex, x, y) && !visited[y * layer->width + x])
            {
                int currentW = 1;
                while (x + currentW < layer->width &&
                       IsSolidTile(tilemap, layerIndex, x + currentW, y) &&
                       !visited[y * layer->width + (x + currentW)])
                {
                    currentW++;
                }

                int currentH = 1;
                while (y + currentH < layer->height)
                {
                    bool canExpand = true;
                    for (int i = 0; i < currentW; i++)
                    {
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

                if (rectCount >= rectCapacity)
                {
                    rectCapacity = rectCapacity == 0 ? 16 : rectCapacity * 2;
                    RectInfo *newRectInfo = realloc(rects, rectCapacity * sizeof(RectInfo));
                    assert(newRectInfo != NULL && "[ERROR] Failed to reallocation newRectInfo");
                    rects = newRectInfo;
                }

                assert(rects != NULL);
                rects[rectCount] = (RectInfo){x, y, currentW, currentH};
                rectCount++;
            }
        }
    }

    free(visited);
    *outRects = rects;
    *outCount = rectCount;
    *outCapacity = rectCapacity;
}

Collision *InitCollisionAllLayers(const Tilemap *tilemap)
{
    assert(tilemap != NULL && "[ERROR] Tilemap not found!\n");

    Collision *collision = malloc(sizeof(Collision));
    collision->rect = NULL;
    collision->rectCount = 0;

    RectInfo *allRects = NULL;
    int allCount = 0;
    int allCapacity = 0;

    for (int l = 0; l < tilemap->layerCount; l++)
    {
        RectInfo *layerRects = NULL;
        int layerCount = 0;
        int layerCapacity = 0;

        ComputeCollisionRects(tilemap, l, &layerRects, &layerCount, &layerCapacity);

        for (int i = 0; i < layerCount; i++)
        {
            if (allCount >= allCapacity)
            {
                allCapacity = allCapacity == 0 ? 16 : allCapacity * 2;
                RectInfo *newRectInfo = realloc(allRects, allCapacity * sizeof(RectInfo));
                assert(newRectInfo != NULL && "[ERROR] Failed to re-allocation newRectInfo\n");
                allRects = newRectInfo;
            }

            assert(allRects != NULL);
            allRects[allCount] = layerRects[i];
            allCount++;
        }

        free(layerRects);
    }

    assert(allRects != NULL);
    collision->rect = malloc(allCount * sizeof(Rectangle));
    assert(collision->rect != NULL && "[ERROR] Failed alloc collision!\n");

    for (int i = 0; i < allCount; i++)
    {
        collision->rect[i].x         = (float)allRects[i].x * TILE_SIZE;
        collision->rect[i].y         = (float)allRects[i].y * TILE_SIZE;
        collision->rect[i].width     = (float)allRects[i].w * TILE_SIZE;
        collision->rect[i].height    = (float)allRects[i].h * TILE_SIZE;
    }
    collision->rectCount = allCount;

    free(allRects);
    return collision;
}

void DestroyCollision(Collision *collision)
{
    if (collision->rect) free(collision->rect);

    collision->rect = NULL;
    collision->rectCount = 0;
}
