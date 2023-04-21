#include "gl_context.h"

#include <SDL2/SDL.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <cassert>

void GLContext::setOpenGLViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void GLContext::clear()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLContext::unbindFrameBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

GLContext::GLContext()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        assert(false && "SDL did not init");
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
}

GLContext::~GLContext()
{
    SDL_Quit();
}
