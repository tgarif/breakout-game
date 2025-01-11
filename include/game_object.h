#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include "mathc.h"
#include "sprite_renderer.h"
#include "texture.h"

typedef struct {
    mfloat_t position[VEC2_SIZE], size[VEC2_SIZE], velocity[VEC2_SIZE];
    mfloat_t color[VEC3_SIZE];
    float rotation;
    bool isSolid;
    bool destroyed;
    Texture2D* sprite;
} GameObject;

GameObject* NewGameObject(mfloat_t* pos, mfloat_t* size, Texture2D* sprite, mfloat_t* color, mfloat_t* velocity);
void DrawGameObject(GameObject* gameObj, SpriteRenderer* renderer);
void CleanupGameObject(GameObject* gameObj);

#endif
