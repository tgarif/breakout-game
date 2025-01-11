#include "game.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#include "game_level.h"
#include "game_object.h"
#include "resource_manager.h"
#include "shader.h"
#include "sprite_renderer.h"

const mfloat_t PLAYER_SIZE[VEC2_SIZE] = {100.0f, 20.0f};
const float PLAYER_VELOCITY = 500.0f;

static SpriteRenderer* renderer = NULL;
static GameObject* player = NULL;

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
    Shader spriteShaderId = LoadShader("shaders/sprite.vs", "shaders/sprite.frag", NULL, "sprite");
    mfloat_t projection[MAT4_SIZE];
    mat4_ortho(projection, 0.0f, (float)game->width, (float)game->height, 0.0f, -1.0f, 1.0f);
    UseShader(spriteShaderId);
    setInteger(spriteShaderId, "image", 0, false);
    setMat4fv(spriteShaderId, "projection", projection, false);
    renderer = NewSpriteRenderer(spriteShaderId);
    // Load textures
    LoadTexture("textures/background.jpg", false, "background");
    LoadTexture("textures/awesomeface.png", true, "face");
    LoadTexture("textures/block.png", false, "block");
    LoadTexture("textures/block_solid.png", false, "block_solid");
    LoadTexture("textures/paddle.png", true, "paddle");
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
    player = NewGameObject(playerPos, (mfloat_t[VEC2_SIZE]){PLAYER_SIZE[0], PLAYER_SIZE[1]}, GetTexture("paddle"), NULL, NULL);
}

void ProcessGameInput(Game* game, float dt) {
    if (game->state == GAME_ACTIVE) {
        float velocity = PLAYER_VELOCITY * dt;

        if (game->keys[GLFW_KEY_A]) {
            if (player->position[0] >= 0.0f)
                player->position[0] -= velocity;
        }
        if (game->keys[GLFW_KEY_D]) {
            if (player->position[0] <= game->width - player->size[0])
                player->position[0] += velocity;
        }
    }
}

void UpdateGame(Game* game, float dt) {
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
    }
}

void DetroyGame(Game* game) {
    if (renderer) {
        DestroySpriteRenderer(renderer);
    }
    if (player) {
        CleanupGameObject(player);
    }
}
