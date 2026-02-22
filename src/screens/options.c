#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"


#define FONT_PATH           "assets/fonts/DenkOne-Regular.ttf"
#define CURSOR_PATH         "assets/cursors/cursorW.bin"


void ScreenOptionsInit(Screen *s)
{
    ScreenOptionsData *od = malloc(sizeof(ScreenOptionsData));
    assert(od != NULL);

    od->cursor        = LoadTextureFromBin(CURSOR_PATH);
    od->font          = LoadFontEx(FONT_PATH, 32, NULL, 0);
    od->selectedIndex = 0;
    od->count         = 5;
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
        // WINDOW MODE
        case 0: break;

        // VOLUME SETTING
        case 1: {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                od->volume -= IsKeyPressed(KEY_LEFT) ? 0.1f : -0.1f;
                if (od->volume < 0.0f) od->volume = 0.0f;
                if (od->volume > 1.0f) od->volume = 1.0f;
            }
        } break;

        // BACK TO TITLE SCREEN
        case 3: {
            if (IsKeyPressed(KEY_ENTER)) {
                s->type             = SCREEN_TITLE;
                s->screenUpdated    = true;
            }
        } break;

        // SAVE
        // case 4: break;

        default: break;
    }
}

void ScreenOptionsDraw(const Screen *s)
{
    const ScreenOptionsData *od = s->data;

    const char *menuItems[] = {
        "WINDOW MODE",
        "VOLUME",
        "KEYBINDINGS",
        "BACK",
        "SAVE"
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
