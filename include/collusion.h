#ifndef IVY_COLLUSION_H
#define IVY_COLLUSION_H

#include "raylib.h"
#include "tilemap.h"

typedef struct {
    Rectangle *rect;
    unsigned int rectCount;
    unsigned int width, height;
} Collusion;

Collusion CreateCollusion(const Tilemap *tilemap, int layerIndex);
void SetupCollusion(Collusion *collusion, const Tilemap *tilemap, int layerIndex);
void DestroyCollusion(Collusion *collusion);

#endif