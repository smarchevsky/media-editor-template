#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "gl_mesh.h"
#include "gl_shader.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class GLFrameBufferBase;

class DrawableBase {
    std::unordered_map<HashString, UniformVariant> m_uniforms;

protected:
    // warning! initialize uniform with proper type
    void initializeUniform(HashString str, const UniformVariant& var) { m_uniforms[str] = var; }

public:
    const std::unordered_map<HashString, UniformVariant>& getUniforms() { return m_uniforms; }
    virtual void applyUniformsAndDraw(GLShader* shader) { }
    virtual ~DrawableBase() = default;
    void setUniform(HashString str, const UniformVariant& var)
    {
        auto it = m_uniforms.find(str);
        if (it != m_uniforms.end()) {
            it->second = var;
        } else {
            assert(false && "No such uniform");
        }
    }
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

    void applyUniformsAndDraw(GLShader* shader) override;
};

#endif // SPRITE2D_H
