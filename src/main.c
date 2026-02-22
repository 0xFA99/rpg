#include <stdio.h>

#include "raylib.h"
#include "game.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "RPG Game");
    SetExitKey(KEY_NULL);

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
        .frameTime = 0.0f,
    };

    sm.currentScreen.Init(&sm.currentScreen);

    SetTargetFPS(60);

    while (!WindowShouldClose() && sm.gameRunning) {

        if (IsWindowResized()) printf("RESIZE!!!!\n");

        sm.frameTime = GetFrameTime();

        sm.currentScreen.Update(&sm, &sm.gameRunning);

        if (sm.currentScreen.screenUpdated) {
            UpdateScreen(&sm.currentScreen);
        }

        BeginDrawing();
            ClearBackground(BLACK);
            sm.currentScreen.Draw(&sm.currentScreen);
        EndDrawing();
    }

    sm.currentScreen.Unload(&sm.currentScreen);

    CloseWindow();
    return 0;
}