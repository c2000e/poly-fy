#include "gl_helper/framebuffer.h"
#include "gl_helper/index_buffer.h"
#include "gl_helper/shader.h"
#include "gl_helper/storage_buffer.h"
#include "gl_helper/texture.h"
#include "gl_helper/vertex_array.h"
#include "gl_helper/vertex_buffer.h"
#include "polygons/polygon.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "SDL.h"
#include "GL/glew.h"
#include "SDL_opengl.h"

#include <algorithm>
#include <iostream>

const GLint WIDTH = 512, HEIGHT = 512;

const uint NUM_VERTICES = 5;
const uint NUM_CELLS_X = 4;
const uint NUM_CELLS_Y = 4;
const uint NUM_POLYGONS = NUM_CELLS_X * NUM_CELLS_Y;

const uint NUM_ELITE = 2;
const uint MAX_STAGNATE = 512;

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
    
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(MessageCallback, 0);

    // Enable custom clipping to clip polygons to their cells.
    glEnable(GL_CLIP_DISTANCE0);
    glEnable(GL_CLIP_DISTANCE1);
    glEnable(GL_CLIP_DISTANCE2);
    glEnable(GL_CLIP_DISTANCE3);

    //=========================================================================
    // Setup common texture rendering components.
    //=========================================================================

    GLuint quad_rend;
    create_rend_shader(quad_rend, "shaders/quad.vert", "shaders/quad.frag");

    std::array<GLfloat, 16> quad_verts = {
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

    GLuint quad_vb;
    create_vertex_buffer(quad_vb, quad_verts.size(), &quad_verts.front(),
            GL_STATIC_DRAW);

    GLuint quad_ib;
    create_index_buffer(quad_ib, quad_elements.size(), &quad_elements.front(),
            GL_STATIC_DRAW);
    
    std::array<int, 2> quad_vert = {2, 2};

    GLuint quad_va;
    create_vertex_array(quad_va, quad_vb, quad_ib, quad_vert.begin(),
            quad_vert.end());

    //=========================================================================
    // Setup target texture.
    //=========================================================================

    // Create a texture to hold a tiled version of the target image.
    GLuint targ_tex;
    create_texture(targ_tex, WIDTH, HEIGHT);

    GLuint targ_fb;
    create_framebuffer(targ_fb, targ_tex);

    // Create a temporary texture to hold the target image.
    GLuint targ_tex_raw;
    create_texture(targ_tex_raw, WIDTH, HEIGHT);

    // Load the target image into the texture.
    int w, h, n;
    unsigned char* image = stbi_load("test.png", &w, &h, &n, STBI_rgb_alpha);
    if (image != NULL)
    {
        glTextureSubImage2D(targ_tex_raw, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA,
                GL_UNSIGNED_BYTE, image);
        stbi_image_free(image);
    }
    else
    {
        std::cerr << "STB IMAGE LOAD ERROR" << std::endl;
    }

    std::array<GLfloat, 16> targ_verts = {
        //  x,     y,             u,           v
        -1.0f,  1.0f,          0.0f,        0.0f, // tl
        -1.0f, -1.0f,          0.0f, NUM_CELLS_Y, // bl
         1.0f, -1.0f,   NUM_CELLS_X, NUM_CELLS_Y, // br
         1.0f,  1.0f,   NUM_CELLS_X,        0.0f // tr
    };

    GLuint targ_vb;
    create_vertex_buffer(targ_vb, targ_verts.size(), &targ_verts.front(),
            GL_STATIC_DRAW);

    std::array<int, 2> targ_vert = {2, 2};

    GLuint targ_va;
    create_vertex_array(targ_va, targ_vb, quad_ib, targ_vert.begin(),
            targ_vert.end());

    glBindFramebuffer(GL_FRAMEBUFFER, targ_fb);
    glBindTextureUnit(0, targ_tex_raw);
    glBindVertexArray(targ_va);
    glUseProgram(quad_rend);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);

    glUseProgram(0);
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteVertexArrays(1, &targ_va);
    glDeleteBuffers(1, &targ_vb);
    glDeleteFramebuffers(1, &targ_fb);
    glDeleteTextures(1, &targ_tex_raw);
    
    //=========================================================================
    // Setup miscellaneous textures.
    //=========================================================================

    // Used to store image generated so far.
    GLuint base_tex;
    create_texture(base_tex, WIDTH, HEIGHT);

    GLuint base_fb;
    create_framebuffer(base_fb, base_tex);

    // Used to store the current generation's images.
    GLuint curr_tex;
    create_texture(curr_tex, WIDTH, HEIGHT);

    GLuint curr_fb;
    create_framebuffer(curr_fb, curr_tex);

    // Used to store the difference of the current generation from the target.
    GLuint diff_tex;
    create_texture(diff_tex, WIDTH, HEIGHT);
    
    GLuint diff_fb;
    create_framebuffer(diff_fb, diff_tex);

    GLuint diff_rend;
    create_rend_shader(diff_rend, "shaders/quad.vert", "shaders/diff.frag");

    // Used to store the average differences (per image).
    GLuint avrg_sb;
    create_storage_buffer(avrg_sb, NUM_POLYGONS, 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, avrg_sb);

    GLuint avrg_comp;
    create_comp_shader(avrg_comp, "shaders/avrg.comp");

    glUseProgram(avrg_comp);
    GLuint diff_tex_loc = glGetUniformLocation(avrg_comp, "diff_tex");
    glUniform1i(diff_tex_loc, 0);
    glUseProgram(0);

    //=========================================================================
    // Initialize polygon population and buffers.
    //=========================================================================

    // Generate initial polygons.
    std::vector<Polygon> poly(NUM_POLYGONS);
    std::vector<GLfloat> poly_data;
    for (int i = 0; i < NUM_POLYGONS; i++)
    {
        poly[i].pushTo(poly_data);
    }

    // Setup polygon compute shader.
    GLuint poly_comp;
    create_comp_shader(poly_comp, "shaders/poly.comp");
    
    // Setup polygon compute shader buffer.
    GLuint poly_sb;
    create_storage_buffer(poly_sb, poly_data.size(), &poly_data.front(),
            GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, poly_sb);

    // Setup polygon rendering shader.
    GLuint poly_rend;
    create_rend_shader(poly_rend, "shaders/poly.vert", "shaders/poly.frag");

    // Setup polygon vertex and index buffers.
    GLuint poly_vb;
    create_vertex_buffer(poly_vb, (NUM_VERTICES + 1) * 8 * NUM_POLYGONS,
            NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, poly_vb);

    GLuint poly_ib;
    create_index_buffer(poly_ib, NUM_VERTICES * 3 * NUM_POLYGONS, NULL,
            GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, poly_ib);

    std::array<int, 3> poly_vert = {4, 2, 2};

    GLuint poly_va;
    create_vertex_array(poly_va, poly_vb, poly_ib,
            poly_vert.begin(), poly_vert.end());

    //=========================================================================
    // Render loop. 
    //=========================================================================
    
    GLfloat max_fitness = 0;
    GLfloat last_fitness = 0;
    int num_stagnate = 0;

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
        glUseProgram(poly_comp);
        glDispatchCompute(1, 1, 1);
        glUseProgram(0);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
                | GL_ELEMENT_ARRAY_BARRIER_BIT);

        // Render base_texture into current_texture.
        glBindFramebuffer(GL_FRAMEBUFFER, curr_fb);
        glBindTextureUnit(0, base_tex);
        glBindVertexArray(quad_va);
        glUseProgram(quad_rend);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);

        glUseProgram(0);
        glBindVertexArray(0);
        glBindTextureUnit(0, 0);

        // Render polygons onto current_texture.
        glBindVertexArray(poly_va);
        glUseProgram(poly_rend);
        glDrawElements(GL_TRIANGLES,
                NUM_VERTICES * 3 * NUM_POLYGONS, GL_UNSIGNED_INT,
                (GLvoid*) 0);
        glUseProgram(0);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render difference_texture
        glBindFramebuffer(GL_FRAMEBUFFER, diff_fb);
        glBindTextureUnit(0, targ_tex);
        glBindTextureUnit(1, curr_tex);

        glBindVertexArray(quad_va);
        glUseProgram(diff_rend);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);
        glUseProgram(0);
        glBindVertexArray(0);

        glBindTextureUnit(1, 0);
        glBindTextureUnit(0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Compute average differences.
        glBindImageTexture(0, diff_tex, 0, GL_FALSE, 0, GL_READ_WRITE,
                GL_RGBA32F);

        glUseProgram(avrg_comp);
        glDispatchCompute(NUM_CELLS_X, NUM_CELLS_Y, 1);
        glUseProgram(0);

        GLfloat avrg_data[NUM_POLYGONS];
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, avrg_sb);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                sizeof(GLfloat) * NUM_POLYGONS, &avrg_data);

        // Generate next generation of polygons.
        poly_data.clear(); 

        for (int i = 0; i < NUM_POLYGONS; i++)
        {
            poly[i].fitness = pow(1 - avrg_data[i], 2);
        }
        std::sort(poly.begin(), poly.end(), [](Polygon a, Polygon b)
                { return a.fitness > b.fitness; });

        if (poly[0].fitness > max_fitness)
        {
            max_fitness = poly[0].fitness;
            num_stagnate = 0;
        }
        else
        {
            num_stagnate++;
        }

        std::vector<Polygon> poly_new(NUM_POLYGONS);
        if (num_stagnate < MAX_STAGNATE)
        {
            for (int i = 0; i < NUM_ELITE; i++)
            {
                poly_new[i] = poly[i];
                poly_new[i].pushTo(poly_data);
            }
            for (int i = NUM_ELITE; i < NUM_POLYGONS; i++)
            {
                poly_new[i] = Polygon(tournament(poly, 1, 0.75),
                        tournament(poly, 1, 0.75));
                poly_new[i].pushTo(poly_data);
            }
            poly = poly_new;
        }
        else
        {
            num_stagnate = 0;

            // Store the polygon if it improved the image.
            if (poly[0].fitness > last_fitness)
            {
                last_fitness = poly[0].fitness;

                for (int i = 0; i < NUM_POLYGONS; i++)
                {
                    poly[0].pushTo(poly_data);
                }
                glNamedBufferSubData(poly_sb, 0,
                        sizeof(GLfloat) * poly_data.size(),
                        &poly_data.front());

                // Generate polygon vertex and index arrays.
                glUseProgram(poly_comp);
                glDispatchCompute(1, 1, 1);
                glUseProgram(0);
                glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
                        | GL_ELEMENT_ARRAY_BARRIER_BIT);

                // Render polygons onto base_texture.
                glBindFramebuffer(GL_FRAMEBUFFER, base_fb);
                glBindVertexArray(poly_va);
                glUseProgram(poly_rend);

                glDrawElements(GL_TRIANGLES,
                        NUM_VERTICES * 3 * NUM_POLYGONS, GL_UNSIGNED_INT,
                        (GLvoid*) 0);
                
                glUseProgram(0);
                glBindVertexArray(0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // Generate new set of random polygons.
            for (int i = 0; i < NUM_POLYGONS; i++)
            {
                poly[i] = Polygon();
                poly[i].pushTo(poly_data);
            }
        }

        // Store new polygons for next pass.
        glNamedBufferSubData(poly_sb, 0, sizeof(GLfloat) * poly_data.size(),
                &poly_data.front());

        // Render current_texture to the screen.
        glBindTextureUnit(0, curr_tex);

        glBindVertexArray(quad_va);
        glUseProgram(quad_rend);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*) 0);
        glUseProgram(0);
        glBindVertexArray(0);

        glBindTextureUnit(0, 0);

        // Display results.
        SDL_GL_SwapWindow(window);
    }

    //=========================================================================
    // Cleanup.
    //=========================================================================

    glDeleteBuffers(1, &poly_sb);
    glDeleteBuffers(1, &poly_vb);
    glDeleteBuffers(1, &poly_ib);
    glDeleteVertexArrays(1, &poly_va);

    glDeleteTextures(1, &base_tex);
    glDeleteTextures(1, &curr_tex);
    glDeleteTextures(1, &targ_tex);
    glDeleteTextures(1, &diff_tex);

    glDeleteProgram(poly_comp);
    glDeleteProgram(poly_rend);
    glDeleteProgram(quad_rend);
    glDeleteProgram(avrg_comp);
    glDeleteProgram(diff_rend);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
