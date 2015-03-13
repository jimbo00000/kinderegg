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
    GLuint progsound;
    GLint uloc_iResolution;
    GLint uloc_iGlobalTime;
    GLint uloc_iBlockOffset;
    GLint uloc_iSampleRate;
};

Shadertoy g_toy;


void keyboard(const SDL_Event& event, int key, int codes, int action, int mods)
{
    (void)codes;
    (void)mods;

    if (action == SDL_KEYDOWN)
    {
        switch (key)
        {
        default:
            break;

        case SDLK_ESCAPE:
            SDL_Quit();
            exit(0);
            break;
        }
    }
}

void PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            keyboard(event, event.key.keysym.sym, 0, event.key.type, 0);
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


//
// Audio
//

struct
{
    SDL_AudioSpec spec;
    Uint8 *sound;               /* Pointer to wave data */
    Uint32 soundlen;            /* Length of wave data */
    int soundpos;               /* Current play position */
} wave;

void SDLCALL fillerup(void *unused, Uint8 * stream, int len)
{
    Uint8 *waveptr;
    int waveleft;

    waveptr = wave.sound + wave.soundpos;
    waveleft = wave.soundlen - wave.soundpos;

    while (waveleft <= len) { // wrap condition
        SDL_memcpy(stream, waveptr, waveleft);
        stream += waveleft;
        len -= waveleft;
        waveptr = wave.sound;
        waveleft = wave.soundlen;
        wave.soundpos = 0;
    }
    SDL_memcpy(stream, waveptr, len);
    wave.soundpos += len;
}

void play_audio()
{
    wave.spec.freq = 44100;
    wave.spec.format = AUDIO_U8; //AUDIO_S16LSB;
    wave.spec.channels = 2;
    wave.spec.callback = fillerup;

    const int mPlayTime = 60; // Shadertoy gives 60 seconds of audio
    wave.soundlen = mPlayTime * wave.spec.freq;
    wave.sound = new Uint8[2*wave.soundlen];
    glViewport(0,0,512,512);
    glUseProgram(g_toy.progsound);

    unsigned char* mData = new unsigned char[512*512*4];
    int mTmpBufferSamples = 262144;
    int mPlaySamples = wave.soundlen;
    int numBlocks = mPlaySamples / mTmpBufferSamples;
    for (int j=0; j<numBlocks; ++j)
    {
        int off = j * mTmpBufferSamples;
        if (g_toy.uloc_iBlockOffset > -1) glUniform1f(g_toy.uloc_iBlockOffset, (float)off / (float)wave.spec.freq);

        glRecti(-1,-1,1,1);
        // mData: Uint8Array[1048576]
        glReadPixels(0,0,512,512, GL_RGBA, GL_UNSIGNED_BYTE, mData);
        for (int i = 0; i<mTmpBufferSamples; ++i)
        {
            const float aL = -1.0f + 2.0f*((float)mData[4 * i + 0] + 256.0f*(float)mData[4 * i + 1]) / 65535.0f;
            const float aR = -1.0f + 2.0f*((float)mData[4 * i + 2] + 256.0f*(float)mData[4 * i + 3]) / 65535.0f;
            wave.sound[2*(off + i)  ] = (unsigned char)(.5f*(1.f+aL) * 255.f);
            wave.sound[2*(off + i)+1] = (unsigned char)(.5f*(1.f+aR) * 255.f);
        }
    }
    delete [] mData;

    if (SDL_OpenAudio(&wave.spec, NULL) < 0)
    {
        SDL_FreeWAV(wave.sound);
        SDL_Quit();
        exit(2);
    }

    SDL_PauseAudio(0); // Start playing
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
    g_toy.progsound = makeShaderByName("basicsound");
    g_toy.uloc_iBlockOffset = glGetUniformLocation(g_toy.progsound, "iBlockOffset");
    g_toy.uloc_iSampleRate = glGetUniformLocation(g_toy.progsound, "iSampleRate");

    play_audio();
    glViewport(0,0, winw, winh);

    int quit = 0;
    while (quit == 0)
    {
        PollEvents();
        display();
        SDL_GL_SwapWindow(pWindow);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(pWindow);
    SDL_CloseAudio();
    SDL_FreeWAV(wave.sound);
    SDL_Quit();
}
