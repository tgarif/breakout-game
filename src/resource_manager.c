#include "resource_manager.h"

#include <GL/glew.h>

#include "shader.h"
#include "stb_image.h"
#include "texture.h"
#include "util.h"

static ResourceManager instance;
static uint8_t isInitialized = 0;

static void initializeResourceManager() {
    if (!isInitialized) {
        initMap(&instance.shaders);
        initMap(&instance.textures);
        isInitialized = 1;
    }
}

static Shader loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    char* vShaderCode = readFile(vShaderFile);
    char* fShaderCode = readFile(fShaderFile);
    char* gShaderCode = NULL;

    if (gShaderFile) {
        gShaderCode = readFile(gShaderFile);
    }

    Shader shader = NewShader(vShaderCode, fShaderCode, gShaderFile != NULL ? gShaderCode : NULL);

    free(vShaderCode);
    free(fShaderCode);
    free(gShaderCode);

    return shader;
}

static void addShader(Key key, Shader shader) {
    if (!isInitialized)
        initializeResourceManager();

    Shader* shaderPtr = malloc(sizeof(Shader));
    *shaderPtr = shader;
    insertIntoMap(&instance.shaders, key, shaderPtr);
}

static Shader getFromShader(Key key) {
    if (!isInitialized)
        initializeResourceManager();

    void* result = getFromMap(&instance.shaders, key);

    if (!result) {
        fprintf(stderr, "Error: Shader not found for the given key\n");
        return (Shader){0};
    }

    return *(Shader*)result;
}

static Texture2D* loadTextureFromFile(const char* file, bool alpha) {
    Texture2D* texture = NewTexture();
    if (alpha) {
        texture->internalFormat = GL_RGBA;
        texture->imageFormat = GL_RGBA;
    }

    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

    GenerateTexture(texture, width, height, data);

    stbi_image_free(data);
    return texture;
}

static void addTexture(Key key, Texture2D* texture) {
    if (!isInitialized)
        initializeResourceManager();

    insertIntoMap(&instance.textures, key, texture);
}

static Texture2D* getFromTexture(Key key) {
    if (!isInitialized)
        initializeResourceManager();

    void* result = getFromMap(&instance.textures, key);

    if (!result) {
        fprintf(stderr, "Error: Texture not found for the given key\n");
        return NULL;
    }

    return (Texture2D*)result;
}

static void clearShaders(Key key __attribute__((unused)), void* value, void* context __attribute__((unused))) {
    glDeleteProgram(*(Shader*)value);
}

static void clearTextures(Key key __attribute__((unused)), void* value, void* context __attribute__((unused))) {
    glDeleteTextures(1, &((Texture2D*)value)->ID);
}

Shader LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, char* name) {
    Key key = {.type = KEY_TYPE_STRING, .strKey = name};
    addShader(key, loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile));
    return getFromShader(key);
}

Shader GetShader(char* name) {
    Key key = {.type = KEY_TYPE_STRING, .strKey = name};
    return getFromShader(key);
}

Texture2D* LoadTexture(const char* file, bool alpha, char* name) {
    Key key = {.type = KEY_TYPE_STRING, .strKey = name};
    addTexture(key, loadTextureFromFile(file, alpha));
    return getFromTexture(key);
}

Texture2D* GetTexture(char* name) {
    Key key = {.type = KEY_TYPE_STRING, .strKey = name};
    return getFromTexture(key);
}

void ClearResources() {
    if (!isInitialized)
        initializeResourceManager();

    traverseInOrder(instance.shaders.root, instance.shaders.nil, clearShaders, NULL);
    freeMap(&instance.shaders);

    traverseInOrder(instance.textures.root, instance.textures.nil, clearTextures, NULL);
    freeMap(&instance.textures);

    isInitialized = 0;
}
