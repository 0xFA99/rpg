#ifndef COLLISION_H
#define COLLISION_H

#include "tilemap.h"

typedef struct {
    int x;
    int y;
    int w;
    int h;
} RectInfo;

typedef struct {
    Rectangle *rect;
    unsigned int rectCount;
} Collision;

Collision *InitCollisionAllLayers(const Tilemap *tilemap);
void DestroyCollision(Collision *collision);

#endif