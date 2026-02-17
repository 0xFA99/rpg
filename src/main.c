#include "camera.h"
#include "raylib.h"
#include "tilemap.h"


int main(void)
{
    const int screenWidth   = 800;
    const int screenHeight  = 600;

    InitWindow(screenWidth, screenHeight, "RPG Game");

    Item hair       = LoadItem("../assets/characters/ivy/ivy_hair_basic.bin", SLOT_HAIR, "hair");
    Item shirt      = LoadItem("../assets/characters/ivy/ivy_shirt_basic.bin", SLOT_TOP, "shirt");
    Item bottom     = LoadItem("../assets/characters/ivy/ivy_bottom_basic.bin", SLOT_BOTTOM, "bottom");
    // Item sword      = LoadItem("../assets/weapons/short_sword.bin", SLOT_HAND_MAIN, "sword");
    // Item lantern    = LoadItem("../assets/weapons/lantern.bin", SLOT_HAND_OFF, "lantern");

    Player player = InitPlayer();
    PlayerEquipItem(&player, hair);
    PlayerEquipItem(&player, shirt);
    PlayerEquipItem(&player, bottom);
    // PlayerEquipItem(&player, sword);
    // PlayerEquipItem(&player, lantern);

    GameCamera camera = InitGameCamera(screenWidth, screenHeight);

    Tilemap tilemap = LoadTilemapBinary("../assets/tilemaps/map.bin");

    Collusion coll1 = CreateCollusion(&tilemap, 0);
    SetupCollusion(&coll1, &tilemap, 0);

    Collusion coll2 = CreateCollusion(&tilemap, 1);
    SetupCollusion(&coll2, &tilemap, 1);

    Collusion colls[] = {coll1, coll2};

    const RenderTexture2D canvas = LoadRenderTexture(tilemap.width * tilemap.tileWidth, tilemap.height * tilemap.tileHeight);

    BeginTextureMode(canvas);
        ClearBackground(BLANK);
        DrawTilemap(&tilemap);
    EndTextureMode();

    SetTargetFPS(60);

    float frameTime;

    while (!WindowShouldClose()) {

        frameTime = GetFrameTime();

        UpdatePlayer(&player, frameTime, colls, 2);
        UpdateCamera2D(&camera, &player, &tilemap, frameTime);

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera.camera);

                DrawTexturePro(
                    canvas.texture,
                    (Rectangle){ 0, 0, (float)canvas.texture.width, -(float)canvas.texture.height },
                    (Rectangle){ 0, 0, (float)canvas.texture.width, (float)canvas.texture.height },
                    (Vector2){ 0, 0 },
                    0.0f,
                    WHITE
                );

                // [DEBUG] Collusion
                // for (int c = 0; c < 2; c++) {
                //     for (int i = 0; i < colls[c].rectCount; i++) {
                //         DrawRectangleLinesEx(colls[c].rect[i], 1, BLUE);
                //     }
                // }

                DrawPlayer(&player);

            EndMode2D();
        EndDrawing();
    }

    DestroyCollusion(&coll1);
    DestroyCollusion(&coll2);
    UnloadTilemap(&tilemap);
    UnloadRenderTexture(canvas);

    CloseWindow();
    return 0;
}