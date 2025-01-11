#include "post_processing.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

#include "shader.h"
#include "texture.h"

static unsigned int MSFBO, FBO;
static unsigned int RBO;
static unsigned int VAO;

static void initRenderData() {
    unsigned int VBO;
    float vertices[] = {
        // pos        // tex
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

PostProcessor* NewPostProcessor(Shader shader, unsigned int width, unsigned int height) {
    PostProcessor* process = malloc(sizeof(PostProcessor));
    process->postProcessingShader = shader;
    process->texture = NewTexture();
    process->width = width;
    process->height = height;
    process->confuse = false;
    process->chaos = false;
    process->shake = false;

    glGenFramebuffers(1, &MSFBO);
    glGenFramebuffers(1, &FBO);
    glGenRenderbuffers(1, &RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    GLint max_samples;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, max_samples, GL_RGB, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Error: Failed to initialize MSFBO\n");
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    GenerateTexture(process->texture, width, height, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, process->texture->ID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Error: Failed to initialize FBO\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initRenderData();
    setInteger(process->postProcessingShader, "scene", 0, true);
    float offset = 1.0f / 300.0f;
    float offsets[9][2] = {
        {-offset, offset},   // top-left
        {0.0f, offset},      // top-center
        {offset, offset},    // top-right
        {-offset, 0.0f},     // center-left
        {0.0f, 0.0f},        // center-center
        {offset, 0.0f},      // center - right
        {-offset, -offset},  // bottom-left
        {0.0f, -offset},     // bottom-center
        {offset, -offset}    // bottom-right
    };
    glUniform2fv(glGetUniformLocation(process->postProcessingShader, "offsets"), 9, (float*)offsets);
    int edge_kernel[9] = {
        -1, -1, -1,
        -1, 8, -1,
        -1, -1, -1};
    glUniform1iv(glGetUniformLocation(process->postProcessingShader, "edge_kernel"), 9, edge_kernel);
    float blur_kernel[9] = {
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
        2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f};
    glUniform1fv(glGetUniformLocation(process->postProcessingShader, "blur_kernel"), 9, blur_kernel);
    return process;
}

void BeginPostProcessRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void EndPostProcessRender(PostProcessor* process) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, MSFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glBlitFramebuffer(0, 0, process->width, process->height, 0, 0, process->width, process->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPostProcess(PostProcessor* process, float time) {
    UseShader(process->postProcessingShader);
    setFloat(process->postProcessingShader, "time", time, false);
    setInteger(process->postProcessingShader, "confuse", process->confuse, false);
    setInteger(process->postProcessingShader, "chaos", process->chaos, false);
    setInteger(process->postProcessingShader, "shake", process->shake, false);
    glActiveTexture(GL_TEXTURE0);
    BindTexture(process->texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void CleanupPostProcess(PostProcessor* process) {
    free(process->texture);
    free(process);
}
