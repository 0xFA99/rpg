#ifndef ITEM_H
#define ITEM_H

#include "raylib.h"
#include "game_types.h"

typedef struct {
    Texture2D   texture;
    EquipSlot   slot;
    char        label[64];
} Item;

Item    ItemLoad(const char *binPath, EquipSlot slot, const char *label);
void    ItemUnload(Item *item);

#endif