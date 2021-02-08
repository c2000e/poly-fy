#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad/glad.h"

void create_texture(GLuint& handle, int width, int height)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(handle, 1, GL_RGBA32F, width, height);
}

#endif
