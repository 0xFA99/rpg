#include <string.h>

#include "item.h"

extern Texture2D LoadTextureFromBin(const char *fileName);

Item ItemLoad(const char *binPath, const EquipSlot slot, const char *label)
{
    Item item       = {0};
    item.texture    = LoadTextureFromBin(binPath);
    item.slot       = slot;

    if (label)
    {
        strncpy(item.label, label, sizeof(item.label) - 1);
        item.label[sizeof(item.label) - 1] = '\0';
    }

    return item;
}

void ItemUnload(Item *item)
{
    if (!item) return;
    if (item->texture.id != 0) {
        UnloadTexture(item->texture);
        item->texture = (Texture2D){0};
    }
}

bool IsItemValid(Item *item)
{
    if (!item) return false;

    return IsTextureValid(item->texture);
}
