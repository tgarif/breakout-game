#ifndef BALL_OBJECT_H_
#define BALL_OBJECT_H_

#include "game_object.h"
#include "mathc.h"
#include "sprite_renderer.h"
#include "texture.h"

typedef struct {
    GameObject base;
    float radius;
    bool stuck;
} BallObject;

BallObject* NewBallObject(mfloat_t* pos, float radius, mfloat_t* velocity, Texture2D* sprite);
void DrawBall(BallObject* ballObj, SpriteRenderer* renderer);
void CleanupBallObject(BallObject* ballObj);

mfloat_t* MoveBall(BallObject* ballObj, float dt, unsigned int window_width);
void ResetBall(BallObject* ballObj, mfloat_t* position, mfloat_t* velocity);

#endif
