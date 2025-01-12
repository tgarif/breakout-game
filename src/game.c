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
#include "post_processing.h"
#include "power_up.h"
#include "resource_manager.h"
#include "shader.h"
#include "sprite_renderer.h"
#include "text_renderer.h"
#include "util.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

const mfloat_t PLAYER_SIZE[VEC2_SIZE] = {100.0f, 20.0f};
const float PLAYER_VELOCITY = 500.0f;
const mfloat_t INITIAL_BALL_VELOCITY[VEC2_SIZE] = {100.0f, -350.0f};
const float BALL_RADIUS = 12.5f;

float shakeTime = 0.0f;

static SpriteRenderer* renderer = NULL;
static GameObject* player = NULL;
static BallObject* ball = NULL;
static PostProcessor* effects = NULL;
static ma_engine engine;
static ma_sound backgroundMusic;
static TextRenderer* text = NULL;

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

static bool CheckCollisionPowerUp(GameObject* one, PowerUp* two) {
    bool collisionX = one->position[0] + one->size[0] >= two->base.position[0] &&
                      two->base.position[0] + two->base.size[0] >= one->position[0];
    bool collisionY = one->position[1] + one->size[1] >= two->base.position[1] &&
                      two->base.position[1] + two->base.size[1] >= one->position[1];
    return collisionX && collisionY;
}

