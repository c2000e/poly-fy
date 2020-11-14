#ifndef STORAGE_BUFFER_H
#define STORAGE_BUFFER_H

#include "GL/glew.h"

void create_storage_buffer(GLuint& handle, int size, GLfloat* first,
        GLenum mode) 
{
    glCreateBuffers(1, &handle);
    glNamedBufferData(handle, size * sizeof(GLfloat), first, mode);
}

#endif
