#ifndef GAME_TYPES_H
#define GAME_TYPES_H

typedef enum {
    DIRECTION_FRONT,
    DIRECTION_BACK,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

typedef enum {
    SLOT_BOTTOM     = 1 << 0,
    SLOT_TOP        = 1 << 1,
    SLOT_MASK       = 1 << 2,
    SLOT_HAIR       = 1 << 3,
    SLOT_FEET       = 1 << 4,
    SLOT_HAND_MAIN  = 1 << 5,
    MAX_SLOTS
} EquipSlot;

typedef enum {
    ACTION_IDLE,
    ACTION_WALK,
    ACTION_RUN
} Action;

typedef enum {
    TRANS_NONE,
    TRANS_FADING_OUT,
    TRANS_SWITCHING,
    TRANS_FADING_IN
} TransitionState;

#endif