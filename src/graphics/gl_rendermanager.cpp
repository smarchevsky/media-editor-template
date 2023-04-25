#include "gl_rendermanager.h"
#include "gl_shader.h"

#include <SDL2/SDL.h>

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>

#include <set>
#include <cassert>

void GLRenderManager::draw(GLFrameBufferBase* frameBuffer, CameraBase* camera, const std::vector<DrawableBase*>& drawable)
{
    std::set<GLShader*> usedShaders;

    // TODO
}
