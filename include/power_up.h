#ifndef POWER_UP_H_
#define POWER_UP_H_

#include "game_object.h"

typedef struct {
    GameObject base;
    char* type;
    float duration;
    bool activated;
} PowerUp;

PowerUp* NewPowerUp(char* type, mfloat_t* color, float duration, mfloat_t* position, Texture2D* texture);
void DrawPowerUp(PowerUp* powerup, SpriteRenderer* renderer);
void CleanupPowerUp(PowerUp* powerup);

#endif
