#include <assert.h>
#include <stdlib.h>

#include "game.h"


#define FONT_PATH           "assets/fonts/DenkOne-Regular.ttf"
#define CURSOR_PATH         "assets/cursors/cursorW.bin"


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
