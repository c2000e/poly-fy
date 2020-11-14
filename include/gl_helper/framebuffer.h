#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "GL/glew.h"

#include <iostream>

void create_framebuffer(GLuint& handle, const GLuint& texture)
{
    glCreateFramebuffers(1, &handle);
    glNamedFramebufferTexture(handle, GL_COLOR_ATTACHMENT0, texture, 0);

    if (glCheckNamedFramebufferStatus(handle, GL_FRAMEBUFFER)
            != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "GL FRAMEBUFFER ERROR: "
            << glCheckNamedFramebufferStatus(handle, GL_FRAMEBUFFER)
            << std::endl;
    }
}

#endif
