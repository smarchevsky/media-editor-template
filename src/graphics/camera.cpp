#include "camera.h"

#include "gl_shader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
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
        shader->setUniform("cameraView", getView());
    }
}

////////////////////// CAMERA PERSPECTIVE //////////////////////////////

void CameraPerspective::pan(glm::vec2 delta)
{
    glm::mat4 invView = glm::inverse(getView());

    float distance = glm::distance(m_cameraPosition, m_aimPosition);
    glm::vec3 right = invView[0];
    glm::vec3 up = invView[1];
    auto offset = (-right * delta.x + up * delta.y) * distance;

    m_cameraPosition += offset;
    setAim(m_aimPosition + offset);
}

void CameraPerspective::rotateAroundAim(glm::vec2 offsetDeltaRadians)
{
    m_sceneRotation += glm::vec2(-offsetDeltaRadians.x, offsetDeltaRadians.y);
    m_sceneRotation.y = glm::clamp(m_sceneRotation.y, -(float)M_PI_2 + 0.001f, (float)M_PI_2 - 0.001f);
    m_sceneRotation.x = fmodf(m_sceneRotation.x, M_PI * 2);

    glm::vec3 rotX = glm::rotateX(glm::vec3(0.f, getDistance(), 0.f), m_sceneRotation.y);
    glm::vec3 rotZ = m_aimPosition + glm::rotateZ(rotX, m_sceneRotation.x);
    m_cameraPosition = rotZ;
    m_viewDirty = true;
}

const glm::mat4& CameraPerspective::getView()
{
    if (m_viewDirty) {
        m_cameraView = glm::lookAt(m_cameraPosition, m_aimPosition, m_up);
        m_viewDirty = false;
    }
    return m_cameraView;
}

const glm::mat4& CameraPerspective::getProjection()
{
    if (m_projectionDirty) {
        m_cameraProjection = glm::perspective(m_fov, m_ar, m_near, m_far);
        m_projectionDirty = false;
    }
    return m_cameraProjection;
}

void CameraPerspective::updateUniforms(GLShader* shader)
{
    if (shader) {
        shader->setUniform("cameraView", getView());
        shader->setUniform("cameraPosition", m_cameraPosition);
        shader->setUniform("cameraProjection", getProjection());
    }
}

CameraPerspective::CameraPerspective()
{
    m_sceneRotation = glm::vec2(1.8f, .5f);
    m_cameraPosition = { -2.f, 0.f, 0.7f };
    m_aimPosition = { 0.f, 0.f, 0.7f };
    m_up = { 0.f, 0.f, 1.f };
    rotateAroundAim(glm::vec2(0));
}

////////////////////// CAMERA PERSPECTIVE JITTERED //////////////////////////////

const glm::mat4& CameraPerspectiveJittered::getProjection()
{
    glm::vec2 jitterAA = glm::diskRand(1.f) * m_jitterSizeAA;
    glm::vec2 jitterDOF = glm::diskRand(1.f) * m_jitterSizeDOF;
    // glm::vec4 j(jitter.x, jitter.y, 0.f, 0.f);
    m_cameraProjection = glm::perspective(m_fov, m_ar, m_near, m_far);
    auto projectedDistance = glm::vec4(0, 0, -getDistance(), 0) * m_cameraProjection;

    glm::vec4& z = m_cameraProjection[2];
    glm::vec4& w = m_cameraProjection[3];

    z.x += jitterDOF.x / projectedDistance.z;
    z.y += jitterDOF.y / projectedDistance.z;
    w.x += jitterDOF.x;
    w.y += jitterDOF.y;

    z.x += jitterAA.x;
    z.y += jitterAA.y;

    m_projectionDirty = true;
    return m_cameraProjection;
}
