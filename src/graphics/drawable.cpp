#include "drawable.h"
#include "gl_shader.h"
#include <glm/gtc/matrix_transform.hpp>

Sprite2d::Sprite2d()
    : m_mesh(GLMeshStatics::get().getQuad2d())
{
    initializeUniform("matModel", glm::mat4(1));
    initializeUniform("texture0", Texture2Ddata());
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

void Sprite2d::applyUniformsAndDraw(GLShader* shader)
{
    if (m_dirty) {
        glm::mat4 mat(1);
        mat = glm::scale(mat, glm::vec3(m_size.x, m_size.y, 0.f));
        mat = glm::rotate(mat, m_angle, glm::vec3(0, 0, 1));
        mat[3][0] = m_pos.x;
        mat[3][1] = m_pos.y;

        setUniform("matModel", mat);
        m_dirty = false;
    }
    for (const auto& u : getUniforms()) {
        shader->setUniform(u.first, u.second);
    }

    m_mesh.draw();
}
