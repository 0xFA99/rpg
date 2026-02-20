#include "player.h"
#include "collusion.h"

Player InitPlayer(void) {
  Player player = {0};

  player.graphics.action = ACTION_IDLE;
  player.graphics.direction = DIRECTION_FRONT;
  player.graphics.frameSize = (Vector2){64.0f, 64.0f};
  player.graphics.headTexture =
      LoadTextureFromBin("assets/characters/ivy/ivy_head_good.bin");
  player.graphics.bodyTexture =
      LoadTextureFromBin("assets/characters/ivy/ivy_body_base.bin");

  player.equipment.equippedFlags = 0;

  player.animation.currentFrame = 0;
  player.animation.frameTimer = 0.0f;
  player.animation.frameDirection = 1;

  player.movement.tilePosition = (Vector2){9.0f, 4.0f};
  player.movement.targetTilePosition = player.movement.tilePosition;
  player.movement.position =
      (Vector2){player.movement.tilePosition.x * TILE_SIZE,
                player.movement.tilePosition.y * TILE_SIZE};
  player.movement.moveTimer = 0.0f;
  player.movement.moveDuration = 0.15f;
  player.movement.isMoving = false;
  player.movement.isHoldingKey = false;
  player.movement.movementSpeed = 1.0f;

  player.movement.holdDelay = 0.15f;
  player.movement.holdTimer = 0.0f;
  player.movement.justTurned = false;

  return player;
}

Player InitPlayerAt(int spawnX, int spawnY) {
  Player player = {0};

  player.graphics.action = ACTION_IDLE;
  player.graphics.direction = DIRECTION_FRONT;
  player.graphics.frameSize = (Vector2){64.0f, 64.0f};
  player.graphics.headTexture =
      LoadTextureFromBin("assets/characters/ivy/ivy_head_good.bin");
  player.graphics.bodyTexture =
      LoadTextureFromBin("assets/characters/ivy/ivy_body_base.bin");

  player.equipment.equippedFlags = 0;

  player.animation.currentFrame = 0;
  player.animation.frameTimer = 0.0f;
  player.animation.frameDirection = 1;

  player.movement.tilePosition = (Vector2){(float)spawnX, (float)spawnY};
  player.movement.targetTilePosition = player.movement.tilePosition;
  player.movement.position =
      (Vector2){(float)spawnX * TILE_SIZE,
                (float)spawnY * TILE_SIZE};
  player.movement.moveTimer = 0.0f;
  player.movement.moveDuration = 0.15f;
  player.movement.isMoving = false;
  player.movement.isHoldingKey = false;
  player.movement.movementSpeed = 1.0f;

  player.movement.holdDelay = 0.15f;
  player.movement.holdTimer = 0.0f;
  player.movement.justTurned = false;

  return player;
}

int GetSpriteRow(const Player *player) {
  int baseRow = 0;

  if (player->graphics.action == ACTION_WALK)
    baseRow = 0;
  else if (player->graphics.action == ACTION_RUN)
    baseRow = 4;

  switch (player->graphics.direction) {
  case DIRECTION_FRONT:
    return baseRow + 0;
  case DIRECTION_LEFT:
    return baseRow + 1;
  case DIRECTION_RIGHT:
    return baseRow + 2;
  case DIRECTION_BACK:
    return baseRow + 3;
  default:
    return baseRow;
  }
}

static void StartMoving(Player *player, const Vector2 inputDir,
                        const Direction nextDir, const bool isShiftDown) {
  player->graphics.direction = nextDir;
  player->movement.moveDuration = isShiftDown ? 0.25f : 0.42f;
  player->graphics.action = isShiftDown ? ACTION_RUN : ACTION_WALK;

  player->movement.targetTilePosition.x =
      player->movement.tilePosition.x + inputDir.x;
  player->movement.targetTilePosition.y =
      player->movement.tilePosition.y + inputDir.y;

  player->movement.isMoving = true;
  player->movement.moveTimer = 0.0f;
}

static void UpdateAnimation(Player *player, const float frameTime) {
  if (player->graphics.action == ACTION_IDLE) {
    player->animation.currentFrame = 1;
    player->animation.frameTimer = 0.0f;
  } else {
    const float animSpeed = player->movement.moveDuration / 3.0f;

    player->animation.frameTimer += frameTime;
    if (player->animation.frameTimer >= animSpeed) {
      player->animation.currentFrame += player->animation.frameDirection;

      if (player->animation.currentFrame >= 2) {
        player->animation.currentFrame = 2;
        player->animation.frameDirection = -1;
      } else if (player->animation.currentFrame <= 0) {
        player->animation.currentFrame = 0;
        player->animation.frameDirection = 1;
      }

      player->animation.frameTimer = 0.0f;
    }
  }
}

static bool DirectionKeyPressed(const Direction dir) {
  switch (dir) {
  case DIRECTION_BACK:
    return IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
  case DIRECTION_FRONT:
    return IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
  case DIRECTION_LEFT:
    return IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);
  case DIRECTION_RIGHT:
    return IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);
  default:
    return false;
  }
}

static bool GetMovementInput(Vector2 *dir, Direction *outDir) {
  *dir = (Vector2){0};

  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
    dir->y = -1;
    *outDir = DIRECTION_BACK;
    return true;
  }

  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
    dir->y = 1;
    *outDir = DIRECTION_FRONT;
    return true;
  }

  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
    dir->x = -1;
    *outDir = DIRECTION_LEFT;
    return true;
  }

  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
    dir->x = 1;
    *outDir = DIRECTION_RIGHT;
    return true;
  }

  return false;
}

