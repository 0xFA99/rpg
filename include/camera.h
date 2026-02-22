#ifndef CAMERA_H
#define CAMERA_H

#include "tilemap.h"
#include "player.h"

typedef struct {
    Camera2D camera;
    Vector2 offset;
    Vector2 boundsMin;
    Vector2 boundsMax;
    float followSpeed;
    float zoom;
    float zoomTarget;
    bool smoothFollow;
} GameCamera;

GameCamera InitGameCamera(int screenWidth, int screenHeight);
void UpdateCamera2D(GameCamera* camera, const Player* player, const Tilemap* map, float frameTime);

#endif