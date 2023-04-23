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
    GLShader::Instance m_shaderInstance;

    glm::vec2 m_pos = glm::vec2(0);
    glm::vec2 m_size = glm::vec2(1);
    float m_angle = 0;
    bool m_dirty = true;

public:
    GLShader::Instance& getShaderInstance() { return m_shaderInstance; }
    Sprite2d();
    void init();
    Sprite2d& setPos(glm::vec2 pos);
    Sprite2d& setSize(glm::vec2 size);
    Sprite2d& setRotation(float angleRad);
    Sprite2d& addRotation(float angleOffset);

    void draw(const GLFrameBufferBase& where);
};

#endif // SPRITE2D_H