static bool IsTileSolid(const Vector2 tilePos, const Collusion *collusion,
                        const int count) {
  const Vector2 worldPos = {tilePos.x * TILE_SIZE + TILE_SIZE / 2.0f,
                            tilePos.y * TILE_SIZE + TILE_SIZE / 2.0f};

  for (int c = 0; c < count; c++) {
    for (int i = 0; i < collusion[c].rectCount; i++) {
      if (CheckCollisionPointRec(worldPos, collusion[c].rect[i])) {
        return true;
      }
    }
  }
  return false;
}

void UpdatePlayer(Player *player, const float frameTime,
                  const Collusion *collusion, const int count) {
  Vector2 inputDir = {0};
  Direction nextDir = player->graphics.direction;
  const bool hasInput = GetMovementInput(&inputDir, &nextDir);
  const bool isShift = IsKeyDown(KEY_LEFT_SHIFT);

  player->movement.isHoldingKey = hasInput;

  if (!player->movement.isMoving) {
    if (!hasInput) {
      player->movement.holdTimer = 0.0f;
      player->movement.justTurned = false;
      player->graphics.action = ACTION_IDLE;
    } else {
      const bool dirChanged = player->graphics.direction != nextDir;

      if (dirChanged && DirectionKeyPressed(nextDir)) {
        player->graphics.direction = nextDir;
        player->graphics.action = ACTION_IDLE;
        player->movement.holdTimer = 0.0f;
        player->movement.justTurned = true;
      } else {
        player->movement.holdTimer += frameTime;

        if (!player->movement.justTurned ||
            player->movement.holdTimer >= player->movement.holdDelay) {

          const Vector2 targetPos = {
              player->movement.tilePosition.x + inputDir.x,
              player->movement.tilePosition.y + inputDir.y};

          if (!IsTileSolid(targetPos, collusion, count)) {
            player->movement.holdTimer = 0.0f;
            player->movement.justTurned = false;
            StartMoving(player, inputDir, nextDir, isShift);
          } else {
            player->graphics.direction = nextDir;
            player->graphics.action = ACTION_IDLE;
          }
        }
      }
    }
  }

  if (player->movement.isMoving) {
    player->movement.moveTimer += frameTime;
    float time = player->movement.moveTimer / player->movement.moveDuration;

    if (time >= 1.0f) {
      player->movement.tilePosition = player->movement.targetTilePosition;

      Vector2 freshInputDir = {0};
      Direction freshNextDir;
      const bool stillHolding = GetMovementInput(&freshInputDir, &freshNextDir);

      if (stillHolding) {
        const Vector2 nextTarget = {
            player->movement.tilePosition.x + freshInputDir.x,
            player->movement.tilePosition.y + freshInputDir.y};

        if (!IsTileSolid(nextTarget, collusion, count)) {
          player->movement.targetTilePosition = nextTarget;
          player->movement.moveTimer -= player->movement.moveDuration;
          player->movement.moveDuration = isShift ? 0.25f : 0.42f;
          player->graphics.action = isShift ? ACTION_RUN : ACTION_WALK;
          player->graphics.direction =
              freshNextDir; // Pake freshNextDir biar update

          time = player->movement.moveTimer / player->movement.moveDuration;
        } else {
          player->movement.isMoving = false;
          player->movement.moveTimer = 0.0f;
          player->graphics.action = ACTION_IDLE;
          time = 1.0f;
        }
      } else {
        player->movement.isMoving = false;
        player->movement.moveTimer = 0.0f;
        player->graphics.action = ACTION_IDLE;
        time = 1.0f;
      }
    }

    if (time > 1.0f)
      time = 1.0f;

    const Vector2 startPosition = {player->movement.tilePosition.x * TILE_SIZE,
                                   player->movement.tilePosition.y * TILE_SIZE};

    const Vector2 endPosition = {
        player->movement.targetTilePosition.x * TILE_SIZE,
        player->movement.targetTilePosition.y * TILE_SIZE};

    player->movement.position.x =
        startPosition.x + (endPosition.x - startPosition.x) * time;
    player->movement.position.y =
        startPosition.y + (endPosition.y - startPosition.y) * time;
  }

  UpdateAnimation(player, frameTime);
}

void DrawPlayer(const Player *player) {
  const int spriteRow = GetSpriteRow(player);

  const Rectangle sourceRect = {
      .x = (float)player->animation.currentFrame * player->graphics.frameSize.x,
      .y = (float)spriteRow * player->graphics.frameSize.y,
      .width = player->graphics.frameSize.x,
      .height = player->graphics.frameSize.y};

  const Rectangle destRec = {
      .x = player->movement.position.x - player->graphics.frameSize.x / 4.0f,
      .y = player->movement.position.y - player->graphics.frameSize.y / 2.0f,
      .width = player->graphics.frameSize.x,
      .height = player->graphics.frameSize.y};

  const Vector2 position = {0};

  DrawTexturePro(player->graphics.bodyTexture, sourceRect, destRec, position,
                 0.0f, WHITE);
  DrawTexturePro(player->graphics.headTexture, sourceRect, destRec, position,
                 0.0f, WHITE);

  for (int i = 0; i < MAX_SLOTS; i++) {
    if (PlayerIsEquipItem(player, i)) {
      DrawTexturePro(player->equipment.equippedTextures[i], sourceRect, destRec,
                     position, 0.0f, WHITE);
    }
  }
}

void PlayerEquipItem(Player *p, const Item newItem) {
  if (newItem.slot < MAX_SLOTS) {
    p->equipment.equippedTextures[newItem.slot] = newItem.texture;
    p->equipment.equippedFlags |= 1 << newItem.slot;
  }
}



bool PlayerIsEquipItem(const Player *p, const EquipSlot slot) {
  return slot < MAX_SLOTS && p->equipment.equippedFlags & 1 << slot;
}
