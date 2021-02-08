#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "glad/glad.h"

void create_index_buffer(GLuint& handle, int size, GLuint* first, GLenum mode)
{
    glCreateBuffers(1, &handle);
    glNamedBufferData(handle, size * sizeof(GLuint), first, mode);
}

#endif
