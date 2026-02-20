
#include "game.h"

#include <stdio.h>
#include <stdlib.h>

Item LoadItem(const char *filename, const EquipSlot slot, const char *label)
{
    Item item = {0};
    item.texture = LoadTextureFromBin(filename);
    item.slot = slot;
    item.label = label;

    return item;
}



Texture2D LoadTextureFromBin(const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open binary file: %s", fileName);
        return (Texture2D){ 0 };
    }

    int size = 0;
    if (fread(&size, sizeof(unsigned int), 1, file) != 1) {
        fclose(file);
        return (Texture2D){ 0 };
    }

    unsigned char *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    const Image img = LoadImageFromMemory(".png", data, size);
    const Texture2D tex = LoadTextureFromImage(img);

    UnloadImage(img);
    free(data);

    return tex;
}