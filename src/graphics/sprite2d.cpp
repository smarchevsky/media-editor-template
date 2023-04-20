#include "sprite2d.h"
#include "glshader.h"
#include <glm/gtc/matrix_transform.hpp>

Sprite2d::Sprite2d()
    : m_mesh(GLMeshStatics::get().getQuad2d())
    , m_shader(GLShaderManager::get().getDefaultShader2d())
{
    m_transform = glm::mat4(1);
}

Sprite2d& Sprite2d::setPos(glm::vec2 pos)
{
    m_transform[3][0] = pos.x;
    m_transform[3][1] = pos.y;
    return *this;
}

Sprite2d& Sprite2d::setSize(glm::vec2 size)
{
    m_transform = glm::scale(m_transform, glm::vec3(size.x, size.y, 0.f));
    return *this;
}

void Sprite2d::draw()
{
    m_shader->setUniform("matModel", m_transform);
    m_mesh.draw();
}
