#include "game_object.h"

#include <stdlib.h>

#include "sprite_renderer.h"

GameObject* NewGameObject(mfloat_t* pos, mfloat_t* size, Texture2D* sprite, mfloat_t* color, mfloat_t* velocity) {
    GameObject* gameObj = malloc(sizeof(GameObject));

    gameObj->position[0] = pos[0];
    gameObj->position[1] = pos[1];

    gameObj->size[0] = size[0];
    gameObj->size[1] = size[1];

    if (color) {
        gameObj->color[0] = color[0];
        gameObj->color[1] = color[1];
        gameObj->color[2] = color[2];
    } else {
        gameObj->color[0] = 1.0f;
        gameObj->color[1] = 1.0f;
        gameObj->color[2] = 1.0f;
    }

    if (velocity) {
        gameObj->velocity[0] = velocity[0];
        gameObj->velocity[1] = velocity[1];
    } else {
        gameObj->velocity[0] = 0.0f;
        gameObj->velocity[1] = 0.0f;
    }

    gameObj->rotation = 0.0f;
    gameObj->sprite = sprite;
    gameObj->isSolid = false;
    gameObj->destroyed = false;

    return gameObj;
}

void DrawGameObject(GameObject* gameObj, SpriteRenderer* renderer) {
    DrawSprite(renderer, gameObj->sprite, gameObj->position, gameObj->size, gameObj->rotation, gameObj->color);
}

void CleanupGameObject(GameObject* gameObj) {
    free(gameObj);
}
