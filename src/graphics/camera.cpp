#include "camera.h"

#include "gl_shader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

////////////////////////// CAMERA ORTHO //////////////////////////////

const glm::mat4& CameraOrtho::getView()
{
    if (m_viewDirty) {
        m_viewDirty = false;
        glm::vec2 scale = m_viewportSize * m_scaleMultiplier;
        glm::vec2 p0 = m_posWorld - scale * 0.5f;
        glm::vec2 p1 = m_posWorld + scale * 0.5f;
        // m_viewProjection = glm::ortho(p0.x, p1.x, p0.y, p1.y, -1.f, 1.f);
        m_matView = glm::ortho(p0.x, p1.x, p1.y, p0.y, -1.f, 1.f);
    }
    return m_matView;
}

void CameraOrtho::updateUniforms(GLShader* shader)
{
    if (shader) {
        shader->setUniform("matView", getView());
    }
}

////////////////////// CAMERA PERSPECTIVE //////////////////////////////

const glm::mat4& CameraPerspective::getView()
{
    if (m_viewDirty) {
        m_matView = glm::lookAt(m_pos, m_aim, m_up);
        m_viewDirty = false;
        // LOG("MatView" << glm::to_string(m_matView));
        // LOG("MatProjection" << glm::to_string(m_matProjection));
    }
    return m_matView;
}

const glm::mat4& CameraPerspective::getProjection()
{
    if (m_projectionDirty) {
        m_matProjection = glm::perspective(m_fov, m_ar, m_near, m_far);
        m_projectionDirty = false;
    }
    return m_matProjection;
}

void CameraPerspective::updateUniforms(GLShader* shader)
{
    if (shader) {
        shader->setUniform("cameraView", getView());
        shader->setUniform("cameraProjection", getProjection());
    }
}
