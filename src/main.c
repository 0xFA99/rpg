#include "raylib.h"

#include "game.h"
#include "player.h"
#include "tilemap.h"
#include "camera.h"
#include "collusion.h"

int main(void) {


    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "RPG Game");

    Player player = InitPlayer();

    const Item shirt  = LoadItem("../assets/characters/ivy/ivy_shirt_basic.png", SLOT_TOP, "Basic Shirt");
    const Item pant   = LoadItem("../assets/characters/ivy/ivy_bottom_basic.png", SLOT_BOTTOM, "Basic Pant");
    const Item hair   = LoadItem("../assets/characters/ivy/ivy_hair_basic.png", SLOT_HAIR, "Basic Hair");

    PlayerEquipItem(&player, pant);
    PlayerEquipItem(&player, shirt);
    PlayerEquipItem(&player, hair);


    Tilemap map = LoadTilemap("../assets/tilemaps/gynecology.json");
    LoadAutoTile(&map, "../assets/tilesets/TileA4.png", (Vector2){256.0f, 320.0f});

    Collusion collusion = CreateCollusion(&map);
    SetupCollusion(&collusion, &map);

    // const RenderTexture2D bakedMap = LoadRenderTexture(map.width * TILE_SIZE, map.height * TILE_SIZE);
    // BeginTextureMode(bakedMap);
    // ClearBackground(BLANK);
    // EndTextureMode();

    RenderTexture2D bakedMap = GenerateTilemapRenderTexture(&map);
    
    // Setup camera bounds based on tilemap
    GameCamera camera = InitGameCamera(screenWidth, screenHeight);
    SetCameraBounds(&camera, &map);

    // TEST TEXTURE
    // Texture2D testTexture = LoadTexture("../assets/tilesets/TileA4.png");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        float frameTime = GetFrameTime();

        UpdatePlayer(&player, frameTime, &collusion);
        UpdateCamera2D(&camera, &player, &map);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera.camera);
        
        DrawTexturePro(
            bakedMap.texture,
            (Rectangle){ 0, 0, (float)bakedMap.texture.width, (float)-bakedMap.texture.height }, // Negatif height buat flip Y
            (Rectangle){ 0, 0, (float)bakedMap.texture.width, (float)bakedMap.texture.height },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );

        DrawPlayer(&player);

        for (int i = 0; i < collusion.rectCount; i++) {
            DrawRectangleLinesEx(collusion.rect[i], 1, RED);
        }
        EndMode2D();

        EndDrawing();
    }

    UnloadRenderTexture(bakedMap);
    // UnloadTexture(tileset);

    UnloadTilemap(&map);
    UnloadItem(shirt);
    UnloadItem(pant);
    UnloadItem(hair);


    CloseWindow();
    return 0;
}