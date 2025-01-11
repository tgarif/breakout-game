#include "game.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ball_object.h"
#include "game_level.h"
#include "game_object.h"
#include "mathc.h"
#include "particle_generator.h"
#include "resource_manager.h"
#include "shader.h"
#include "sprite_renderer.h"

const mfloat_t PLAYER_SIZE[VEC2_SIZE] = {100.0f, 20.0f};
const float PLAYER_VELOCITY = 500.0f;
const mfloat_t INITIAL_BALL_VELOCITY[VEC2_SIZE] = {100.0f, -350.0f};
/* const mfloat_t INITIAL_BALL_VELOCITY[VEC2_SIZE] = {200.0f, -550.0f}; */
const float BALL_RADIUS = 12.5f;

static SpriteRenderer* renderer = NULL;
static GameObject* player = NULL;
static BallObject* ball = NULL;

static Direction VectorDirection(mfloat_t* target) {
    mfloat_t* compass[4] = {
        (mfloat_t[VEC2_SIZE]){0.0f, 1.0f},   // up
        (mfloat_t[VEC2_SIZE]){1.0f, 0.0f},   // right
        (mfloat_t[VEC2_SIZE]){0.0f, -1.0f},  // down
        (mfloat_t[VEC2_SIZE]){-1.0f, 0.0f},  // left
    };
    float max = 0.0f;
    unsigned int best_match = 0;

    mfloat_t normalized_target[VEC2_SIZE];
    vec2_normalize(normalized_target, target);

    for (size_t i = 0; i < 4; i++) {
        float dot_product = vec2_dot(normalized_target, compass[i]);
        if (dot_product > max) {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

static Collision CheckCollision(BallObject* one, GameObject* two) {
    mfloat_t center[VEC2_SIZE];
    vec2_add_f(center, one->base.position, one->radius);

    mfloat_t aabb_half_extents[VEC2_SIZE] = {two->size[0] / 2.0f, two->size[1] / 2.0f};
    mfloat_t aabb_center[VEC2_SIZE] = {two->position[0] + aabb_half_extents[0], two->position[1] + aabb_half_extents[1]};

    mfloat_t difference[VEC2_SIZE];
    vec2_subtract(difference, center, aabb_center);
    mfloat_t clamped[VEC2_SIZE];
    mfloat_t negative_aabb[VEC2_SIZE];
    vec2_negative(negative_aabb, aabb_half_extents);
    vec2_clamp(clamped, difference, negative_aabb, aabb_half_extents);

    mfloat_t closest[VEC2_SIZE];
    vec2_add(closest, aabb_center, clamped);

    vec2_subtract(difference, closest, center);

    if (vec2_length(difference) < one->radius) {
        return (Collision){
            .hasCollision = true,
            .direction = VectorDirection(difference),
            .collisionPoint = {difference[0], difference[1]},
        };
    } else {
        return (Collision){
            .hasCollision = false,
            .direction = UP,
            .collisionPoint = {0.0f, 0.0f},
        };
    }
}

Game* NewGame(Game* game, unsigned int width, unsigned int height) {
    *game = (Game){
        .state = GAME_ACTIVE,
        .width = width,
        .height = height,
        .keys = {false},
    };
    initialize(&game->levels, 4, sizeof(GameLevel*));
    return game;
}

void InitGame(Game* game) {
    // Load shaders
    Shader spriteShaderId = LoadShader("shaders/sprite.vs", "shaders/sprite.frag", NULL, "sprite");
    Shader particleShaderId = LoadShader("shaders/particle.vs", "shaders/particle.frag", NULL, "particle");
    // Configure shaders
    mfloat_t projection[MAT4_SIZE];
    mat4_ortho(projection, 0.0f, (float)game->width, (float)game->height, 0.0f, -1.0f, 1.0f);
    UseShader(spriteShaderId);
    setInteger(spriteShaderId, "image", 0, false);
    setMat4fv(spriteShaderId, "projection", projection, false);
    UseShader(particleShaderId);
    setInteger(particleShaderId, "sprite", 0, false);
    setMat4fv(particleShaderId, "projection", projection, false);
    // Load textures
    LoadTexture("textures/background.jpg", false, "background");
    LoadTexture("textures/awesomeface.png", true, "face");
    LoadTexture("textures/block.png", false, "block");
    LoadTexture("textures/block_solid.png", false, "block_solid");
    LoadTexture("textures/paddle.png", true, "paddle");
    LoadTexture("textures/particle.png", true, "particle");
    // Set render-specific controls
    renderer = NewSpriteRenderer(spriteShaderId);
    NewParticleGenerator(particleShaderId, GetTexture("particle"), 500);
    // Load levels
    GameLevel* one = NewGameLevel();
    LoadLevel(one, "levels/one.lvl", game->width, game->height / 2);
    GameLevel* two = NewGameLevel();
    LoadLevel(two, "levels/two.lvl", game->width, game->height / 2);
    GameLevel* three = NewGameLevel();
    LoadLevel(three, "levels/three.lvl", game->width, game->height / 2);
    GameLevel* four = NewGameLevel();
    LoadLevel(four, "levels/four.lvl", game->width, game->height / 2);
    pushPtr(&game->levels, one);
    pushPtr(&game->levels, two);
    pushPtr(&game->levels, three);
    pushPtr(&game->levels, four);
    game->level = 0;
    // Configure game objects
    mfloat_t playerPos[VEC2_SIZE] = {
        game->width / 2.0f - PLAYER_SIZE[0] / 2.0f,
        game->height - PLAYER_SIZE[1]  // #
    };
    player = NewGameObject(playerPos, (mfloat_t*)PLAYER_SIZE, GetTexture("paddle"), NULL, NULL);
    mfloat_t ballPos[VEC2_SIZE];
    vec2_add(ballPos, playerPos, (mfloat_t[VEC2_SIZE]){PLAYER_SIZE[0] / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f});
    ball = NewBallObject(ballPos, BALL_RADIUS, (mfloat_t*)INITIAL_BALL_VELOCITY, GetTexture("face"));
}

void ProcessGameInput(Game* game, float dt) {
    if (game->state == GAME_ACTIVE) {
        float velocity = PLAYER_VELOCITY * dt;

        if (game->keys[GLFW_KEY_A]) {
            if (player->position[0] >= 0.0f) {
                player->position[0] -= velocity;
                if (ball->stuck)
                    ball->base.position[0] -= velocity;
            }
        }
        if (game->keys[GLFW_KEY_D]) {
            if (player->position[0] <= game->width - player->size[0]) {
                player->position[0] += velocity;
                if (ball->stuck)
                    ball->base.position[0] += velocity;
            }
        }
        if (game->keys[GLFW_KEY_SPACE])
            ball->stuck = false;
    }
}

void UpdateGame(Game* game, float dt) {
    MoveBall(ball, dt, game->width);
    DoCollisions(game);
    UpdateParticle(dt, ball, 2, (mfloat_t[VEC2_SIZE]){ball->radius / 2.0f, ball->radius / 2.0f});
    if (ball->base.position[1] >= game->height) {
        ResetLevel(game);
        ResetPlayer(game);
    }
}

void RenderGame(Game* game) {
    if (game->state == GAME_ACTIVE) {
        // Draw background
        DrawSprite(
            renderer,
            GetTexture("background"),
            (mfloat_t[VEC2_SIZE]){0.0f, 0.0f},
            (mfloat_t[VEC2_SIZE]){game->height, game->width},
            0.0f,
            NULL  // #
        );
        // Draw level
        GameLevel* level = ((GameLevel**)(game->levels.array))[game->level];
        DrawLevel(level, renderer);
        // Draw player
        DrawGameObject(player, renderer);
        // Draw particles
        DrawParticle();
        // Draw ball
        DrawBall(ball, renderer);
    }
}

void DoCollisions(Game* game) {
    GameLevel* level = ((GameLevel**)(game->levels.array))[game->level];
    DYNAMIC_ARRAY_FOR_EACH_PTR(&level->bricks, GameObject, box) {
        if (!(*box)->destroyed) {
            Collision collision = CheckCollision(ball, *box);
            if (collision.hasCollision) {
                if (!(*box)->isSolid)
                    (*box)->destroyed = true;

                if (collision.direction == LEFT || collision.direction == RIGHT) {
                    ball->base.velocity[0] = -ball->base.velocity[0];
                    float penetration = ball->radius - MFABS(collision.collisionPoint[0]);
                    if (collision.direction == LEFT)
                        ball->base.position[0] += penetration;
                    else
                        ball->base.position[0] -= penetration;
                } else {
                    ball->base.velocity[1] = -ball->base.velocity[1];
                    float penetration = ball->radius - MFABS(collision.collisionPoint[1]);
                    if (collision.direction == UP)
                        ball->base.position[1] -= penetration;
                    else
                        ball->base.position[1] += penetration;
                }
            }
        }
    }
    Collision result = CheckCollision(ball, player);
    if (!ball->stuck && result.hasCollision) {
        float centerBoard = player->position[0] + player->size[0] / 2.0f;
        float distance = (ball->base.position[0] + ball->radius) - centerBoard;
        float percentage = distance / (player->size[0] / 2.0f);

        float strength = 2.0f;
        mfloat_t oldVelocity[VEC2_SIZE];
        vec2_assign(oldVelocity, ball->base.velocity);
        ball->base.velocity[0] = INITIAL_BALL_VELOCITY[0] * percentage * strength;
        vec2_multiply_f(ball->base.velocity, vec2_normalize(ball->base.velocity, ball->base.velocity), vec2_length(oldVelocity));
        ball->base.velocity[1] = -1.0f * MFABS(ball->base.velocity[1]);
    }
}

void ResetLevel(Game* game) {
    if (game->level == 0)
        LoadLevel(((GameLevel**)(game->levels.array))[0], "levels/one.lvl", game->width, game->height / 2);
    if (game->level == 1)
        LoadLevel(((GameLevel**)(game->levels.array))[1], "levels/two.lvl", game->width, game->height / 2);
    if (game->level == 2)
        LoadLevel(((GameLevel**)(game->levels.array))[2], "levels/three.lvl", game->width, game->height / 2);
    if (game->level == 3)
        LoadLevel(((GameLevel**)(game->levels.array))[3], "levels/four.lvl", game->width, game->height / 2);
}

void ResetPlayer(Game* game) {
    vec2_assign(player->size, (mfloat_t*)PLAYER_SIZE);
    vec2_assign(player->position, (mfloat_t[]){game->width / 2.0f - PLAYER_SIZE[0] / 2.0f, game->height - PLAYER_SIZE[1]});
    mfloat_t ballPos[VEC2_SIZE];
    vec2_add(ballPos, player->position, (mfloat_t[]){PLAYER_SIZE[0] / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)});
    ResetBall(ball, ballPos, (mfloat_t*)INITIAL_BALL_VELOCITY);
}

void DetroyGame() {
    if (renderer) {
        DestroySpriteRenderer(renderer);
    }
    if (player) {
        CleanupGameObject(player);
    }
    if (ball) {
        CleanupBallObject(ball);
    }
}
