#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#include "collision.h"
#include "tilemap.h"
#include "game_types.h"
#include "item.h"

typedef struct {
    Texture2D head;
    Texture2D hair;
    Texture2D subHair;
    Texture2D eyes;
    Texture2D mouth;
    Texture2D body;
    Texture2D shirt;
    Texture2D pant;
    Texture2D panty;
} PlayerPortrait;

typedef struct {
    Texture2D   headTexture;
    Texture2D   bodyTexture;
    Direction   direction;
    Vector2     frameSize;
    Action      action;
} PlayerGraphics;

typedef struct {
    Item        equipped[MAX_SLOTS];
    uint16_t    equippedFlags;
} PlayerEquipment;

typedef struct {
    float       frameTimer;
    int         currentFrame;
    int         frameDirection;
} PlayerAnimation;

typedef struct {
    Vector2     position;
    Vector2     tilePosition;
    Vector2     targetTilePosition;
    float       moveDuration;
    float       movementSpeed;
    float       moveTimer;
    float       holdTimer;
    float       holdDelay;
    bool        isHoldingKey;
    bool        justTurned;
    bool        isMoving;
    bool        justTriggeredEvent;
} PlayerMovement;

typedef struct Player {
    PlayerGraphics   graphics;
    PlayerEquipment  equipment;
    PlayerAnimation  animation;
    PlayerMovement   movement;
    PlayerPortrait   portrait;
} Player;


Player      *InitPlayer(void);
void         UpdatePlayer(Player *player, float frameTime, const Collision *collision, const Tilemap *tilemap);
void         DrawPlayer(const Player *player);
void         UnloadPlayer(Player *player);

void         PlayerEquipItem(Player *p, const Item *item);
void         PlayerUnequipItem(Player *p, EquipSlot slot);
bool         PlayerIsEquipped(const Player *p, EquipSlot slot);
const Item  *PlayerGetEquippedItem(const Player *p, EquipSlot slot);

bool         TryTriggerTileEvent(const Player *player, const Tilemap *tilemap);
int          GetSpriteRow(const Player *player);


#endif