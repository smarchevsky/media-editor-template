#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "gl_mesh.h"
#include "gl_shader.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class GLFrameBufferBase;

class DrawableBase {
protected:
    GLShader::Instance m_shaderInstance;

public:
    GLShader::Instance& getShaderInstance() { return m_shaderInstance; }
    const GLShader::Instance& getShaderInstance() const { return m_shaderInstance; }

    virtual void draw(bool forceBindShader = true) { }
    virtual ~DrawableBase() = default;
};

// square sprite -1 to 1
class Sprite2d : public DrawableBase {
    const GLMesh& m_mesh;

    glm::vec2 m_pos = glm::vec2(0);
    glm::vec2 m_size = glm::vec2(1);
    float m_angle = 0;
    bool m_dirty = true;

public:
    Sprite2d();

    Sprite2d& setPos(glm::vec2 pos);
    Sprite2d& setSize(glm::vec2 size);
    Sprite2d& setRotation(float angleRad);
    Sprite2d& addRotation(float angleOffset);

    void draw(bool forceBindShader = true) override;
};

#endif // SPRITE2D_H
