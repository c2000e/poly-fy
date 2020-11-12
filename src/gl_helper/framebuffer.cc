#include "gl_helper/framebuffer.h"

#include <iostream>

void create_framebuffer(GLuint& handle, GLuint texture)
{
    glGenFramebuffers(1, &handle);
    
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "GL FRAMEBUFFER ERROR: "
            << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
