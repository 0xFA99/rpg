#include <stdio.h>
#include <stdlib.h>

#include "tilemap.h"
#include "item.h"
#include "game.h"


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
