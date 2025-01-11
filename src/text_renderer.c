#include "text_renderer.h"

#include <GL/glew.h>
#include <ft2build.h>
#include <stdio.h>
#include <stdlib.h>
#include FT_FREETYPE_H

#include "resource_manager.h"
#include "shader.h"
#include "util.h"

static unsigned int VAO, VBO;

TextRenderer* NewTextRenderer(unsigned int width, unsigned int height) {
    TextRenderer* tRenderer = malloc(sizeof(TextRenderer));

    initMap(&tRenderer->characters);

    tRenderer->textShader = LoadShader("shaders/text.vs", "shaders/text.frag", NULL, "text");
    mfloat_t projection[MAT4_SIZE];
    mat4_ortho(projection, 0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    setMat4fv(tRenderer->textShader, "projection", projection, true);
    setInteger(tRenderer->textShader, "text", 0, false);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return tRenderer;
}

void LoadText(TextRenderer* tRenderer, char* font, unsigned int fontSize) {
    clearMap(&tRenderer->characters);
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        fprintf(stderr, "Error: Could not init FreeType Library\n");
    FT_Face face;
    if (FT_New_Face(ft, font, 0, &face))
        fprintf(stderr, "Error: Failed to load font\n");
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (GLubyte c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "Error: Failed to load Glyph\n");
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character* character = malloc(sizeof(Character));
        *character = (Character){
            .textureID = texture,
            .size = {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            .bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top},
            .advance = face->glyph->advance.x,
        };

        char* keyString = malloc(2);
        keyString[0] = (char)c;
        keyString[1] = '\0';

        Key key = {KEY_TYPE_STRING, .strKey = keyString};
        insertIntoMap(&tRenderer->characters, key, character);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void RenderText(TextRenderer* tRenderer, char* text, float x, float y, float scale, mfloat_t* color) {
    UseShader(tRenderer->textShader);
    setVec3fv(tRenderer->textShader, "textColor", color ? color : (mfloat_t[VEC3_SIZE]){1.0f, 1.0f, 1.0f}, false);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    char hString[2];
    hString[0] = 'H';
    hString[1] = '\0';

    Key hKey = {KEY_TYPE_STRING, .strKey = hString};
    void* hRes = getFromMap(&tRenderer->characters, hKey);
    Character hCh = *(Character*)hRes;

    const char* c;
    for (c = text; *c != '\0'; c++) {
        char keyString[2];
        keyString[0] = *c;
        keyString[1] = '\0';

        Key key = {KEY_TYPE_STRING, .strKey = keyString};
        void* result = getFromMap(&tRenderer->characters, key);
        Character ch = *(Character*)result;

        float xpos = x + ch.bearing[0] * scale;
        float ypos = y + (hCh.bearing[1] - ch.bearing[1]) * scale;

        float w = ch.size[0] * scale;
        float h = ch.size[1] * scale;

        // update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 0.0f},
            {xpos, ypos, 0.0f, 0.0f},

            {xpos, ypos + h, 0.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 0.0f},
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
