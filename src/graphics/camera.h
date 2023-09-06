#ifndef CAMERA_H
#define CAMERA_H

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <string>

#include "entity.h"

class GLShader;

class CameraBase : public EntityBase {

public:
    template <class CameraType>
    CameraType* as() { return dynamic_cast<CameraType*>(this); }
    virtual ~CameraBase() = 0;
};

////////////////////////// CAMERA ORTHO //////////////////////////////

class CameraOrtho : public CameraBase {
protected:
    glm::mat4 m_matView = glm::mat4(1);

private:
    glm::vec2 m_posWorld = glm::vec2(0);
    glm::vec2 m_viewportSize = glm::vec2(1000, 1000);
    float m_scaleMultiplier = 0.003f;

    bool m_viewDirty = true;

public:
    void setScale(float scale) { m_scaleMultiplier = scale, m_viewDirty = true; }
    void multiplyScale(float scaleOffset) { m_scaleMultiplier *= scaleOffset, m_viewDirty = true; }
    void multiplyScaleOffseted(float scaleFactor, glm::vec2 viewSpaceOffset)
    {
        glm::vec2 mousePosOffset = (viewSpaceOffset - 0.5f * m_viewportSize)
            * m_scaleMultiplier;
        m_posWorld += mousePosOffset * (1 - scaleFactor);
        multiplyScale(scaleFactor);
    }

    void setViewportSize(glm::vec2 vs) { m_viewportSize = vs, m_viewDirty = true; }
    void setPosition_WS(glm::vec2 pos) { m_posWorld = pos, m_viewDirty = true; }
    void addOffset_WS(glm::vec2 offset) { m_posWorld += offset, m_viewDirty = true; }
    void addOffset_View(glm::vec2 offset) { addOffset_WS(offset * m_scaleMultiplier); }

    glm::vec2 getPos() const { return m_posWorld; }
    float getScale() const { return m_scaleMultiplier; }

    const glm::mat4& getViewInv();

    virtual const NameUniformMap& updateAndGetUniforms() override;
};

////////////////////// CAMERA PERSPECTIVE //////////////////////////////

class CameraPerspective : public CameraBase {
protected:
    glm::mat4 m_cameraView;
    glm::mat4 m_cameraViewInv;
    glm::mat4 m_cameraProjection;

    glm::vec2 m_sceneRotation; // set in constructor
    glm::vec3 m_cameraPosition, m_aimPosition, m_up;

    float m_near = .1f, m_far = 2000.f;
    float m_fov = 1.f /*radians*/, m_ar = 1.f;
    bool m_viewDirty = true, m_projectionDirty = true;

protected:
    void updateView();
    void updateProjection();

public:
    CameraPerspective();
    void setFOV(float fov) { m_fov = fov, m_projectionDirty = true; }
    void setAR(float ar) { m_ar = ar, m_projectionDirty = true; }

    void setAim(glm::vec3 aim) { m_aimPosition = aim, m_viewDirty = true; }
    void offsetAim(glm::vec3 delta_aim) { setAim(m_cameraPosition + delta_aim); }

    void setUp(glm::vec3 up) { m_up = up, m_viewDirty = true; }

    void setDistanceFromAim(float newDistance)
    {
        auto offset = m_cameraPosition - m_aimPosition;
        auto dir = offset / glm::length(offset);
        m_cameraPosition = m_aimPosition + dir * newDistance;
        m_viewDirty = true;
    }

    void rotateAroundAim(glm::vec2 offsetDeltaRadians);
    void pan(glm::vec2 delta);

    glm::vec3 getPos() const { return m_cameraPosition; }
    glm::vec3 getAim() const { return m_aimPosition; }
    glm::vec3 getUp() const { return m_up; }
    float getDistance() const { return glm::distance(m_cameraPosition, m_aimPosition); }

    const glm::mat4& getViewInv();
    const glm::mat4& getView();

    virtual const glm::mat4& getProjection();
    virtual const NameUniformMap& updateAndGetUniforms() override;
};

////////////////////// CAMERA PERSPECTIVE JITTERED //////////////////////////////

class CameraPerspectiveJittered : public CameraPerspective {
protected:
    glm::vec2 m_jitterSizeAA = glm::vec2(0);
    float m_jitterSizeDOF = 0.1f;
    bool m_jitterEnabled = true;

public:
    void setJitterAA(glm::vec2 jitter) { m_jitterSizeAA = jitter; }
    void setJitterDOV(float radius) { m_jitterSizeDOF = radius; }
    void setJitterEnabled(bool enabled) { m_jitterEnabled = enabled; }
    virtual const glm::mat4& getProjection() override;
};

#endif // ORTHOCAMERA_H
