#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "GL/glew.h"

template<class Iter>
void set_vertex_array_layout(const GLuint& handle, const GLuint& vertex_buffer,
        Iter begin, Iter end)
{
    // Calculate the total size (in bytes) of a vertex.
    int size = 0;
    for (Iter curr = begin; curr < end; curr++)
    {
        size += *curr;
    }
    size *= sizeof(GLfloat);

    // Bind a vertex buffer to the vertex array.
    glVertexArrayVertexBuffer(handle, 0, vertex_buffer, 0, size);

    // Setup vertex attributes.
    int i = 0;
    int offset = 0;
    for (Iter curr = begin; curr < end; curr++)
    {
        glEnableVertexArrayAttrib(handle, i);

        glVertexArrayAttribFormat(handle, i, *curr, GL_FLOAT, GL_FALSE,
                offset);

        glVertexArrayAttribBinding(handle, i, 0);

        i++;
        offset += *curr * sizeof(GLfloat);
    }
}

template<class Iter>
void create_vertex_array(GLuint& handle, const GLuint vertex_buffer,
        const GLuint index_buffer, Iter begin, Iter end)
{
    glCreateVertexArrays(1, &handle);

    set_vertex_array_layout(handle, vertex_buffer, begin, end);

    glVertexArrayElementBuffer(handle, index_buffer);
}

#endif
