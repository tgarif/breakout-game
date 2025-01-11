#ifndef GAME_LEVEL_H_
#define GAME_LEVEL_H_

#include "sprite_renderer.h"
#include "util.h"

typedef struct {
    DynamicArray bricks;
} GameLevel;

GameLevel* NewGameLevel();
void LoadLevel(GameLevel* level, const char* file, unsigned int levelWidth, unsigned int levelHeight);
void DrawLevel(GameLevel* level, SpriteRenderer* renderer);
bool IsLevelCompleted(GameLevel* level);

#endif
