#include "game.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "item.h"
#include "tilemap.h"
#include "collision.h"
#include "player.h"
#include "camera.h"


#define FONT_PATH           "assets/fonts/DenkOne-Regular.ttf"
#define CURSOR_PATH         "assets/cursors/cursorW.bin"
#define DEFAULT_TILEMAP_ID  1


Texture2D LoadTextureFromBin(const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open binary file: %s", fileName);
        return (Texture2D){0};
    }

    unsigned int size = 0;
    if (fread(&size, sizeof(unsigned int), 1, file) != 1) {
        fclose(file);
        return (Texture2D){0};
    }

    unsigned char *data = malloc(size);
    if (!data) { fclose(file); return (Texture2D){0}; }

    fread(data, 1, size, file);
    fclose(file);

    const Image     img = LoadImageFromMemory(".png", data, (int)size);
    const Texture2D tex = LoadTextureFromImage(img);

    UnloadImage(img);
    free(data);

    return tex;
}

static void ReloadCanvas(RenderTexture2D *canvas, const Tilemap *tilemap)
{
    *canvas = LoadRenderTexture(
        tilemap->width  * tilemap->tileWidth,
        tilemap->height * tilemap->tileHeight);

    BeginTextureMode(*canvas);
    ClearBackground(BLANK);
    DrawTilemap(tilemap);
    EndTextureMode();
}

void ScreenTitleInit(Screen *s)
{
    ScreenTitleData *sd = malloc(sizeof(ScreenTitleData));
    assert(sd != NULL);

    sd->cursor        = LoadTextureFromBin(CURSOR_PATH);
    sd->font          = LoadFontEx(FONT_PATH, 32, NULL, 0);
    sd->selectedIndex = 0;
    sd->menuCount     = 4;
    sd->menuItems     = malloc(sizeof(char *) * sd->menuCount);

    sd->menuItems[0] = "NEW GAME";
    sd->menuItems[1] = "CONTINUE";
    sd->menuItems[2] = "OPTIONS";
    sd->menuItems[3] = "EXIT";

    s->data = sd;
}

void ScreenTitleUpdate(GameState *gs, bool *running)
{
    Screen          *s  = &gs->currentScreen;
    ScreenTitleData *sd = s->data;

    if (IsKeyPressed(KEY_DOWN)) sd->selectedIndex = (sd->selectedIndex + 1) % sd->menuCount;
    if (IsKeyPressed(KEY_UP))   sd->selectedIndex = (sd->selectedIndex - 1 + sd->menuCount) % sd->menuCount;

    if (IsKeyPressed(KEY_ENTER))
    {
        switch (sd->selectedIndex)
        {
            case 0: /* TODO: NEW GAME */ break;
            case 1: {
                s->type          = SCREEN_GAMEPLAY;
                s->screenUpdated = true;
            } break;
            case 2: {
                s->type          = SCREEN_OPTIONS;
                s->screenUpdated = true;
            } break;
            case 3: *running = false; break;
            default: break;
        }
    }
}

void ScreenTitleDraw(const Screen *s)
{
    const ScreenTitleData *sd = s->data;

    const float startY  = (float)GetScreenHeight() / 2.0f;
    const float spacing = (float)sd->font.baseSize * 1.2f;

    for (int i = 0; i < sd->menuCount; i++)
    {
        const Color   color = i == sd->selectedIndex ? WHITE : GRAY;
        const Vector2 pos   = { 60.0f, startY + (float)i * spacing };

        DrawTextEx(sd->font, sd->menuItems[i], pos, (float)sd->font.baseSize, 2, color);

        if (i == sd->selectedIndex) {
            DrawTexture(sd->cursor, 20, (int)pos.y + sd->font.baseSize / 4, WHITE);
        }
    }
}

void ScreenTitleUnload(const Screen *s)
{
    if (!s || !s->data) return;
    ScreenTitleData *sd = s->data;

    UnloadTexture(sd->cursor);
    UnloadFont(sd->font);
    free(sd->menuItems);
    free(sd);
}

static void GameplayLoadMap(ScreenGameplayData *gd, int mapId)
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

    ReloadCanvas(&gd->canvas, gd->tilemap);
    gd->canvasInitialized = true;
}

static void GameplayInitPlayer(ScreenGameplayData *gd)
{
    assert(gd && gd->tilemap);

    Player *player = calloc(1, sizeof(Player));
    assert(player != NULL);

    player->graphics.action      = ACTION_IDLE;
    player->graphics.direction   = DIRECTION_FRONT;
    player->graphics.frameSize   = (Vector2){ 64.0f, 64.0f };
    player->graphics.headTexture = LoadTextureFromBin("assets/characters/ivy/ivy_head_good.bin");
    player->graphics.bodyTexture = LoadTextureFromBin("assets/characters/ivy/ivy_body_base.bin");

    player->animation.currentFrame   = 0;
    player->animation.frameTimer     = 0.0f;
    player->animation.frameDirection = 1;

    const float spawnX = (float)gd->tilemap->spawnPointX;
    const float spawnY = (float)gd->tilemap->spawnPointY;

    player->movement.tilePosition       = (Vector2){ spawnX, spawnY };
    player->movement.targetTilePosition = player->movement.tilePosition;
    player->movement.position           = (Vector2){
        spawnX * (float)gd->tilemap->tileWidth,
        spawnY * (float)gd->tilemap->tileHeight
    };
    player->movement.moveDuration = 0.42f;
    player->movement.holdDelay    = 0.15f;

    gd->player = player;
}

