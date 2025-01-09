#include "shader.h"

#include <GL/glew.h>
#include <stdio.h>
#include <string.h>

static void checkShaderCompileErrors(unsigned int object, const char* type) {
    int success;
    char infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::SHADER: Compile-time error: Type: %s\n", type);
        }
    } else {
        glGetShaderiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            fprintf(stderr, "ERROR::SHADER: Link-time error: Type: %s\n", type);
        }
    }
}

Shader NewShader(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
    unsigned int sVertex, sFragment, gShader;

    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource, NULL);
    glCompileShader(sVertex);
    checkShaderCompileErrors(sVertex, "VERTEX");

    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource, NULL);
    glCompileShader(sFragment);
    checkShaderCompileErrors(sFragment, "FRAGMENT");

    if (geometrySource != NULL) {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &geometrySource, NULL);
        glCompileShader(gShader);
        checkShaderCompileErrors(gShader, "GEOMETRY");
    }

    Shader shaderID = glCreateProgram();
    glAttachShader(shaderID, sVertex);
    glAttachShader(shaderID, sFragment);
    if (geometrySource != NULL)
        glAttachShader(shaderID, gShader);
    glLinkProgram(shaderID);
    checkShaderCompileErrors(shaderID, "PROGRAM");

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (geometrySource != NULL)
        glDeleteShader(gShader);

    return shaderID;
}

void UseShader(Shader shaderID) {
    glUseProgram(shaderID);
}

void setFloat(Shader shaderID, const char* name, float value, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform1f(glGetUniformLocation(shaderID, name), value);
}
void setInteger(Shader shaderID, const char* name, int value, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, name), value);
}
void setVec2f(Shader shaderID, const char* name, float x, float y, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform2f(glGetUniformLocation(shaderID, name), x, y);
}
void setVec2fv(Shader shaderID, const char* name, mfloat_t* value, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform2fv(glGetUniformLocation(shaderID, name), 1, value);
}
void setVec3f(Shader shaderID, const char* name, float x, float y, float z, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform3f(glGetUniformLocation(shaderID, name), x, y, z);
}
void setVec3fv(Shader shaderID, const char* name, mfloat_t* value, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform3fv(glGetUniformLocation(shaderID, name), 1, value);
}
void setVec4f(Shader shaderID, const char* name, float x, float y, float z, float w, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform4f(glGetUniformLocation(shaderID, name), x, y, z, w);
}
void setVec4fv(Shader shaderID, const char* name, mfloat_t* value, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniform4fv(glGetUniformLocation(shaderID, name), 1, value);
}
void setMat4fv(Shader shaderID, const char* name, mfloat_t* matrix, bool useShader) {
    if (useShader)
        UseShader(shaderID);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, name), 1, GL_FALSE, matrix);
}
