#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "glad/glad.h"

void create_vertex_buffer(GLuint& handle, int size, GLfloat* first,
        GLenum mode)
{
    glCreateBuffers(1, &handle);
    glNamedBufferData(handle, size * sizeof(GLfloat), first, mode);
}

#endif
