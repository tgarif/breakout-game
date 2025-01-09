#ifndef SHADER_H_
#define SHADER_H_

#include "mathc.h"

typedef unsigned int Shader;

Shader NewShader(const char* vertexSource, const char* fragmentSource, const char* geometrySource);
void UseShader(Shader shaderID);
void setFloat(Shader shaderID, const char* name, float value, bool useShader);
void setInteger(Shader shaderID, const char* name, int value, bool useShader);
void setVec2f(Shader shaderID, const char* name, float x, float y, bool useShader);
void setVec2fv(Shader shaderID, const char* name, mfloat_t* value, bool useShader);
void setVec3f(Shader shaderID, const char* name, float x, float y, float z, bool useShader);
void setVec3fv(Shader shaderID, const char* name, mfloat_t* value, bool useShader);
void setVec4f(Shader shaderID, const char* name, float x, float y, float z, float w, bool useShader);
void setVec4fv(Shader shaderID, const char* name, mfloat_t* value, bool useShader);
void setMat4fv(Shader shaderID, const char* name, mfloat_t* matrix, bool useShader);

#endif
