#include "game.h"

Game* NewGame(Game* game, unsigned int width, unsigned int height) {
    *game = (Game){
        .state = GAME_ACTIVE,
        .keys = {false},
        .width = width,
        .height = height,
    };

    return game;
}

void InitGame(Game* game) {
}

void ProcessGameInput(Game* game, float dt) {
}

void UpdateGame(Game* game, float dt) {
}

void RenderGame(Game* game) {
}
