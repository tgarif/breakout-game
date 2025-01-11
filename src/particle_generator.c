#include "particle_generator.h"

#include <GL/glew.h>
#include <stddef.h>
#include <stdlib.h>

#include "mathc.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

static DynamicArray particles;
static unsigned int amount;
static Shader shader;
static Texture2D* texture;
static unsigned int VAO;

static void init() {
    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f  // #
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    for (size_t i = 0; i < amount; ++i) {
        pushPtr(&particles, NewParticle());
    }
}

static unsigned int lastUsedParticle = 0;
static unsigned int firstUnusedParticle() {
    for (unsigned int i = lastUsedParticle; i < amount; ++i) {
        Particle* particle = ((Particle**)(particles.array))[i];
        if (particle->life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        Particle* particle = ((Particle**)(particles.array))[i];
        if (particle->life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    lastUsedParticle = 0;
    return 0;
}

static void respawnParticle(Particle* particle, BallObject* ball, mfloat_t* offset) {
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    vec2_add(particle->position, ball->base.position, vec2_add_f(offset, offset, random));
    particle->color[0] = rColor;
    particle->color[1] = rColor;
    particle->color[2] = rColor;
    particle->color[3] = 1.0f;
    particle->life = 1.0f;
    vec2_multiply_f(particle->velocity, ball->base.velocity, 0.1f);
}

static void cleanupParticlesCallback(void* item) {
    Particle* particle = *(Particle**)item;
    free(particle);
}

Particle* NewParticle() {
    Particle* p = malloc(sizeof(Particle));
    p->position[0] = 0.0f;
    p->position[1] = 0.0f;
    p->velocity[0] = 0.0f;
    p->velocity[1] = 0.0f;
    p->color[0] = 1.0f;
    p->color[1] = 1.0f;
    p->color[2] = 1.0f;
    p->color[3] = 1.0f;
    p->life = 0.0f;
    return p;
}

void NewParticleGenerator(Shader s, Texture2D* t, unsigned int a) {
    shader = s;
    texture = t;
    amount = a;
    initialize(&particles, 256, sizeof(Particle*));
    init();
}

void UpdateParticle(float dt, BallObject* ball, unsigned int newParticles, mfloat_t* offset) {
    for (size_t i = 0; i < newParticles; ++i) {
        int unusedParticle = firstUnusedParticle();
        Particle* particle = ((Particle**)(particles.array))[unusedParticle];
        respawnParticle(particle, ball, offset);
    }
    for (size_t i = 0; i < amount; ++i) {
        Particle* p = ((Particle**)(particles.array))[i];
        p->life -= dt;
        if (p->life > 0.0f) {
            mfloat_t multiply[VEC2_SIZE];
            vec2_subtract(p->position, p->position, vec2_multiply_f(multiply, p->velocity, dt));
            p->color[3] -= dt * 2.5f;
        }
    }
}

void DrawParticle() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    UseShader(shader);
    DYNAMIC_ARRAY_FOR_EACH_PTR(&particles, Particle, p) {
        if ((*p)->life > 0.0f) {
            setVec2fv(shader, "offset", (*p)->position, false);
            setVec4fv(shader, "color", (*p)->color, false);
            BindTexture(texture);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CleanupParticles() {
    cleanup(&particles, cleanupParticlesCallback);
}
