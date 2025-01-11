#include "power_up.h"

#include <stdlib.h>

#include "mathc.h"

const mfloat_t POWERUP_SIZE[VEC2_SIZE] = {60.0f, 20.0f};
const mfloat_t VELOCITY[VEC2_SIZE] = {0.0f, 150.0f};

PowerUp* NewPowerUp(char* type, mfloat_t* color, float duration, mfloat_t* position, Texture2D* texture) {
    PowerUp* powerup = malloc(sizeof(PowerUp));

    powerup->base.position[0] = position[0];
    powerup->base.position[1] = position[1];

    powerup->base.size[0] = POWERUP_SIZE[0];
    powerup->base.size[1] = POWERUP_SIZE[1];

    powerup->base.sprite = texture;

    powerup->base.color[0] = color[0];
    powerup->base.color[1] = color[1];
    powerup->base.color[2] = color[2];

    powerup->base.velocity[0] = VELOCITY[0];
    powerup->base.velocity[1] = VELOCITY[1];

    powerup->base.isSolid = false;
    powerup->base.destroyed = false;

    powerup->type = type;
    powerup->duration = duration;
    powerup->activated = false;

    return powerup;
}

void DrawPowerUp(PowerUp* powerup, SpriteRenderer* renderer) {
    DrawSprite(renderer, powerup->base.sprite, powerup->base.position, powerup->base.size, powerup->base.rotation, powerup->base.color);
}
void CleanupPowerUp(PowerUp* powerup) {
    CleanupGameObject(&powerup->base);
    free(powerup);
}
