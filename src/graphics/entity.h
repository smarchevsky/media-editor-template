#ifndef ENTITY_H
#define ENTITY_H

#include "gl_mesh.h"
#include "gl_shader.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

//typedef std::tuple<HashString, UniformVariant> Uniform;

class EntityBase {
    typedef std::unordered_map<HashString, UniformVariant> NameUniformMap;

protected:
    NameUniformMap m_uniforms;
    std::shared_ptr<GLMeshBase> m_mesh;

public:
    void initializeUniform(HashString str, const UniformVariant& var) { m_uniforms[str] = var; }
    void setUniform(HashString str, const UniformVariant& var);
    const NameUniformMap& getUniforms() { return m_uniforms; }
    virtual void applyUniformsAndDraw(GLShader* shader);

    virtual ~EntityBase() = default;
};

/////////////////////////////// SPRITE 2D ////////////////////////////

// square sprite -1 to 1
class EntitySprite2D : public EntityBase {
    glm::vec2 m_pos = glm::vec2(0);
    glm::vec2 m_size = glm::vec2(1);
    float m_angle = 0;
    bool m_dirty = true;

public:
    EntitySprite2D();

    void setPos(glm::vec2 pos) { m_pos = pos, m_dirty = true; }
    void setSize(glm::vec2 size) { m_size = size, m_dirty = true; }
    void setRotation(float angleRad) { m_angle = angleRad, m_dirty = true; }
    void addRotation(float angleOffset) { setRotation(m_angle + angleOffset); }
    void applyUniformsAndDraw(GLShader* shader) override;
};

/////////////////////////////// MESH 3D ////////////////////////////

class EntityMesh3D : public EntityBase {

public:
    EntityMesh3D();
    void setMesh(const std::shared_ptr<GLMeshTriIndices>& mesh) { m_mesh = mesh; }
    void setTransform(const glm::mat4& transform);
    void applyUniformsAndDraw(GLShader* shader) override;
};

#endif // SPRITE2D_H
