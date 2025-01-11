#ifndef GAME_H_
#define GAME_H_

#include <stdbool.h>

#include "util.h"

typedef enum {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
} GameState;

typedef struct {
    GameState state;
    bool keys[1024];
    unsigned int width, height;
    DynamicArray levels;
    unsigned int level;
} Game;

Game* NewGame(Game* game, unsigned int width, unsigned int height);
void InitGame(Game* game);
void ProcessGameInput(Game* game, float dt);
void UpdateGame(Game* game, float dt);
void RenderGame(Game* game);
void DetroyGame(Game* game);

#endif