static Collision CheckCollisionBall(BallObject* one, GameObject* two) {
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

static bool isOtherPowerUpActive(DynamicArray* powerups, char* type) {
    DYNAMIC_ARRAY_FOR_EACH_PTR(powerups, PowerUp, powerUp) {
        if ((*powerUp)->activated)
            if (strcmp((*powerUp)->type, type) == 0)
                return true;
    }
    return false;
}

static bool isPowerUpRemovable(const void* element) {
    PowerUp* powerUp = *(PowerUp**)element;
    return powerUp->base.destroyed && !powerUp->activated;
}

static bool ShouldSpawn(unsigned int chance) {
    unsigned int random = rand() % chance;
    return random == 0;
}

static void ActivatePowerUp(PowerUp* powerup) {
    if (strcmp(powerup->type, "speed") == 0) {
        vec2_multiply_f(ball->base.velocity, ball->base.velocity, 1.2);
    } else if (strcmp(powerup->type, "sticky") == 0) {
        ball->sticky = true;
        player->color[0] = 1.0f;
        player->color[1] = 0.5f;
        player->color[2] = 1.0f;
    } else if (strcmp(powerup->type, "pass-through") == 0) {
        ball->passthrough = true;
        ball->base.color[0] = 1.0f;
        ball->base.color[1] = 0.5f;
        ball->base.color[2] = 0.5f;
    } else if (strcmp(powerup->type, "pad-size-increase") == 0) {
        player->size[0] += 50;
    } else if (strcmp(powerup->type, "confuse") == 0) {
        if (!effects->chaos)
            effects->confuse = true;
    } else if (strcmp(powerup->type, "chaos") == 0) {
        if (!effects->confuse)
            effects->chaos = true;
    }
}

Game* NewGame(Game* game, unsigned int width, unsigned int height) {
    *game = (Game){
        .state = GAME_MENU,
        .width = width,
        .height = height,
        .keys = {false},
        .keysProcessed = {false},
        .lives = 3,
    };
    initialize(&game->levels, 4, sizeof(GameLevel*));
    initialize(&game->powerups, 128, sizeof(PowerUp*));
    return game;
}

void InitGame(Game* game) {
    // Load shaders
    Shader spriteShaderId = LoadShader("shaders/sprite.vs", "shaders/sprite.frag", NULL, "sprite");
    Shader particleShaderId = LoadShader("shaders/particle.vs", "shaders/particle.frag", NULL, "particle");
    Shader effectsShaderId = LoadShader("shaders/post_processing.vs", "shaders/post_processing.frag", NULL, "postprocessing");
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
    LoadTexture("textures/powerup_speed.png", true, "powerup_speed");
    LoadTexture("textures/powerup_sticky.png", true, "powerup_sticky");
    LoadTexture("textures/powerup_increase.png", true, "powerup_increase");
    LoadTexture("textures/powerup_confuse.png", true, "powerup_confuse");
    LoadTexture("textures/powerup_chaos.png", true, "powerup_chaos");
    LoadTexture("textures/powerup_passthrough.png", true, "powerup_passthrough");
    // Set render-specific controls
    renderer = NewSpriteRenderer(spriteShaderId);
    NewParticleGenerator(particleShaderId, GetTexture("particle"), 500);
    effects = NewPostProcessor(effectsShaderId, game->width, game->height);
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
    // Audio
    ma_engine_init(NULL, &engine);
    ma_sound_init_from_file(&engine, "audio/breakout.mp3", MA_SOUND_FLAG_STREAM, NULL, NULL, &backgroundMusic);
    ma_sound_set_looping(&backgroundMusic, MA_TRUE);
    ma_sound_start(&backgroundMusic);
    // Text
    text = NewTextRenderer(game->width, game->height);
    LoadText(text, "fonts/ocraext.TTF", 24);
}

void ProcessGameInput(Game* game, float dt) {
    if (game->state == GAME_MENU) {
        if (game->keys[GLFW_KEY_ENTER] && !game->keysProcessed[GLFW_KEY_ENTER]) {
            game->state = GAME_ACTIVE;
            game->keysProcessed[GLFW_KEY_ENTER] = true;
        }
        if (game->keys[GLFW_KEY_W] && !game->keysProcessed[GLFW_KEY_W]) {
            game->level = (game->level + 1) % 4;
            game->keysProcessed[GLFW_KEY_W] = true;
        }
        if (game->keys[GLFW_KEY_S] && !game->keysProcessed[GLFW_KEY_S]) {
            if (game->level > 0)
                --game->level;
            else
                game->level = 3;

            game->keysProcessed[GLFW_KEY_S] = true;
        }
    }
    if (game->state == GAME_WIN) {
        if (game->keys[GLFW_KEY_ENTER]) {
            game->keysProcessed[GLFW_KEY_ENTER] = true;
            effects->chaos = false;
            game->state = GAME_MENU;
        }
    }
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
    // Update objects
    MoveBall(ball, dt, game->width);
    // Check for collisions
    DoCollisions(game);
    // Update particles
    UpdateParticle(dt, ball, 2, (mfloat_t[VEC2_SIZE]){ball->radius / 2.0f, ball->radius / 2.0f});
    // Update powerups
    UpdatePowerUps(game, dt);
    // Reduce shake time
    if (shakeTime > 0.0f) {
        shakeTime -= dt;
        if (shakeTime <= 0.0f)
            effects->shake = false;
    }
    // Check loss condition
    if (ball->base.position[1] >= game->height) {
        --game->lives;
        if (game->lives == 0) {
            ResetLevel(game);
            game->state = GAME_MENU;
        }
        ResetPlayer(game);
    }
    // Check win condition
    GameLevel* level = ((GameLevel**)(game->levels.array))[game->level];
    if (game->state == GAME_ACTIVE && IsLevelCompleted(level)) {
        ResetLevel(game);
        ResetPlayer(game);
        effects->chaos = true;
        game->state = GAME_WIN;
    }
}

void RenderGame(Game* game) {
    if (game->state == GAME_ACTIVE || game->state == GAME_MENU || game->state == GAME_WIN) {
        BeginPostProcessRender();
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
        DYNAMIC_ARRAY_FOR_EACH_PTR(&game->powerups, PowerUp, powerUp) {
            if (!(*powerUp)->base.destroyed)
                DrawPowerUp(*powerUp, renderer);
        }
        // Draw particles
        DrawParticle();
        // Draw ball
        DrawBall(ball, renderer);
        EndPostProcessRender(effects);
        RenderPostProcess(effects, glfwGetTime());

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Lives:%u", game->lives);
        RenderText(text, buffer, 5.0f, 5.0f, 1.0f, NULL);
    }
    if (game->state == GAME_MENU) {
        RenderText(text, "Press ENTER to start", 490.0f, game->height / 2.0f, 1.0f, NULL);
        RenderText(text, "Press W or S to select level", 485.0f, game->height / 2.0f + 20.0f, 0.75f, NULL);
    }
    if (game->state == GAME_WIN) {
        RenderText(text, "You WON!!!", 560.0f, game->height / 2.0f - 20.0f, 1.0f, (mfloat_t[VEC3_SIZE]){0.0f, 1.0f, 0.0f});
        RenderText(text, "Press ENTER to retry or ESC to quit", 370.0f, game->height / 2.0f, 1.0f, (mfloat_t[VEC3_SIZE]){1.0f, 1.0f, 0.0f});
    }
}

void DoCollisions(Game* game) {
    GameLevel* level = ((GameLevel**)(game->levels.array))[game->level];
    DYNAMIC_ARRAY_FOR_EACH_PTR(&level->bricks, GameObject, box) {
        if (!(*box)->destroyed) {
            Collision collision = CheckCollisionBall(ball, *box);
            if (collision.hasCollision) {
                if (!(*box)->isSolid) {
                    (*box)->destroyed = true;
                    SpawnPowerUps(game, *box);
                    ma_engine_play_sound(&engine, "audio/bleep.mp3", NULL);
                } else {
                    shakeTime = 0.05f;
                    effects->shake = true;
                    ma_engine_play_sound(&engine, "audio/solid.wav", NULL);
                }

                if (!(ball->passthrough && !(*box)->isSolid)) {
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
    }
    DYNAMIC_ARRAY_FOR_EACH_PTR(&game->powerups, PowerUp, powerUp) {
        if (!(*powerUp)->base.destroyed) {
            if ((*powerUp)->base.position[1] >= game->height)
                (*powerUp)->base.destroyed = true;

            if (CheckCollisionPowerUp(player, *powerUp)) {
                ActivatePowerUp(*powerUp);
                (*powerUp)->base.destroyed = true;
                (*powerUp)->activated = true;
                ma_engine_play_sound(&engine, "audio/powerup.wav", NULL);
            }
        }
    }
    Collision result = CheckCollisionBall(ball, player);
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

        ball->stuck = ball->sticky;
        ma_engine_play_sound(&engine, "audio/bleep.wav", NULL);
    }
}

void ResetLevel(Game* game) {
    if (game->level == 0)
        LoadLevel(((GameLevel**)(game->levels.array))[0], "levels/one.lvl", game->width, game->height / 2);
    else if (game->level == 1)
        LoadLevel(((GameLevel**)(game->levels.array))[1], "levels/two.lvl", game->width, game->height / 2);
    else if (game->level == 2)
        LoadLevel(((GameLevel**)(game->levels.array))[2], "levels/three.lvl", game->width, game->height / 2);
    else if (game->level == 3)
        LoadLevel(((GameLevel**)(game->levels.array))[3], "levels/four.lvl", game->width, game->height / 2);

    game->lives = 3;
}

void ResetPlayer(Game* game) {
    // reset player/ball stats
    vec2_assign(player->size, (mfloat_t*)PLAYER_SIZE);
    vec2_assign(player->position, (mfloat_t[]){game->width / 2.0f - PLAYER_SIZE[0] / 2.0f, game->height - PLAYER_SIZE[1]});
    mfloat_t ballPos[VEC2_SIZE];
    vec2_add(ballPos, player->position, (mfloat_t[]){PLAYER_SIZE[0] / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)});
    ResetBall(ball, ballPos, (mfloat_t*)INITIAL_BALL_VELOCITY);
    // also disable all active powerups
    effects->chaos = effects->confuse = false;
    ball->passthrough = ball->sticky = false;
    SET_ARRAY_VAL(player->color, VEC3_SIZE, 1.0f);
    SET_ARRAY_VAL(ball->base.color, VEC3_SIZE, 1.0f);
}

void UpdatePowerUps(Game* game, float dt) {
    DYNAMIC_ARRAY_FOR_EACH_PTR(&game->powerups, PowerUp, powerUp) {
        PowerUp* powerup = *powerUp;
        mfloat_t multiply[VEC2_SIZE];
        vec2_add(powerup->base.position, powerup->base.position, vec2_multiply_f(multiply, powerup->base.velocity, dt));
        if (powerup->activated) {
            powerup->duration -= dt;

            if (powerup->duration <= 0.0f) {
                powerup->activated = false;

                if (strcmp(powerup->type, "sticky") == 0) {
                    if (!isOtherPowerUpActive(&game->powerups, "sticky")) {
                        ball->sticky = false;
                        SET_ARRAY_VAL(player->color, VEC3_SIZE, 1.0f);
                    }
                } else if (strcmp(powerup->type, "pass-through") == 0) {
                    if (!isOtherPowerUpActive(&game->powerups, "pass-through")) {
                        ball->passthrough = false;
                        SET_ARRAY_VAL(ball->base.color, VEC3_SIZE, 1.0f);
                    }
                } else if (strcmp(powerup->type, "confuse") == 0) {
                    if (!isOtherPowerUpActive(&game->powerups, "confuse")) {
                        effects->confuse = false;
                    }
                } else if (strcmp(powerup->type, "chaos") == 0) {
                    if (!isOtherPowerUpActive(&game->powerups, "chaos")) {
                        effects->chaos = false;
                    }
                }
            }
        }
    }

    size_t newEnd = remove_if(&game->powerups, isPowerUpRemovable);
    if (newEnd > 0 && newEnd < game->powerups.size) {
        erase(&game->powerups, newEnd, game->powerups.size);
    }
}

void SpawnPowerUps(Game* game, GameObject* block) {
    if (ShouldSpawn(15)) {
        pushPtr(&game->powerups, NewPowerUp("speed", (mfloat_t[VEC3_SIZE]){0.5f, 0.5f, 0.5f}, 0.0f, block->position, GetTexture("powerup_speed")));
    } else if (ShouldSpawn(15)) {
        pushPtr(&game->powerups, NewPowerUp("sticky", (mfloat_t[VEC3_SIZE]){1.0f, 0.5f, 1.0f}, 20.0f, block->position, GetTexture("powerup_sticky")));
    } else if (ShouldSpawn(15)) {
        pushPtr(&game->powerups, NewPowerUp("pass-through", (mfloat_t[VEC3_SIZE]){0.5f, 1.0f, 0.5f}, 10.0f, block->position, GetTexture("powerup_passthrough")));
    } else if (ShouldSpawn(15)) {
        pushPtr(&game->powerups, NewPowerUp("pad-size-increase", (mfloat_t[VEC3_SIZE]){1.0f, 0.6f, 0.4f}, 0.0f, block->position, GetTexture("powerup_increase")));
    } else if (ShouldSpawn(10)) {
        pushPtr(&game->powerups, NewPowerUp("confuse", (mfloat_t[VEC3_SIZE]){1.0f, 0.3f, 0.3f}, 15.0f, block->position, GetTexture("powerup_confuse")));
    } else if (ShouldSpawn(10)) {
        pushPtr(&game->powerups, NewPowerUp("chaos", (mfloat_t[VEC3_SIZE]){0.9f, 0.25f, 0.25f}, 15.0f, block->position, GetTexture("powerup_chaos")));
    }
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
    if (effects) {
        CleanupPostProcess(effects);
    }
    ma_sound_uninit(&backgroundMusic);
    ma_engine_uninit(&engine);
}
