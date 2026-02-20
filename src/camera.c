#include "camera.h"
#include "raymath.h"

GameCamera InitGameCamera(const int screenWidth, const int screenHeight)
{
    GameCamera camera = {0};
    
    camera.camera.target    = (Vector2){ 0.0f, 0.0f };
    camera.camera.offset    = (Vector2){ (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    camera.camera.rotation  = 0.0f;
    camera.camera.zoom      = 1.0f;
    
    camera.followSpeed      = 5.0f;
    camera.zoom             = 2.0f;
    camera.zoomTarget       = 2.0f;
    camera.offset           = (Vector2){ (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    camera.smoothFollow     = true;
    camera.boundsMin        = (Vector2){ 0.0f, 0.0f };
    camera.boundsMax        = (Vector2){ 1000.0f, 1000.0f };
    
    return camera;
}

void UpdateCamera2D(GameCamera *camera, const Player *player, const Tilemap *map, const float frameTime)
{
    if (!camera || !player || !map) return;

    camera->camera.offset = (Vector2){ (float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f };

    const float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        camera->zoomTarget += wheel * 0.25f;
        camera->zoomTarget = Clamp(camera->zoomTarget, 0.5f, 4.0f);
    }

    const float zoomLerpFactor = 1.0f - expf(-8.0f * frameTime);
    camera->zoom = Lerp(camera->zoom, camera->zoomTarget, zoomLerpFactor);
    camera->camera.zoom = camera->zoom;

    const Vector2 targetPos = {
        player->movement.position.x + 16,
        player->movement.position.y + 16
    };

    if (camera->smoothFollow) {
        const float posLerpFactor = 1.0f - expf(-camera->followSpeed * frameTime);
        camera->camera.target = Vector2Lerp(camera->camera.target, targetPos, posLerpFactor);
    } else {
        camera->camera.target = targetPos;
    }

    const Vector2 viewSize = {
        (float)GetScreenWidth() / camera->camera.zoom,
        (float)GetScreenHeight() / camera->camera.zoom
    };

    const float mapW = (float)(map->width * map->tileWidth);
    const float mapH = (float)(map->height * map->tileHeight);

    if (viewSize.x >= mapW) camera->camera.target.x = mapW / 2.0f;
    else camera->camera.target.x = Clamp(camera->camera.target.x, viewSize.x / 2.0f, mapW - (viewSize.x / 2.0f));

    if (viewSize.y >= mapH) camera->camera.target.y = mapH / 2.0f;
    else camera->camera.target.y = Clamp(camera->camera.target.y, viewSize.y / 2.0f, mapH - (viewSize.y / 2.0f));
}

