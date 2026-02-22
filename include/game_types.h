#ifndef GAME_TYPES_H
#define GAME_TYPES_H

typedef enum {
    DIRECTION_FRONT,
    DIRECTION_BACK,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

typedef enum {
    SLOT_BOTTOM,
    SLOT_TOP,
    SLOT_MASK,
    SLOT_HAIR,
    SLOT_FEET,
    SLOT_HAND_MAIN,
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