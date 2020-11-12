#include "gl_helper/framebuffer.h"
#include "gl_helper/shader.h"
#include "gl_helper/texture.h"
#include "polygons/polygon.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "SDL.h"
#include "GL/glew.h"
#include "SDL_opengl.h"

#include <iostream>

const GLint WIDTH = 512, HEIGHT = 512;

const uint NUM_VERTICES = 5;
const uint NUM_CELLS_X = 4;
const uint NUM_CELLS_Y = 4;
const uint NUM_POLYGONS = NUM_CELLS_X * NUM_CELLS_Y;

// OpenGL debugging info 
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const GLchar* message,
        const void* userParam)
{
    fprintf(stderr,
            "GL CALLBACK: %s type = 0x%x, severity = 0x%xm nessage = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            severity, message);
}

int main(int argc, char *argv[])
{
    //=========================================================================
    // Initialize SDL2, GLEW, and OpenGL.
    //=========================================================================

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cerr << "SDL INIT ERROR: " << SDL_GetError() << std::endl;
    }

    SDL_Window *window = SDL_CreateWindow("PolyFy", 100, 100, WIDTH, HEIGHT,
            SDL_WINDOW_OPENGL);

    if (window == NULL)
    {
        std::cerr << "SDL WINDOW ERROR: " << SDL_GetError() << std::endl;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (context == NULL)
    {
        std::cerr << "SDL GL CONTEXT ERROR: " << SDL_GetError() << std::endl;
    }
 
    glewExperimental = GL_TRUE;

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        std::cerr << "GLEW ERROR: " << glewGetErrorString(glew_status)
            << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    // Enable custom clipping to clip polygons to their cells.
    glEnable(GL_CLIP_DISTANCE0);
    glEnable(GL_CLIP_DISTANCE1);
    glEnable(GL_CLIP_DISTANCE2);
    glEnable(GL_CLIP_DISTANCE3);

    //=========================================================================
    // Setup common texture rendering components.
    //=========================================================================

    std::vector<ShaderSource> quad_rend_source = {
        ShaderSource(GL_VERTEX_SHADER, "shaders/quad.vert"),
        ShaderSource(GL_FRAGMENT_SHADER, "shaders/quad.frag")
    };

    Shader quad_rend(quad_rend_source);

    std::array<GLfloat, 16> quad_vertices = {
        //  x,     y,      u,    v
        -1.0f,  1.0f,   0.0f, 1.0f, // top left
        -1.0f, -1.0f,   0.0f, 0.0f, // bottom left
         1.0f, -1.0f,   1.0f, 0.0f, // bottom right
         1.0f,  1.0f,   1.0f, 1.0f  // top right
    };

    std::array<GLuint, 6> quad_elements = {
        1, 3, 0,
        1, 2, 3
    };

    GLuint quad_vao;
    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    GLuint quad_vbo;
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quad_vertices.size(),
            &quad_vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
            (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
            (GLvoid*) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    GLuint quad_ebo;
    glGenBuffers(1, &quad_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)
            * quad_elements.size(), &quad_elements.front(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //=========================================================================
    // Setup target texture.
    //=========================================================================

    // Create a texture to hold a tiled version of the target image.
    GLuint target_texture;
    create_blank_texture(target_texture, WIDTH, HEIGHT);

    // Create a framebuffer to render to the target texture.
    GLuint target_fbo;
    create_framebuffer(target_fbo, target_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);

    // Create a temporary texture to hold the target image.
    GLuint raw_target_texture;
    create_empty_texture(raw_target_texture);
    glBindTexture(GL_TEXTURE_2D, raw_target_texture);

    // Load the target image into the texture.
    int w, h, n;
    unsigned char* image = stbi_load("test.png", &w, &h, &n, STBI_rgb_alpha);
    if (image != NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, image);
        stbi_image_free(image);
    }
    else
    {
        std::cerr << "STB IMAGE LOAD ERROR" << std::endl;
    }

    std::array<GLfloat, 16> target_vertices = {
        //  x,     y,             u,           v
        -1.0f,  1.0f,          0.0f,        0.0f, // tl
        -1.0f, -1.0f,          0.0f, NUM_CELLS_Y, // bl
         1.0f, -1.0f,   NUM_CELLS_X, NUM_CELLS_Y, // br
         1.0f,  1.0f,   NUM_CELLS_X,        0.0f // tr
    };

    GLuint target_vao;
    glGenVertexArrays(1, &target_vao);
    glBindVertexArray(target_vao);

    GLuint target_vbo;
    glGenBuffers(1, &target_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, target_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * target_vertices.size(),
            &target_vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
            (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
            (GLvoid*) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);

    quad_rend.use();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    quad_rend.unuse();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteTextures(1, &raw_target_texture);

    glDeleteFramebuffers(1, &target_fbo);
    glDeleteVertexArrays(1, &target_vao);
    glDeleteBuffers(1, &target_vbo);

    //=========================================================================
    // Setup polygon textures.
    //=========================================================================

    // Setup texture to store finalized frames.
    GLuint base_texture;
    create_blank_texture(base_texture, WIDTH, HEIGHT);

    // Setup texture to store current frame.
    GLuint current_texture;
    create_blank_texture(current_texture, WIDTH, HEIGHT);

    // Setup framebuffer to render to current_texture
    GLuint current_fbo;
    create_framebuffer(current_fbo, current_texture);

    //=========================================================================
    // Initialize polygon population and buffers.
    //=========================================================================

    // Generate initial polygons.
    std::vector<Polygon> polygons(NUM_POLYGONS);
    std::vector<GLfloat> polygon_buffer_data;
    for (int i = 0; i < NUM_POLYGONS; i++)
    {
        polygons[i].pushTo(polygon_buffer_data);
    }

    // Setup polygon compute shader.
    std::vector<ShaderSource> polygon_comp_source = {
        ShaderSource(GL_COMPUTE_SHADER, "shaders/polygon.comp")
    };
    Shader polygon_comp(polygon_comp_source);
    
    // Setup polygon compute shader buffer.
    GLuint polygon_buffer;
    glGenBuffers(1, &polygon_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, polygon_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
            sizeof(GLfloat) * polygon_buffer_data.size(),
            &polygon_buffer_data.front(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, polygon_buffer);

    // Setup polygon rendering shader.
    std::vector<ShaderSource> polygon_rend_source = {
        ShaderSource(GL_VERTEX_SHADER, "shaders/polygon.vert"),
        ShaderSource(GL_FRAGMENT_SHADER, "shaders/polygon.frag")
    };
    Shader polygon_rend(polygon_rend_source);

    // Create polygon vbo and ibo.
    GLuint polygon_vbo;
    glGenBuffers(1, &polygon_vbo);

    GLuint polygon_ibo;
    glGenBuffers(1, &polygon_ibo);

    // Setup polygon vbo and ibo for compute shader writing.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, polygon_vbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
            sizeof(GLfloat) * (NUM_VERTICES + 1) * 8 * NUM_POLYGONS, NULL,
            GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, polygon_vbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, polygon_ibo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
            sizeof(GLuint) * NUM_VERTICES * 3 * NUM_POLYGONS, NULL,
            GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, polygon_ibo);

    // Setup polygon vbo and ibo for render reading.
    GLuint polygon_vao;
    glGenVertexArrays(1, &polygon_vao);
    glBindVertexArray(polygon_vao);

    glBindBuffer(GL_ARRAY_BUFFER, polygon_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid*) (4 * sizeof(GLfloat))); 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
            (GLvoid*) (6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygon_ibo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //=========================================================================
    // Render loop. 
    //=========================================================================

    SDL_Event windowEvent;
    while (true)
    {
        if (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
            {
                break;
            }
            else if (windowEvent.type == SDL_KEYDOWN)
            {
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE)
                {
                    break;
                }
            }
        }

        // Generate polygon vertex and index arrays.
        polygon_comp.use();
        glDispatchCompute(1, 1, 1);
        polygon_comp.unuse();
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
                | GL_ELEMENT_ARRAY_BARRIER_BIT);

        // Render base_texture into current_texture.
        glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
        glBindTexture(GL_TEXTURE_2D, base_texture);

        quad_rend.use();
        glBindVertexArray(quad_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);
        glBindVertexArray(0);
        quad_rend.unuse();

        glBindTexture(GL_TEXTURE_2D, 0);

        // Render polygons onto current_texture.
        polygon_rend.use();
        glBindVertexArray(polygon_vao);
        glDrawElements(GL_TRIANGLES,
                NUM_VERTICES * 3 * NUM_POLYGONS, GL_UNSIGNED_INT,
                (GLvoid*) 0);
        glBindVertexArray(0);
        polygon_rend.unuse();

        // Render current_texture to the screen.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, current_texture);

        quad_rend.use();
        glBindVertexArray(quad_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);
        glBindVertexArray(0);
        quad_rend.unuse();

        glBindTexture(GL_TEXTURE_2D, 0);

        // Display results.
        SDL_GL_SwapWindow(window);
    }

    //=========================================================================
    // Cleanup.
    //=========================================================================

    glDeleteBuffers(1, &polygon_buffer);
    glDeleteBuffers(1, &polygon_vbo);
    glDeleteBuffers(1, &polygon_ibo);
    glDeleteVertexArrays(1, &polygon_vao);

    glDeleteTextures(1, &base_texture);
    glDeleteTextures(1, &current_texture);

    glDeleteProgram(polygon_comp.handle);
    glDeleteProgram(polygon_rend.handle);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
