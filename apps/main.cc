#include "gl_helper/shader.h"
#include "polygons/polygon.h"

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
    // Initialize SDL2 and GLEW for OpenGL-based rendering.
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

    glEnable(GL_CLIP_DISTANCE0);
    glEnable(GL_CLIP_DISTANCE1);
    glEnable(GL_CLIP_DISTANCE2);
    glEnable(GL_CLIP_DISTANCE3);

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

    // Render loop starts here.
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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        polygon_comp.use();
        glDispatchCompute(1, 1, 1);
        polygon_comp.unuse();

        polygon_rend.use();
        glBindVertexArray(polygon_vao);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
                | GL_ELEMENT_ARRAY_BARRIER_BIT);
        glDrawElements(GL_TRIANGLES,
                NUM_VERTICES * 3 * NUM_POLYGONS, GL_UNSIGNED_INT,
                (GLvoid*) 0);
        glBindVertexArray(0);
        polygon_rend.unuse();

        SDL_GL_SwapWindow(window);
    }

    glDeleteBuffers(1, &polygon_buffer);
    glDeleteBuffers(1, &polygon_vbo);
    glDeleteBuffers(1, &polygon_ibo);
    glDeleteVertexArrays(1, &polygon_vao);

    glDeleteProgram(polygon_comp.handle);
    glDeleteProgram(polygon_rend.handle);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
