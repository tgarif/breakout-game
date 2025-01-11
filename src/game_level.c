#include "game_level.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_object.h"
#include "resource_manager.h"
#include "util.h"

static void init(GameLevel* level, DynamicArray* tileData, unsigned int levelWidth, unsigned int levelHeight) {
    DynamicArray** rows = (DynamicArray**)(tileData->array);
    unsigned int height = tileData->size;
    unsigned int width = rows[0]->size;
    float unit_width = levelWidth / (float)width, unit_height = levelHeight / (float)height;

    for (size_t y = 0; y < height; ++y) {
        DynamicArray* yData = rows[y];
        for (size_t x = 0; x < width; ++x) {
            unsigned int yxData = ((unsigned int*)yData->array)[x];

            if (yxData == 1) {
                GameObject* obj = NewGameObject(
                    (mfloat_t[]){unit_width * x, unit_height * y},
                    (mfloat_t[]){unit_width, unit_height},
                    GetTexture("block_solid"),
                    (mfloat_t[]){0.8f, 0.8f, 0.7f},
                    NULL);

                obj->isSolid = true;
                pushPtr(&level->bricks, obj);
            } else if (yxData > 1) {
                mfloat_t* color = (mfloat_t[]){1.0f, 1.0f, 1.0f};
                if (yxData == 2)
                    color = (mfloat_t[]){0.2f, 0.6f, 1.0f};
                if (yxData == 3)
                    color = (mfloat_t[]){0.0f, 0.7f, 0.0f};
                if (yxData == 4)
                    color = (mfloat_t[]){0.8f, 0.8f, 0.4f};
                if (yxData == 5)
                    color = (mfloat_t[]){1.0f, 0.5f, 0.0f};

                pushPtr(
                    &level->bricks,
                    NewGameObject(
                        (mfloat_t[]){unit_width * x, unit_height * y},
                        (mfloat_t[]){unit_width, unit_height},
                        GetTexture("block"),
                        color,
                        NULL  // #
                        )     // #
                );
            }
        }
    }
}

static void processLine(const char* line, void* context) {
    char* lineCopy = custom_strdup(line);

    DynamicArray* outerTileArray = (DynamicArray*)context;
    DynamicArray* innerTileArray = malloc(sizeof(DynamicArray));
    initialize(innerTileArray, 50, sizeof(unsigned int));

    char* token = strtok(lineCopy, " ");
    while (token != NULL) {
        char* endptr;
        errno = 0;
        unsigned long value = strtoul(token, &endptr, 10);

        if (errno != 0 || *endptr != '\0') {
            fprintf(stderr, "Error: Invalid unsigned integer: %s\n", token);
        } else if (value > UINT_MAX) {
            fprintf(stderr, "Error: Value out of range for unsigned int: %lu\n", value);
        } else {
            unsigned int parsedValue = (unsigned int)value;
            push(innerTileArray, &parsedValue);
        }

        token = strtok(NULL, " ");
    }
    pushPtr(outerTileArray, innerTileArray);

    free(lineCopy);
}

static void clearArrayCallback(void* obj) {
    GameObject* gameObj = *(GameObject**)obj;
    CleanupGameObject(gameObj);
}

static void cleanupOuterArrayCallback(void* item) {
    DynamicArray* innerArray = *(DynamicArray**)item;
    cleanup(innerArray, NULL);
    free(innerArray);
}

GameLevel* NewGameLevel() {
    GameLevel* level = malloc(sizeof(GameLevel));
    initialize(&level->bricks, 256, sizeof(GameObject*));
    return level;
}

void LoadLevel(GameLevel* level, const char* file, unsigned int levelWidth, unsigned int levelHeight) {
    clearArray(&level->bricks, clearArrayCallback);
    DynamicArray outerTileArray;
    initialize(&outerTileArray, 50, sizeof(DynamicArray*));
    readAndProcessLine(file, processLine, &outerTileArray);
    if (outerTileArray.size > 0) {
        init(level, &outerTileArray, levelWidth, levelHeight);
    }
    cleanup(&outerTileArray, cleanupOuterArrayCallback);
}

void DrawLevel(GameLevel* level, SpriteRenderer* renderer) {
    DYNAMIC_ARRAY_FOR_EACH_PTR(&level->bricks, GameObject, tile) {
        if (!(*tile)->destroyed)
            DrawGameObject(*tile, renderer);
    }
}

bool IsLevelCompleted(GameLevel* level) {
    DYNAMIC_ARRAY_FOR_EACH_PTR(&level->bricks, GameObject, tile) {
        if (!(*tile)->isSolid && !(*tile)->destroyed)
            return false;
    }
    return true;
}
