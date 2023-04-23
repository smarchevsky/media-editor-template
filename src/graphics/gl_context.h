#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

#include "gl_framebuffer.h"
#include "sprite2d.h"

#include <glm/vec3.hpp>
class GLContext {
public:
    static GLContext& getInit()
    {
        static GLContext instance;
        return instance;
    }

    static void setOpenGLViewport(int x, int y, int width, int height);
    static void unbindFrameBuffer();


private:
    GLContext();
    ~GLContext();
};

#endif // GRAPHICS_COMMON_H
