#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "player.h"


#define DEFAULT_TILEMAP_ID  1
#define PORTRAIT_WIDTH      295
#define PORTRAIT_HEIGHT     370

static void GameplayLoadMap(ScreenGameplayData *gd, int mapId);
static void GameplayInitPlayer(ScreenGameplayData *gd);
static void UpdateTilemapTexture(ScreenGameplayData *gd);
static void UpdatePortraitTexture(ScreenGameplayData *gd);

// TODO: Finish UpdateTexture2D
static void UpdatePortraitTexture(ScreenGameplayData *gd)
{
    assert (gd != NULL);

    // if (gd->portraitInitialized) {
    //     UnloadRenderTexture(gd->portrait);
    // }

    gd->portrait = LoadRenderTexture(PORTRAIT_WIDTH, PORTRAIT_HEIGHT);
    BeginTextureMode(gd->portrait);
        ClearBackground(BLANK);

        const Player *player = gd->player;

        const Texture *bodyTexture = &player->portrait.body;
        DrawTextureRec(player->portrait.body,
            (Rectangle){ 0.0f, 0.0f, (float)bodyTexture->width, (float)bodyTexture->height },
            (Vector2){ 0.0f, 88.0f },
            WHITE);

        const Texture *headTexture = &player->portrait.head;
        DrawTextureRec(player->portrait.head,
            (Rectangle){ 0.0f, 0.0f, (float)headTexture->width, (float)headTexture->height },
            (Vector2){ 95.0f, 1.0f },
            WHITE);

        const Texture *hairTexture = &player->portrait.hair;
        DrawTextureRec(player->portrait.hair,
            (Rectangle){ 0.0f, 0.0f, (float)hairTexture->width, (float)hairTexture->height},
            (Vector2){ 95.0f, 1.0f },
            WHITE);

        const Texture *eyesTexture = &player->portrait.eyes;
        DrawTextureRec(player->portrait.eyes,
            (Rectangle){ 0.0f, 0.0f, (float)eyesTexture->width , (float)eyesTexture->height },
            (Vector2){ 103.0f, 22.0f },
            WHITE);

        const Texture *mouthTexture = &player->portrait.mouth;
        DrawTextureRec(player->portrait.mouth,
            (Rectangle){ 0.0f, 0.0f, (float)mouthTexture->width, (float)mouthTexture->height },
            (Vector2){ 121.0f, 86.0f },
            WHITE);

        const Texture *pantyTexture = &player->portrait.panty;
        DrawTextureRec(player->portrait.panty,
            (Rectangle){ 0.0f, 0.0f, (float)pantyTexture->width, (float)pantyTexture->height },
            (Vector2){ 68.0f, 239.0f },
            WHITE);
    EndTextureMode();

    gd->portraitInitialized = true;
}

static void UpdateTilemapTexture(ScreenGameplayData *gd)
{
    gd->canvas = LoadRenderTexture(
        gd->tilemap->width  * gd->tilemap->tileWidth,
        gd->tilemap->height * gd->tilemap->tileHeight);

    BeginTextureMode(gd->canvas);
    ClearBackground(BLANK);
    DrawTilemap(gd->tilemap);
    EndTextureMode();
}

static void GameplayLoadMap(ScreenGameplayData *gd, const int mapId)
{
    if (gd->canvasInitialized) {
        UnloadRenderTexture(gd->canvas);
        gd->canvasInitialized = false;
    }
    if (gd->collision) {
        DestroyCollision(gd->collision);
        free(gd->collision);
        gd->collision = NULL;
    }
    if (gd->tilemap) {
        UnloadTilemap(gd->tilemap);
        gd->tilemap = NULL;
    }

    gd->tilemap   = LoadTilemapById(mapId);
    gd->collision = InitCollisionAllLayers(gd->tilemap);

    UpdateTilemapTexture(gd);
    gd->canvasInitialized = true;
}

static void GameplayInitPlayer(ScreenGameplayData *gd)
{
    assert(gd && gd->tilemap);

    Player *newPlayer = InitPlayer();

    const float spawnX = (float)gd->tilemap->spawnPointX;
    const float spawnY = (float)gd->tilemap->spawnPointY;

    newPlayer->movement.tilePosition       = (Vector2){ spawnX, spawnY };
    newPlayer->movement.targetTilePosition = newPlayer->movement.tilePosition;
    newPlayer->movement.position           = (Vector2){
        spawnX * (float)gd->tilemap->tileWidth,
        spawnY * (float)gd->tilemap->tileHeight
    };

    gd->player = newPlayer;
}

void ScreenGameplayInit(Screen *s)
{
    assert(s != NULL);

    ScreenGameplayData *gd = calloc(1, sizeof(ScreenGameplayData));
    assert(gd != NULL);

    GameplayLoadMap(gd, DEFAULT_TILEMAP_ID);
    GameplayInitPlayer(gd);
    gd->camera = InitGameCamera(GetScreenWidth(), GetScreenHeight());

    gd->transAlpha = 0.0f;
    gd->pendingMapId = 0;
    gd->isTransitioning = false;
    gd->transitionFadeOut = true;
    gd->transitionSpeed = 2.4f;

    // TODO: Make Items dynamic after creating inventory system
    const Item hair   = ItemLoad("assets/characters/ivy/ivy_hair_basic.bin",   SLOT_HAIR,   "hair");
    const Item shirt  = ItemLoad("assets/characters/ivy/ivy_shirt_basic.bin",  SLOT_TOP,    "shirt");
    const Item pants  = ItemLoad("assets/characters/ivy/ivy_bottom_basic.bin", SLOT_BOTTOM, "pants");

    PlayerEquipItem(gd->player, &hair);
    PlayerEquipItem(gd->player, &shirt);
    PlayerEquipItem(gd->player, &pants);

    gd->portraitInitialized = false;

    s->data = gd;
}

