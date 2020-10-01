#include "gl_helper/shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

ShaderSource::ShaderSource(GLuint type, const char* path) : type(type),
    path(path) {}

Shader::Shader(std::vector<ShaderSource>& shaderSources)
{ 
    handle = glCreateProgram();

    GLint success;
    GLchar infoLog[512];

    // Load and compile each shader source file.
    for (ShaderSource& shaderSource : shaderSources)
    {
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::badbit);

        std::stringstream shaderStream;

        // Read shader from file.
        try
        {
            shaderFile.open(shaderSource.path);
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
        }
        catch (std::ifstream::failure e)
        {
            std::cerr << "ERROR: " << shaderSource.path
                << " NOT SUCCESFULLY READ.\n" << infoLog << std::endl;
        }

        std::string shaderCodeString = shaderStream.str();
        const GLchar* shaderCode = shaderCodeString.c_str();

        // Compile shader source.
        shaderSource.handle = glCreateShader(shaderSource.type);
        glShaderSource(shaderSource.handle, 1, &shaderCode, NULL);
        glCompileShader(shaderSource.handle);

        // Make sure it compiled correctly.
        glGetShaderiv(shaderSource.handle, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderSource.handle, 512, NULL, infoLog);
            std::cerr << "ERROR: " << shaderSource.path <<
                " COMPILATION FAILED.\n" << infoLog << std::endl;
        }
        
        // Add it to shader program.
        glAttachShader(handle, shaderSource.handle);
    }

    // Link shader program together.
    glLinkProgram(handle);

    // Make sure it linked correctly.
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(handle, 512, NULL, infoLog);
        std::cerr << "ERROR: SHADER PROGRAM LINKING FAILED.\n" << infoLog
            << std::endl;
    }

    // Delete shader sources.
    for (ShaderSource& shaderSource : shaderSources)
    {
        glDetachShader(handle, shaderSource.handle);
        glDeleteShader(shaderSource.handle);
    }
} 

void Shader::use()
{
    glUseProgram(handle);
}

void Shader::unuse()
{
    glUseProgram(0);
}
