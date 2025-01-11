#ifndef POST_PROCESSING_H_
#define POST_PROCESSING_H_

#include "shader.h"
#include "texture.h"

typedef struct {
    Shader postProcessingShader;
    Texture2D* texture;
    unsigned int width, height;
    bool confuse, chaos, shake;
} PostProcessor;

PostProcessor* NewPostProcessor(Shader shader, unsigned int width, unsigned int height);
void BeginPostProcessRender();
void EndPostProcessRender(PostProcessor* process);
void RenderPostProcess(PostProcessor* process, float time);
void CleanupPostProcess(PostProcessor* process);

#endif