void ScreenGameplayUpdate(GameState *gs, bool *running)
{
    ScreenGameplayData *gd = gs->currentScreen.data;
    if (!gd || !gd->tilemap || !gd->player) return;

    Player *player = gd->player;

    if (gd->portraitInitialized == false) UpdatePortraitTexture(gs->currentScreen.data);

    if (gd->isTransitioning) {
        if (gd->transitionFadeOut)
        {
            gd->transAlpha += gd->transitionSpeed * gs->frameTime;

            if (gd->transAlpha >= 1.0f) {
                gd->transAlpha = 1.0f;

                GameplayLoadMap(gd, gd->pendingMapId);

                const int spawnX = gd->tilemap->spawnPointX;
                const int spawnY = gd->tilemap->spawnPointY;

                const float worldX = (float)spawnX * (float)gd->tilemap->tileWidth;
                const float worldY = (float)spawnY * (float)gd->tilemap->tileHeight;

                player->movement.tilePosition       = (Vector2){ (float)spawnX, (float)spawnY };
                player->movement.targetTilePosition = player->movement.tilePosition;
                player->movement.position           = (Vector2){ worldX, worldY };
                player->movement.isMoving           = false;
                player->movement.moveTimer          = 0.0f;
                player->movement.holdTimer          = 0.0f;
                player->movement.justTurned         = false;
                player->movement.isHoldingKey       = false;
                player->graphics.action             = ACTION_IDLE;

                UpdateCamera2D(&gd->camera, player, gd->tilemap, 0.0f);

                gd->transitionFadeOut = false;
            }
        } else {
            gd->transAlpha -= gd->transitionSpeed * gs->frameTime;

            if (gd->transAlpha <= 0.0f)
            {
                gd->transAlpha        = 0.0f;
                gd->isTransitioning   = false;
                gd->transitionFadeOut = true;
                gd->pendingMapId      = 0;
            }
        }
        return;
    }

    UpdateCamera2D(&gd->camera, player, gd->tilemap, gs->frameTime);
    UpdatePlayer(player, gs->frameTime, gd->collision, gd->tilemap);

    // Map transition
    if (player->movement.justTriggeredEvent && gd->tilemap->eventGotoMapId > 0) {
        player->movement.justTriggeredEvent = false;

        gd->pendingMapId = gd->tilemap->eventGotoMapId;
        gd->isTransitioning = true;
        gd->transitionFadeOut = true;
        gd->transAlpha = 0.0f;
    }
}

void ScreenGameplayDraw(const Screen *s)
{
    const ScreenGameplayData *gd = s->data;
    if (!gd || !gd->tilemap) return;

    BeginMode2D(gd->camera.camera);
        if (gd->canvasInitialized && gd->canvas.texture.id != 0)
        {
            DrawTexturePro(
                gd->canvas.texture,
                (Rectangle){ 0, 0, (float)gd->canvas.texture.width, -(float)gd->canvas.texture.height },
                (Rectangle){ 0, 0, (float)gd->canvas.texture.width,  (float)gd->canvas.texture.height },
                (Vector2){0, 0}, 0.0f, WHITE);
        }

        DrawTilemap(gd->tilemap);
        DrawPlayer(gd->player);
    EndMode2D();

    if (gd->portraitInitialized && gd->portrait.texture.id != 0)
    {
        const float texWidth = (float)gd->portrait.texture.width;
        const float texHeight = (float)gd->portrait.texture.height;

        DrawTexturePro(gd->portrait.texture,
            (Rectangle){ 0.0f, 0.0f, texWidth, -texHeight },
            (Rectangle){ (float)GetScreenWidth() - texWidth, (float)GetScreenHeight() - texHeight, texWidth, texHeight },
            (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }

    if (gd->isTransitioning || gd->transAlpha > 0.0f) {
        const unsigned char alpha = (unsigned char)(gd->transAlpha * 255.0f);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, alpha });
    }
}

void ScreenGameplayUnload(const Screen *s)
{
    if (!s || !s->data) return;
    ScreenGameplayData *gd = s->data;

    if (gd->player)
    {
        UnloadTexture(gd->player->graphics.headTexture);
        UnloadTexture(gd->player->graphics.bodyTexture);
        for (int i = 0; i < MAX_SLOTS; i++) {
            PlayerUnequipItem(gd->player, (EquipSlot)i);
        }
        free(gd->player);
        gd->player = NULL;
    }

    if (gd->canvasInitialized) {
        UnloadRenderTexture(gd->canvas);
        gd->canvasInitialized = false;
    }
    if (gd->portraitInitialized) {
        UnloadRenderTexture(gd->portrait);
        gd->portraitInitialized = false;
    }
    if (gd->collision) {
        DestroyCollision(gd->collision);
        free(gd->collision);
        gd->collision = NULL;
    }
    if (gd->tilemap) {
        UnloadTilemap(gd->tilemap);
        gd->tilemap = NULL;
    }

    free(gd);
}
