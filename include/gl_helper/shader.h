#ifndef SHADER_H
#define SHADER_H

#include "GL/glew.h"

#include <fstream>
#include <iostream>
#include <sstream>

void load_shader_source(GLuint& handle, GLuint type, const char* path)
{
    std::ifstream shader_file;
    shader_file.exceptions(std::ifstream::badbit);

    std::stringstream shader_stream;

    try
    {
        shader_file.open(path);
        shader_stream << shader_file.rdbuf();
        shader_file.close();
    }
    catch (std::ifstream::failure e)
    {
        std::cerr << "ERROR: UNABLE TO READ " << path << std::endl;
    }

    std::string shader_string = shader_stream.str();
    const GLchar* shader_cstring = shader_string.c_str();

    handle = glCreateShader(type);
    glShaderSource(handle, 1, &shader_cstring, NULL);
    glCompileShader(handle);

    GLint success;
    GLchar info_log[512];
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(handle, 512, NULL, info_log);
        std::cerr << "ERROR: FAILED TO COMPILE " << path << std::endl;
        std::cerr << info_log << std::endl;
    }
}

void link_shader_program(const GLuint& handle)
{
    glLinkProgram(handle);

    GLint success;
    GLchar info_log[512];
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(handle, 512, NULL, info_log);
        std::cerr << "ERROR: FAILED TO LINK SHADER PROGRAM." << std::endl; 
        std::cerr << info_log << std::endl;
    }
}

void create_rend_shader(GLuint& handle, const char* vert_path,
        const char* frag_path)
{
    handle = glCreateProgram();

    GLuint vert_shader;
    load_shader_source(vert_shader, GL_VERTEX_SHADER, vert_path);
    glAttachShader(handle, vert_shader);

    GLuint frag_shader;
    load_shader_source(frag_shader, GL_FRAGMENT_SHADER, frag_path);
    glAttachShader(handle, frag_shader);

    link_shader_program(handle);

    glDetachShader(handle, vert_shader);
    glDeleteShader(vert_shader);
    
    glDetachShader(handle, frag_shader);
    glDeleteShader(frag_shader);
}

void create_comp_shader(GLuint& handle, const char* path)
{
    handle = glCreateProgram();

    GLuint comp_shader;
    load_shader_source(comp_shader, GL_COMPUTE_SHADER, path);
    glAttachShader(handle, comp_shader);

    link_shader_program(handle);

    glDetachShader(handle, comp_shader);
    glDeleteShader(comp_shader);
}

#endif