void ScreenGameplayInit(Screen *s)
{
    assert(s != NULL);

    ScreenGameplayData *gd = calloc(1, sizeof(ScreenGameplayData));
    assert(gd != NULL);

    GameplayLoadMap(gd, DEFAULT_TILEMAP_ID);
    GameplayInitPlayer(gd);
    gd->camera = InitGameCamera(GetScreenWidth(), GetScreenHeight());

    // TODO: Make Items dynamic after creating inventory system
    const Item hair   = ItemLoad("assets/characters/ivy/ivy_hair_basic.bin",   SLOT_HAIR,   "hair");
    const Item shirt  = ItemLoad("assets/characters/ivy/ivy_shirt_basic.bin",  SLOT_TOP,    "shirt");
    const Item pants  = ItemLoad("assets/characters/ivy/ivy_bottom_basic.bin", SLOT_BOTTOM, "pants");

    PlayerEquipItem(gd->player, &hair);
    PlayerEquipItem(gd->player, &shirt);
    PlayerEquipItem(gd->player, &pants);

    s->data = gd;
}

void ScreenGameplayUpdate(GameState *gs, bool *running)
{
    ScreenGameplayData *gd = gs->currentScreen.data;
    if (!gd || !gd->tilemap || !gd->player) return;

    Player *player = gd->player;

    UpdateCamera2D(&gd->camera, player, gd->tilemap, gs->frameTime);
    UpdatePlayer(player, gs->frameTime, gd->collision, gd->tilemap);

    // Map transition
    if (player->movement.justTriggeredEvent && gd->tilemap->eventGotoMapId > 0) {
        player->movement.justTriggeredEvent = false;

        const int newMapId = gd->tilemap->eventGotoMapId;

        GameplayLoadMap(gd, newMapId);

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

void UpdateScreen(Screen *s)
{
    if (s->data && s->Unload) {
        s->Unload(s);
        s->data = NULL;
    }

    switch (s->type)
    {
        case SCREEN_TITLE: {
            s->Init   = ScreenTitleInit;
            s->Update = ScreenTitleUpdate;
            s->Draw   = ScreenTitleDraw;
            s->Unload = ScreenTitleUnload;
        } break;

        case SCREEN_GAMEPLAY: {
            s->Init   = ScreenGameplayInit;
            s->Update = ScreenGameplayUpdate;
            s->Draw   = ScreenGameplayDraw;
            s->Unload = ScreenGameplayUnload;
        } break;

        case SCREEN_OPTIONS: {
            s->Init   = ScreenOptionsInit;
            s->Update = ScreenOptionsUpdate;
            s->Draw   = ScreenOptionsDraw;
            s->Unload = ScreenOptionsUnload;
        } break;

        default: break;
    }

    s->Init(s);
    s->screenUpdated = false;
}

void ScreenOptionsInit(Screen *s)
{
    ScreenOptionsData *od = malloc(sizeof(ScreenOptionsData));
    assert(od != NULL);

    od->cursor        = LoadTextureFromBin(CURSOR_PATH);
    od->font          = LoadFontEx(FONT_PATH, 32, NULL, 0);
    od->selectedIndex = 0;
    od->count         = 4;
    od->volume        = 0.5f;

    s->data = od;
}

void ScreenOptionsUpdate(GameState *gs, bool *running)
{
    Screen *s = &gs->currentScreen;
    ScreenOptionsData *od = s->data;

    if (IsKeyPressed(KEY_DOWN)) {
        od->selectedIndex = (od->selectedIndex + 1) % od->count;
    }
    if (IsKeyPressed(KEY_UP)) {
        od->selectedIndex = (od->selectedIndex - 1 + od->count) % od->count;
    }

    switch (od->selectedIndex)
    {
        case 1: {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                od->volume -= IsKeyPressed(KEY_LEFT) ? 0.1f : -0.1f;
                if (od->volume < 0.0f) od->volume = 0.0f;
                if (od->volume > 1.0f) od->volume = 1.0f;
            }
        } break;

        case 3: {
            if (IsKeyPressed(KEY_ENTER)) {
                s->type             = SCREEN_TITLE;
                s->screenUpdated    = true;
            }
        } break;

        default: break;
    }
}

void ScreenOptionsDraw(const Screen *s)
{
    const ScreenOptionsData *od = s->data;

    const char *menuItems[] = {
        "FULLSCREEN",
        "VOLUME",
        "KEYBINDINGS",
        "BACK"
    };

    const float startY = (float)GetScreenHeight() / 2.0f;
    const float spacing = (float)od->font.baseSize * 1.2f;

    DrawTextEx(od->font, "OPTIONS", (Vector2){ 60.0f, startY - spacing * 2.0f }, (float)od->font.baseSize, 2, WHITE);

    for (int i = 0; i < od->count; i++)
    {
        const Color color = i == od->selectedIndex ? WHITE : GRAY;
        const Vector2 pos = { 60.0f, startY + (float)i * spacing };

        if (i == 1) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "VOLUME: %.0f%%", od->volume * 100.0f);
            DrawTextEx(od->font, buffer, pos, (float)od->font.baseSize, 2, color);
        } else {
            DrawTextEx(od->font, menuItems[i], pos, (float)od->font.baseSize, 2, color);
        }

        if (i == od->selectedIndex) {
            DrawTexture(od->cursor, 20, (int)pos.y + od->font.baseSize / 4, WHITE);
        }
    }
}

void ScreenOptionsUnload(const Screen *s)
{
    if (!s || !s->data) return;
    ScreenOptionsData *od = s->data;

    UnloadTexture(od->cursor);
    UnloadFont(od->font);
    free(od);
}
