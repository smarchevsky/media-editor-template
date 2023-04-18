#include "orthocamera.h"
#include <glm/gtc/matrix_transform.hpp>
OrthoCamera::OrthoCamera()
{
}

glm::mat4 OrthoCamera::getViewProjection()
{
    if (m_isDirty) {
        glm::vec2 scale = m_viewportSize * m_scaleMultiplier;
        glm::vec2 p0 = m_posWorld - scale * 0.5f;
        glm::vec2 p1 = m_posWorld + scale * 0.5f;
        // m_viewProjection = glm::ortho(p0.x, p1.x, p0.y, p1.y, -1.f, 1.f);
        m_viewProjection = glm::ortho(p0.x, p1.x, p1.y, p0.y, -1.f, 1.f);
        m_isDirty = false;
    }
    return m_viewProjection;
}
