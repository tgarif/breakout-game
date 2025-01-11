#ifndef SPRITE_RENDERER_H_
#define SPRITE_RENDERER_H_

#include "shader.h"
#include "texture.h"

typedef struct {
    Shader shader;
    unsigned int quadVAO;
} SpriteRenderer;

SpriteRenderer* NewSpriteRenderer(Shader shader);
void DrawSprite(SpriteRenderer* renderer, Texture2D* texture, mfloat_t* position, mfloat_t* size, float rotate, mfloat_t* color);
void DestroySpriteRenderer(SpriteRenderer* renderer);

#endif
