#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#include "collision.h"
#include "tilemap.h"
#include "game_types.h"
#include "item.h"

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
} Player;


Player       InitPlayer(void);
Player       InitPlayerAt(int spawnX, int spawnY);
void         UpdatePlayer(Player *player, float frameTime, const Collision *collision, const Tilemap *tilemap);
void         DrawPlayer(const Player *player);

void         PlayerEquipItem(Player *p, const Item *item);
void         PlayerUnequipItem(Player *p, EquipSlot slot);
bool         PlayerIsEquipped(const Player *p, EquipSlot slot);
const Item  *PlayerGetEquippedItem(const Player *p, EquipSlot slot);

bool         TryTriggerTileEvent(const Player *player, const Tilemap *tilemap);
int          GetSpriteRow(const Player *player);


#endif