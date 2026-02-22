#include <stdio.h>

#include "raylib.h"
#include "game.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "RPG Game");

    // Item hair = LoadItem("assets/characters/ivy/ivy_hair_basic.bin", SLOT_HAIR, "hair");
    // Item shirt = LoadItem("assets/characters/ivy/ivy_shirt_basic.bin", SLOT_TOP, "shirt");
    // Item bottom = LoadItem("assets/characters/ivy/ivy_bottom_basic.bin", SLOT_BOTTOM, "bottom");

    // GameState game = {0};
    // GameInit(&game);

    // Player player = InitPlayerAt(1, 1);
    // PlayerEquipItem(&player, hair);
    // PlayerEquipItem(&player, shirt);
    // PlayerEquipItem(&player, bottom);

    // Vector2 spawnPos = {0};
    // GameLoadMap(1, &game, &spawnPos);
    // player.movement.tilePosition = spawnPos;
    // player.movement.position = (Vector2){ spawnPos.x * TILE_SIZE, spawnPos.y * TILE_SIZE };
    // player.movement.targetTilePosition = spawnPos;
    // player.movement.isMoving = false;

    // GameCamera camera = InitGameCamera(screenWidth, screenHeight);

    // ScreenManager sm = { SCREEN_TITLE, .data.title = NULL };
    // ChangeToScreen(&sm, SCREEN_TITLE);

    GameState sm = {
        .gameRunning    = true,
        .currentScreen  = (Screen) {
            .type           = SCREEN_TITLE,
            .data           = NULL,
            .Init           = ScreenTitleInit,
            .Update         = ScreenTitleUpdate,
            .Draw           = ScreenTitleDraw,
            .Unload         = ScreenTitleUnload,
            .screenUpdated  = false
        },
        .frameTime = 0.0f
    };

    sm.currentScreen.Init(&sm.currentScreen);

    SetTargetFPS(60);

    while (!WindowShouldClose() && sm.gameRunning) {
        sm.frameTime = GetFrameTime();

        if (sm.currentScreen.screenUpdated) {
            UpdateScreen(&sm.currentScreen);
        }

        sm.currentScreen.Update(&sm, &sm.gameRunning);

        BeginDrawing();
            ClearBackground(BLACK);
            sm.currentScreen.Draw(&sm.currentScreen);
        EndDrawing();
    }

    // ChangeToScreen(&sm, -1);
    // GameUnload(&game);

    sm.currentScreen.Unload(&sm.currentScreen);

    CloseWindow();
    return 0;
}