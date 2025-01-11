#include "sprite_renderer.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#include "mathc.h"
#include "shader.h"
#include "texture.h"

static void initRenderData(SpriteRenderer* renderer) {
    unsigned int VBO;
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,  //
    };

    glGenVertexArrays(1, &renderer->quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(renderer->quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SpriteRenderer* NewSpriteRenderer(Shader shader) {
    SpriteRenderer* renderer = malloc(sizeof(SpriteRenderer));
    *renderer = (SpriteRenderer){
        .shader = shader,
        .quadVAO = 0,
    };
    initRenderData(renderer);
    return renderer;
}

void DrawSprite(SpriteRenderer* renderer, Texture2D* texture, mfloat_t* position, mfloat_t* size, float rotate, mfloat_t* color) {
    UseShader(renderer->shader);
    mfloat_t identity[MAT4_SIZE];
    mfloat_t model[MAT4_SIZE];
    mat4_translate(model, mat4_identity(identity), (mfloat_t[VEC3_SIZE]){position[0], position[1], 0.0f});
    mat4_translate(identity, mat4_identity(identity), (mfloat_t[VEC3_SIZE]){0.5f * size[0], 0.5f * size[1], 0.0f});
    mat4_multiply(model, model, identity);
    mat4_rotation_axis(identity, (mfloat_t[VEC3_SIZE]){0.0f, 0.0f, 1.0f}, MRADIANS(rotate));
    mat4_multiply(model, model, identity);
    mat4_translate(identity, mat4_identity(identity), (mfloat_t[VEC3_SIZE]){-0.5f * size[0], -0.5f * size[1], 0.0f});
    mat4_multiply(model, model, identity);
    mat4_scale(identity, mat4_identity(identity), (mfloat_t[VEC3_SIZE]){size[0], size[1], 1.0f});
    mat4_multiply(model, model, identity);

    setMat4fv(renderer->shader, "model", model, false);
    setVec3fv(renderer->shader, "spriteColor", color == NULL ? (mfloat_t[]){1.0f, 1.0f, 1.0f} : color, false);

    glActiveTexture(GL_TEXTURE0);
    BindTexture(texture);

    glBindVertexArray(renderer->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void DestroySpriteRenderer(SpriteRenderer* renderer) {
    glDeleteVertexArrays(1, &renderer->quadVAO);
    free(renderer);
}
