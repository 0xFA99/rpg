#ifndef GAME_H
#define GAME_H

#define TILE_SIZE        32

#include "camera.h"
#include "player.h"

typedef enum {
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_OPTIONS,
    SCREEN_EXIT
} ScreenType;

typedef struct Screen Screen;
typedef struct GameState GameState;

typedef struct Screen {
    ScreenType type;
    void *data;
    void (*Init)(Screen *s);
    void (*Update)(GameState *s, bool *running);
    void (*Draw)(const Screen *s);
    void (*Unload)(const Screen *s);
    bool screenUpdated;
} Screen;

typedef struct {
    Texture2D   cursor;
    Font        font;
    int         selectedIndex;
    int         menuCount;
    char        **menuItems;
} ScreenTitleData;

typedef struct {
    GameCamera camera;
    Player *player;

    Tilemap *tilemap;
    Collision *collision;

    RenderTexture2D canvas;
    bool canvasInitialized;
    float transAlpha;
    int pendingMapId;
} ScreenGameplayData;

typedef struct {
    Texture2D cursor;
    Font       font;
    int        selectedIndex;
    int        count;
    float      volume;
} ScreenOptionsData;

struct GameState {
    bool gameRunning;
    Screen currentScreen;
    float frameTime;
};

Texture2D LoadTextureFromBin(const char *fileName);

void ScreenTitleInit(Screen *s);
void ScreenTitleUpdate(GameState *gs, bool *running);
void ScreenTitleDraw(const Screen *s);
void ScreenTitleUnload(const Screen *s);

void ScreenGameplayInit(Screen *s);
void ScreenGameplayUpdate(GameState *gs, bool *running);
void ScreenGameplayDraw(const Screen *s);
void ScreenGameplayUnload(const Screen *s);

void ScreenOptionsInit(Screen *s);
void ScreenOptionsUpdate(GameState *gs, bool *running);
void ScreenOptionsDraw(const Screen *s);
void ScreenOptionsUnload(const Screen *s);

void UpdateScreen(Screen *s);


#endif