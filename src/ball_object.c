#include "ball_object.h"

#include <stdlib.h>

#include "game_object.h"
#include "mathc.h"
#include "sprite_renderer.h"

BallObject* NewBallObject(mfloat_t* pos, float radius, mfloat_t* velocity, Texture2D* sprite) {
    BallObject* ball = malloc(sizeof(BallObject));

    ball->base.position[0] = pos[0];
    ball->base.position[1] = pos[1];

    ball->base.size[0] = (radius * 2.0f);
    ball->base.size[1] = (radius * 2.0f);

    ball->base.color[0] = 1.0f;
    ball->base.color[1] = 1.0f;
    ball->base.color[2] = 1.0f;

    if (velocity) {
        ball->base.velocity[0] = velocity[0];
        ball->base.velocity[1] = velocity[1];
    } else {
        ball->base.velocity[0] = 0.0f;
        ball->base.velocity[1] = 0.0f;
    }

    ball->base.rotation = 0.0f;
    ball->base.sprite = sprite;
    ball->base.isSolid = false;
    ball->base.destroyed = false;

    ball->radius = radius;
    ball->stuck = true;

    return ball;
}

void DrawBall(BallObject* ballObj, SpriteRenderer* renderer) {
    DrawSprite(renderer, ballObj->base.sprite, ballObj->base.position, ballObj->base.size, ballObj->base.rotation, ballObj->base.color);
}

void CleanupBallObject(BallObject* ballObj) {
    CleanupGameObject(&ballObj->base);
    free(ballObj);
}

mfloat_t* MoveBall(BallObject* ballObj, float dt, unsigned int window_width) {
    if (!ballObj->stuck) {
        mfloat_t tempVelocity[VEC2_SIZE];
        vec2_add(
            ballObj->base.position,
            ballObj->base.position,
            vec2_multiply_f(tempVelocity, ballObj->base.velocity, dt)  // #
        );

        if (ballObj->base.position[0] <= 0.0f) {
            ballObj->base.velocity[0] = -ballObj->base.velocity[0];
            ballObj->base.position[0] = 0.0f;
        } else if (ballObj->base.position[0] + ballObj->base.size[0] >= window_width) {
            ballObj->base.velocity[0] = -ballObj->base.velocity[0];
            ballObj->base.position[0] = window_width - ballObj->base.size[0];
        }

        if (ballObj->base.position[1] <= 0.0f) {
            ballObj->base.velocity[1] = -ballObj->base.velocity[1];
            ballObj->base.position[1] = 0.0f;
        }
    }

    return ballObj->base.position;
}

void ResetBall(BallObject* ballObj, mfloat_t* position, mfloat_t* velocity) {
    vec2_assign(ballObj->base.position, position);
    vec2_assign(ballObj->base.velocity, velocity);
    ballObj->stuck = true;
}
