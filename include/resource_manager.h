#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "shader.h"
#include "texture.h"
#include "util.h"

typedef struct {
    DynamicMap shaders;
    DynamicMap textures;
} ResourceManager;

Shader LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, char* name);
Shader GetShader(char* name);
Texture2D* LoadTexture(const char* file, bool alpha, char* name);
Texture2D* GetTexture(char* name);
void ClearResources();

#endif
