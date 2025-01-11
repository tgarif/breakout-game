#include "texture.h"

#include <GL/glew.h>
#include <stdlib.h>

Texture2D* NewTexture() {
    Texture2D* texture = malloc(sizeof(Texture2D));
    texture->width = 0;
    texture->height = 0;
    texture->internalFormat = GL_RGB;
    texture->imageFormat = GL_RGB;
    texture->wrapS = GL_REPEAT;
    texture->wrapT = GL_REPEAT;
    texture->filterMin = GL_LINEAR;
    texture->filterMax = GL_LINEAR;

    glGenTextures(1, &texture->ID);
    return texture;
}

void GenerateTexture(Texture2D* texture, unsigned int width, unsigned int height, unsigned char* data) {
    texture->width = width;
    texture->height = height;

    glBindTexture(GL_TEXTURE_2D, texture->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, texture->internalFormat, width, height, 0, texture->imageFormat, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->filterMax);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void BindTexture(Texture2D* texture) {
    glBindTexture(GL_TEXTURE_2D, texture->ID);
}
