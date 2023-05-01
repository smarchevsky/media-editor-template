#ifndef GL_RENDERMANAGER_H
#define GL_RENDERMANAGER_H

#include "camera.h"
#include "entity.h"
#include "gl_framebuffer.h"

class GLRenderManager {
public:
    void draw(GLShader& shader,
        GLFrameBufferBase& frameBuffer,
        CameraBase* camera,
        const std::vector<std::unique_ptr<EntityBase>>& drawable);

    void draw(GLShader& shader,
        GLFrameBufferBase& frameBuffer,
        CameraBase* camera,
        EntityBase* drawable);
};

#endif // GL_RENDERMANAGER_H
