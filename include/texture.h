#ifndef TEXTURE_H_
#define TEXTURE_H_

typedef struct {
    unsigned int ID;
    unsigned int width, height;
    unsigned int internalFormat;
    unsigned int imageFormat;
    unsigned int wrapS;
    unsigned int wrapT;
    unsigned int filterMin;
    unsigned int filterMax;
} Texture2D;

Texture2D* NewTexture();
void GenerateTexture(Texture2D* texture, unsigned int width, unsigned int height, unsigned char* data);
void BindTexture(Texture2D* texture);

#endif
