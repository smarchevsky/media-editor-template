#include "sprite2d.h"
#include "gl_framebuffer.h"
#include "gl_shader.h"
#include <glm/gtc/matrix_transform.hpp>

Sprite2d::Sprite2d()
    : m_mesh(GLMeshStatics::get().getQuad2d())
{
}

void Sprite2d::init()
{
    m_shader.setShader(GLShaderManager::get().getDefaultShader2d());
    m_shader.trackUniformsExcept({ "matViewProjection" });

    m_shader.updateUniform("matModel", m_transform);
}

Sprite2d& Sprite2d::setPos(glm::vec2 pos)
{
    m_transform[3][0] = pos.x;
    m_transform[3][1] = pos.y;
    m_shader.updateUniform("matModel", m_transform);
    return *this;
}

Sprite2d& Sprite2d::setSize(glm::vec2 size)
{
    m_transform = glm::scale(m_transform, glm::vec3(size.x, size.y, 0.f));
    m_shader.updateUniform("matModel", m_transform);
    return *this;
}

void Sprite2d::draw(const GLFrameBufferBase& where)
{
    where.bind();

    m_shader.applyUniformData();
    m_mesh.draw();
}
