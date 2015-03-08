// glfw_main.cpp

#include <GL/glew.h>

#if defined(_WIN32)
#  include <Windows.h>
#  define GLFW_EXPOSE_NATIVE_WIN32
#  define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__linux__)
#  include <X11/X.h>
#  include <X11/extensions/Xrandr.h>
#  define GLFW_EXPOSE_NATIVE_X11
#  define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3.h>

#if !defined(__APPLE__)
#  include <GLFW/glfw3native.h>
#endif

#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <string.h>
#include <sstream>

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

// Set VSync is framework-dependent and has to come before the include
///@param state 0=off, 1=on, -1=adaptive
// Set vsync for both contexts.
static void SetVsync(int state)
{
    glfwSwapInterval(state);
}

static void ErrorCallback(int p_Error, const char* p_Description)
{
    (void)p_Error;
    (void)p_Description;
}


void keyboard(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
    (void)pWindow;
    (void)codes;

    if (action == GLFW_PRESS)
    {
    switch (key)
    {
        default:
            break;

        case GLFW_KEY_ESCAPE:
            glfwTerminate();
            break;
        }
    }
}

void resize(GLFWwindow* pWindow, int w, int h)
{
    (void)pWindow;
}

void display()
{
    glUseProgram(g_toy.prog);
    if (g_toy.uloc_iResolution > -1) glUniform3f(g_toy.uloc_iResolution, (float)winw, (float)winh, 1.f);
    if (g_toy.uloc_iGlobalTime > -1) glUniform1f(g_toy.uloc_iGlobalTime, g_timer.seconds());
    glRecti(-1,-1,1,1);
}

int main(int argc, char** argv)
{
    bool useOpenGLCoreContext = false;

#ifdef USE_CORE_CONTEXT
   // useOpenGLCoreContext = true;
#endif

#ifdef _LINUX
    // Linux driver seems to be lagging a bit
    useOpenGLCoreContext = false;
#endif

    // Command line options
    for (int i=0; i<argc; ++i)
    {
        const std::string a = argv[i];
        if (!a.compare("-core"))
        {
            useOpenGLCoreContext = true;
        }
        else if (!a.compare("-compat"))
        {
            useOpenGLCoreContext = false;
        }
    }

    GLFWwindow* l_Window = NULL;
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

#ifndef _LINUX
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#  if defined(_MACOS)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#  else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#  endif
#endif //ndef _LINUX
    if (useOpenGLCoreContext)
    {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }
    else
    {
#ifndef _LINUX
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
    }

    glfwWindowHint(GLFW_SAMPLES, 0);
    l_Window = glfwCreateWindow(winw, winh, "kinderegg", NULL, NULL);
    if (!l_Window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(l_Window);
    glfwSetWindowSizeCallback(l_Window, resize);
    glfwSetKeyCallback(l_Window, keyboard);

    glfwMakeContextCurrent(l_Window);

    // Don't forget to initialize Glew, turn glewExperimental on to
    // avoid problems fetching function pointers...
    glewExperimental = GL_TRUE;
    const GLenum l_Result = glewInit();
    if (l_Result != GLEW_OK)
    {
        exit(EXIT_FAILURE);
    }

    GLuint l_vao = 0;
    glGenVertexArrays(1, &l_vao);
    glBindVertexArray(l_vao);

    g_toy.prog = makeShaderByName("basic");
    g_toy.uloc_iResolution = glGetUniformLocation(g_toy.prog, "iResolution");
    g_toy.uloc_iGlobalTime = glGetUniformLocation(g_toy.prog, "iGlobalTime");

    while (!glfwWindowShouldClose(l_Window))
    {
        glfwPollEvents();
        display();
        glfwSwapBuffers(l_Window);
    }

    glfwDestroyWindow(l_Window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
