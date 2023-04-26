#include "drawable.h"
#include "gl_shader.h"
#include <glm/gtc/matrix_transform.hpp>

Sprite2d::Sprite2d()
    : m_mesh(GLMeshStatics::get().getQuad2d())
{
    m_shaderInstance = GLShader::Instance(
        GLShaderManager::get().getDefaultShader2d(),
        UniformDependency::Object);
}

Sprite2d& Sprite2d::setPos(glm::vec2 pos)
{
    m_pos = pos;
    m_dirty = true;
    return *this;
}

Sprite2d& Sprite2d::setSize(glm::vec2 size)
{
    m_size = size;
    m_dirty = true;
    return *this;
}

Sprite2d& Sprite2d::setRotation(float angleRad)
{
    m_angle = angleRad;
    m_dirty = true;
    return *this;
}

Sprite2d& Sprite2d::addRotation(float angleOffset)
{
    return setRotation(m_angle + angleOffset);
}

void Sprite2d::draw(bool forceBindShader)
{
    if (m_dirty) {
        glm::mat4 mat(1);
        mat = glm::scale(mat, glm::vec3(m_size.x, m_size.y, 0.f));
        mat = glm::rotate(mat, m_angle, glm::vec3(0, 0, 1));
        mat[3][0] = m_pos.x;
        mat[3][1] = m_pos.y;

        m_shaderInstance.set("matModel", mat);
        m_dirty = false;
    }
    m_shaderInstance.applyUniforms(forceBindShader);
    m_mesh.draw();
}
