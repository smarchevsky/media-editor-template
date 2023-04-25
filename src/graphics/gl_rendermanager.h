#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "gl_framebuffer.h"
#include "camera.h"
#include "drawable.h"

class GLRenderManager {
public:
    void draw(GLFrameBufferBase* frameBuffer, CameraBase* camera, const std::vector<DrawableBase*>& drawable);

private:
};

#endif // GRAPHICS_COMMON_H
