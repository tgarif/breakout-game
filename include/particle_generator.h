#ifndef PARTICLE_GENERATOR_H_
#define PARTICLE_GENERATOR_H_

#include "ball_object.h"
#include "mathc.h"
#include "shader.h"
#include "texture.h"

typedef struct {
    mfloat_t position[VEC2_SIZE], velocity[VEC2_SIZE];
    mfloat_t color[VEC4_SIZE];
    float life;
} Particle;

Particle* NewParticle();
void NewParticleGenerator(Shader shader, Texture2D* texture, unsigned int amount);
void UpdateParticle(float dt, BallObject* ball, unsigned int newParticles, mfloat_t* offset);
void DrawParticle();
void CleanupParticles();

#endif
