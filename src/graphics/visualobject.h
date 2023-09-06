#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

#include "entity.h"
#include "gl_mesh.h"

class VisualObjectBase : public EntityBase {
protected:
    static constexpr const char* modelWorldName = "modelWorld";
    std::shared_ptr<GLMeshBase> m_mesh;

public:
    const std::shared_ptr<GLMeshBase>& getMesh() const { return m_mesh; }
    void draw();
    virtual ~VisualObjectBase() = 0;
};

/////////////////////////////// SPRITE 2D ////////////////////////////

// square sprite -1 to 1
class VisualObjectSprite2D : public VisualObjectBase {
    glm::vec2 m_pos = glm::vec2(0);
    glm::vec2 m_size = glm::vec2(1);
    float m_angle = 0;
    bool m_dirty = true;

public:
    VisualObjectSprite2D();

    void setPos(glm::vec2 pos) { m_pos = pos, m_dirty = true; }
    void setSize(glm::vec2 size) { m_size = size, m_dirty = true; }
    void setRotation(float angleRad) { m_angle = angleRad, m_dirty = true; }
    void addRotation(float angleOffset) { setRotation(m_angle + angleOffset); }
    void applyUniforms(GLShader* shader) override;
};

/////////////////////////////// MESH 3D ////////////////////////////

class VisualObject3D : public VisualObjectBase {

public:
    void setMesh(const std::shared_ptr<GLMeshTriIndices>& mesh) { m_mesh = mesh; }

    void setTransform(const glm::mat4& transform);

    void applyUniforms(GLShader* shader) override;
};

#endif // VISUALOBJECT_H
