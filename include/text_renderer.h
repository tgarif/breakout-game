#ifndef TEXT_RENDERER_H_
#define TEXT_RENDERER_H_

#include "mathc.h"
#include "shader.h"
#include "util.h"

typedef struct {
    unsigned int textureID;
    mint_t size[VEC2_SIZE];
    mint_t bearing[VEC2_SIZE];
    unsigned int advance;
} Character;

typedef struct {
    DynamicMap characters;
    Shader textShader;
} TextRenderer;

TextRenderer* NewTextRenderer(unsigned int width, unsigned int height);
void LoadText(TextRenderer* tRenderer, char* font, unsigned int fontSize);
void RenderText(TextRenderer* tRenderer, char* text, float x, float y, float scale, mfloat_t* color);

#endif
