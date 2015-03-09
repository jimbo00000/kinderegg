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

// prototype for our audio callback
void my_audio_callback(void *userdata, Uint8 *stream, int len);

const int samps = 1024*8; // 65536 bytes appears to be max chunk size
const int bufSz = samps*sizeof(Sint16); //*channels
Uint8 *sound_buffer = NULL;
Uint32 audio_pos = 0;
int counter = 0;


void play_audio()
{
    sound_buffer = new Uint8[bufSz];
    audio_pos = 0;

    static SDL_AudioSpec wav_spec;
    wav_spec.freq = 44100;
    wav_spec.format = AUDIO_S16LSB;
    wav_spec.channels = 1;
    wav_spec.samples = samps;
    wav_spec.callback = my_audio_callback;
    wav_spec.userdata = NULL;

    // Open the audio device
    if ( SDL_OpenAudio(&wav_spec, NULL) < 0 )
    {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        exit(-1);
    }
    SDL_PauseAudio(0); // Start playing
}

int cb = 0;

// audio callback function
// here you have to copy the data of your audio buffer into the requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void my_audio_callback(void *userdata, Uint8 *stream, int len)
{
    printf("   callback:  %x  %d  counter:%d   ap: %d\n", stream, len, counter, audio_pos);

    double pi = 3.1415;
    double Hz = 440 * ((cb%2)+1);// + (counter/256);
    double A = 32767;
    double SR = 44100;
    double F=2*pi*Hz/SR;

    Sint16* stream16 = reinterpret_cast<Sint16*>(sound_buffer);
    const int len16 = len / sizeof(Sint16);
    for (int z = 0; z<len16; z++)
    {
        counter++;
        stream16[z] = (Sint16) A*sin(F*(double)counter);
    }

    // Loop to beginning of buffer
    if (audio_pos + len > bufSz)
    {
        audio_pos = 0;
    }

    SDL_memset(stream, 0, len);
    const Uint8* apos = sound_buffer + audio_pos;
    //SDL_memcpy (stream, apos, len);                  // simply copy from one buffer into the other
    SDL_MixAudio(stream, apos, len, SDL_MIX_MAXVOLUME);// mix from one buffer into another

    audio_pos += len;
    ++cb;
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

    //play_audio();

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
    SDL_Quit();
}
