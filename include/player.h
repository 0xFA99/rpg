#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

#include <stdint.h>
#include "collusion.h"

typedef struct {
    Texture2D   headTexture;
    Texture2D   bodyTexture;
    Direction   direction;
    Vector2     frameSize;
    Action      action;
} PlayerGraphics;

typedef struct {
    Texture2D   equippedTextures[MAX_SLOTS];
    uint16_t    equippedFlags;
} PlayerEquipment;

typedef struct {
    float       frameTimer;
    int         currentFrame;
    int         maxFrames;
    int         frameDirection;
} PlayerAnimation;

typedef struct {
    Vector2     position;
    Vector2     velocity;
    Vector2     tilePosition;
    Vector2     targetTilePosition;
    Vector2     actualPosition;
    float       moveDuration;
    float       movementSpeed;
    float       moveTimer;
    float       holdTimer;
    float       holdDelay;
    bool        isHoldingKey;
    bool        justTurned;
    bool        isMoving;
} PlayerMovement;

typedef struct {
    PlayerGraphics   graphics;
    PlayerEquipment  equipment;
    PlayerAnimation  animation;
    PlayerMovement   movement;
} Player;

Player InitPlayer(void);
int GetSpriteRow(const Player *player);
void UpdatePlayer(Player *player, float frameTime, const Collusion *collusion, int count);
void DrawPlayer(const Player *player);

void PlayerEquipItem(Player *p, Item newItem);
void PlayerUnequipItem(Player *p, EquipSlot slot);

bool PlayerIsEquipItem(const Player *p, EquipSlot slot);

#endif