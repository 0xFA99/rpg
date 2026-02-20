#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"
#include "tilemap.h"
#include "player.h"

typedef struct {
    Camera2D camera;
    float followSpeed;
    float zoom;
    float zoomTarget;
    Vector2 offset;
    bool smoothFollow;
    Vector2 boundsMin;
    Vector2 boundsMax;
} GameCamera;

GameCamera InitGameCamera(int screenWidth, int screenHeight);
void UpdateCamera2D(GameCamera* camera, const Player* player, const Tilemap* map, float frameTime);

#endif