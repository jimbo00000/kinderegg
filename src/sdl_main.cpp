// sdl_main.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#undef main

#include "ShaderFunctions.h"
#include "Timer.h"
#include "g_textures.h"

Timer g_timer;
int winw = 800;
int winh = 600;

struct renderpass {
    GLuint prog;
    GLint uloc_iResolution;
    GLint uloc_iGlobalTime;
    GLint uloc_iChannelResolution;
    //iChannelTime not implemented
    GLint uloc_iChannel[4];
    GLint uloc_iMouse;
    GLint uloc_iDate;
    GLint uloc_iBlockOffset;
    GLint uloc_iSampleRate;
    GLuint texs[4];
};

struct Shadertoy {
    renderpass image;
    renderpass sound;
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
    const renderpass& r = g_toy.image;
    glUseProgram(r.prog);
    if (r.uloc_iResolution > -1) glUniform3f(r.uloc_iResolution, (float)winw, (float)winh, 1.f);
    if (r.uloc_iChannelResolution > -1)
    {
        float resos[] = {
            (float)texdims[3*0+0],
            (float)texdims[3*0+1],
            (float)texdims[3*0+2],
            (float)texdims[3*1+0],
            (float)texdims[3*1+1],
            (float)texdims[3*1+2],
            (float)texdims[3*2+0],
            (float)texdims[3*2+1],
            (float)texdims[3*2+2],
            (float)texdims[3*3+0],
            (float)texdims[3*3+1],
            (float)texdims[3*3+2],
        };
        glUniform3fv(r.uloc_iChannelResolution, 4, resos);
    }
    if (r.uloc_iGlobalTime > -1) glUniform1f(r.uloc_iGlobalTime, g_timer.seconds());
    if (r.uloc_iMouse > -1) glUniform4f(r.uloc_iMouse, 0.f, 0.f, 0.f, 0.f);

    SYSTEMTIME stNow;
    GetLocalTime(&stNow);
    if (r.uloc_iDate > -1) glUniform4f(r.uloc_iDate,
        (float)stNow.wYear,
        (float)stNow.wMonth - 1.f,
        (float)stNow.wDay,
        (float)stNow.wHour*60.f*60.f +
        (float)stNow.wMinute*60.f +
        (float)stNow.wSecond
        );

    for (int i=0; i<4; ++i)
    {
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D, r.texs[i]);
        if (r.uloc_iChannel[i] > -1) glUniform1i(r.uloc_iChannel[i], i);
    }
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
    wave.spec.format = AUDIO_F32;
    wave.spec.channels = 2;
    wave.spec.callback = fillerup;

    const int mPlayTime = 60; // Shadertoy gives 60 seconds of audio
    wave.soundlen = mPlayTime * wave.spec.freq * 2 * sizeof(float); // interlaced stereo
    wave.sound = new Uint8[wave.soundlen * sizeof(float)];
    float* pWaveData = (float*)wave.sound;
    glViewport(0,0,512,512);
    const renderpass& r = g_toy.sound;
    glUseProgram(r.prog);

    unsigned char* mData = new unsigned char[512*512*4];
    int mTmpBufferSamples = 512*512;
    int mPlaySamples = wave.soundlen / 2;
    int numBlocks = mPlaySamples / mTmpBufferSamples;
    for (int j=0; j<numBlocks; ++j)
    {
        int off = j * mTmpBufferSamples;
        if (r.uloc_iBlockOffset > -1) glUniform1f(r.uloc_iBlockOffset, (float)off / (float)wave.spec.freq);

        glRecti(-1,-1,1,1);
        glReadPixels(0,0,512,512, GL_RGBA, GL_UNSIGNED_BYTE, mData);
        for (int i = 0; i<mTmpBufferSamples; ++i)
        {
            unsigned char Llo = mData[4*i+0];
            unsigned char Lhi = mData[4*i+1];
            unsigned char Rlo = mData[4*i+2];
            unsigned char Rhi = mData[4*i+3];
            const float aL = ((float)Llo + 256.f*(float)Lhi) / 65535.f;
            const float aR = ((float)Rlo + 256.f*(float)Rhi) / 65535.f;
            pWaveData[2*(off + i)  ] = aL;
            pWaveData[2*(off + i)+1] = aR;
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
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_Window* pWindow = SDL_CreateWindow(
        shadername, // written to autogen/g_textures.h
        100, 100,
        winw, winh,
#ifdef NDEBUG
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL
#else
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
#endif
        );
    SDL_GetWindowSize(pWindow, &winw, &winh);
#ifdef NDEBUG
    SDL_ShowCursor(0);
#endif

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

    renderpass& r = g_toy.image;
    r.prog = makeShaderFromSource("passthru.vert", "image.frag");
    r.uloc_iResolution = glGetUniformLocation(r.prog, "iResolution");
    r.uloc_iGlobalTime = glGetUniformLocation(r.prog, "iGlobalTime");
    r.uloc_iChannelResolution = glGetUniformLocation(r.prog, "iChannelResolution");
    r.uloc_iChannel[0] = glGetUniformLocation(r.prog, "iChannel0");
    r.uloc_iChannel[1] = glGetUniformLocation(r.prog, "iChannel1");
    r.uloc_iChannel[2] = glGetUniformLocation(r.prog, "iChannel2");
    r.uloc_iChannel[3] = glGetUniformLocation(r.prog, "iChannel3");
    r.uloc_iMouse = glGetUniformLocation(r.prog, "iMouse");
    r.uloc_iDate = glGetUniformLocation(r.prog, "iDate");

    for (int i=0; i<4; ++i)
    {
        const int w = texdims[3*i];
        const int h = texdims[3*i+1];
        const int d = texdims[3*i+2];
        if (w == 0)
            continue;
        GLuint t = 0;
        glActiveTexture(GL_TEXTURE0+i);
        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        GLuint mode = 0;
        switch (d)
        {
        default:break;
        case 1: mode = GL_LUMINANCE;  break;
        case 3: mode = GL_RGB; break;
        case 4: mode = GL_RGBA; break;
        }

        char texname[6] = "tex00";
        texname[4] += i;
        std::ifstream file(texname, std::ios::binary);
        if (!file.is_open())
            continue;
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size))
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D,
                0, mode,
                w, h,
                0, mode,
                GL_UNSIGNED_BYTE,
                &buffer[0]);
            r.texs[i] = t;
        }
    }

    renderpass& s = g_toy.sound;
    s.prog = makeShaderFromSource("passthru.vert", "sound.frag");
    s.uloc_iBlockOffset = glGetUniformLocation(s.prog, "iBlockOffset");
    s.uloc_iSampleRate = glGetUniformLocation(s.prog, "iSampleRate");

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
