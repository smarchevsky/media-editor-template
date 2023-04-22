#ifndef SPRITE2D_H
#define SPRITE2D_H

#include "gl_mesh.h"
#include "gl_shader.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class GLFrameBufferBase;

class Sprite2d // square sprite -1 to 1
{
    const GLMesh& m_mesh;
    GLShaderInstance m_shader;

    glm::mat4 m_transform = glm::mat4(1);

public:
    GLShaderInstance& getShaderInstance() { return m_shader; }
    Sprite2d();
    void init();
    Sprite2d& setPos(glm::vec2 pos);
    Sprite2d& setSize(glm::vec2 size);

    void draw(const GLFrameBufferBase& where);
};

#endif // SPRITE2D_H
