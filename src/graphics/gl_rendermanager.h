#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "camera.h"
#include "drawable.h"
#include "gl_framebuffer.h"

class GLRenderManager {
public:
    void draw(GLFrameBufferBase* frameBuffer, CameraBase* camera, const std::vector<std::unique_ptr<DrawableBase>>& drawable);
    void draw(GLFrameBufferBase* frameBuffer, CameraBase* camera, DrawableBase* drawable);
};

#endif // GL_RENDERMANAGER_H
