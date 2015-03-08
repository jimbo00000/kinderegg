// sdl_main.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#undef main

#include "ShaderFunctions.h"
#include "Timer.h"

Timer g_timer;
int winw = 800;
int winh = 600;

struct Shadertoy {
    GLuint prog;
    GLint uloc_iResolution;
    GLint uloc_iGlobalTime;
};

Shadertoy g_toy;


void PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            break;

        case SDL_MOUSEMOTION:
            break;

        case SDL_MOUSEWHEEL:
            break;

        case SDL_WINDOWEVENT:
            break;

        case SDL_QUIT:
            exit(0);
            break;

        default:
            break;
        }
    }
}

void display()
{
    glUseProgram(g_toy.prog);
    if (g_toy.uloc_iResolution > -1) glUniform3f(g_toy.uloc_iResolution, (float)winw, (float)winh, 1.f);
    if (g_toy.uloc_iGlobalTime > -1) glUniform1f(g_toy.uloc_iGlobalTime, g_timer.seconds());
    glRecti(-1,-1,1,1);
}


int main(void)
{
    ///@todo cmd line aargs

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    int winw = 800;
    int winh = 600;

    SDL_Window* pWindow = SDL_CreateWindow(
        "kinderegg",
        100,100,
        winw, winh,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    // thank you http://www.brandonfoltz.com/2013/12/example-using-opengl-3-0-with-sdl2-and-glew/
    SDL_GLContext glContext = SDL_GL_CreateContext(pWindow);
    if (glContext == NULL)
    {
        printf("There was an error creating the OpenGL context!\n");
        return 0;
    }

    const unsigned char *version = glGetString(GL_VERSION);
    if (version == NULL) 
    {
        printf("There was an error creating the OpenGL context!\n");
        return 1;
    }

    SDL_GL_MakeCurrent(pWindow, glContext);

    // Don't forget to initialize Glew, turn glewExperimental on to
    // avoid problems fetching function pointers...
    glewExperimental = GL_TRUE;
    const GLenum l_Result = glewInit();
    if (l_Result != GLEW_OK)
    {
        exit(EXIT_FAILURE);
    }

    g_toy.prog = makeShaderByName("basic");
    g_toy.uloc_iResolution = glGetUniformLocation(g_toy.prog, "iResolution");
    g_toy.uloc_iGlobalTime = glGetUniformLocation(g_toy.prog, "iGlobalTime");

    int quit = 0;
    while (quit == 0)
    {
        PollEvents();
        display();
        SDL_GL_SwapWindow(pWindow);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(pWindow);

    SDL_Quit();
}
