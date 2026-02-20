#ifndef IVY_COLLUSION_H
#define IVY_COLLUSION_H

#include "raylib.h"
#include "tilemap.h"

typedef struct {
    Rectangle *rect;
    unsigned int rectCount;
} Collusion;

Collusion InitCollusion(const Tilemap *tilemap, int layerIndex);
void DestroyCollusion(Collusion *collusion);

#endif