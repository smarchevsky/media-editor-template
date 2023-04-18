#include "graphics_common.h"

#include <SDL2/SDL.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <cassert>

void GraphicsCommon::setOpenGLViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

GraphicsCommon::GraphicsCommon()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        assert(false && "SDL did not init");
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
}

GraphicsCommon::~GraphicsCommon()
{
    SDL_Quit();
}
