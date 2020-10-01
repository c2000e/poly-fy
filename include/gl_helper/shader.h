#ifndef SHADER_H
#define SHADER_H

#include "GL/glew.h"

#include <vector>

struct ShaderSource
{
    GLuint handle;
    GLuint type;
    const char* path;

    ShaderSource(GLuint type, const char* path);
};

struct Shader
{
    GLuint handle;

    // Creates a Shader from a container of ShaderSources.
    Shader(std::vector<ShaderSource>& shaderSources);

    void use();
    void unuse();
};

#endif
