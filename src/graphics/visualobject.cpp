#include "visualobject.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

void VisualObjectBase::draw() { m_mesh->draw(); }

VisualObjectBase::~VisualObjectBase() { }

/////////////////////////// VisualObject2D /////////////////////

VisualObjectSprite2D::VisualObjectSprite2D()
{
    m_mesh = GLMeshStatics::get().getQuad2d();
}

const NameUniformMap& VisualObjectSprite2D::updateAndGetUniforms()
{
    if (m_dirty) {
        mat4 mat(1);
        vec2 size = (m_rectBounds[1] - m_rectBounds[0]) * 0.5f; //... 2.0 | 0.5
        vec2 center2 = (m_rectBounds[1] + m_rectBounds[0]) * 0.5f;

        mat = translate(mat, glm::vec3(center2.x, center2.y, 0));
        mat = scale(mat, glm::vec3(size.x, size.y, 0.f));
        mat = rotate(mat, m_angle, glm::vec3(0, 0, 1));
        mat[3][0] += m_pos.x;
        mat[3][1] += m_pos.y;

        setUniform(modelWorldName, mat);
        m_dirty = false;
    }
    return m_uniforms;
}

/////////////////////////////// MESH 3D ////////////////////////////

void VisualObject3D::setTransform(const glm::mat4& transform)
{
    setUniform(modelWorldName, transform);
}
