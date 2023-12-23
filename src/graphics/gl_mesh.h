#ifndef GLMESH_H
#define GLMESH_H

#include "common.h"
#include <cstdint> // uintXX_t
#include <memory>
#include <vector>

class Model3D;

//////////////////////// GL MESH BASE /////////////////////////

class GLMeshBase : NoCopy<GLMeshBase> {
protected:
    static uint32_t s_currentBindedMeshHandle;
    uint32_t m_VAO {};

protected:
    void bind() const;

public:
    virtual ~GLMeshBase() = default;

    virtual void draw() const = 0;
};

//////////////////////// GL MESH TRI ARRAY /////////////////////////

class GLMeshTriArray : public GLMeshBase {
    friend class GLMeshStatics;
    uint32_t m_VBO {}, m_vertCount {};

public:
    GLMeshTriArray& operator=(GLMeshTriArray&& rhs)
    {
        m_VAO = rhs.m_VAO, rhs.m_VAO = 0;
        m_VBO = rhs.m_VBO;
        m_vertCount = rhs.m_vertCount;
        return *this;
    }

    GLMeshTriArray(GLMeshTriArray&& rhs) { *this = std::move(rhs); }
    GLMeshTriArray() = default;
    ~GLMeshTriArray();

    void draw() const override;
};

//////////////////////// GL MESH TRI INDICES /////////////////////////

class GLMeshTriIndices : public GLMeshBase {
    uint32_t m_VBO {}, m_EBO {};
    uint32_t m_indicesCount {};

public:
    GLMeshTriIndices& operator=(GLMeshTriIndices&& rhs)
    {
        m_VAO = rhs.m_VAO, rhs.m_VAO = 0;
        m_VBO = rhs.m_VBO;
        m_indicesCount = rhs.m_indicesCount;
        return *this;
    }

    GLMeshTriIndices(GLMeshTriIndices&& rhs) { *this = std::move(rhs); }
    GLMeshTriIndices(const Model3D& model);
    ~GLMeshTriIndices();

    void draw() const override;
};

//////////////////////// STATIC FUNCTIONS /////////////////////////

class GLMeshStatics {
public:
    static GLMeshStatics& get()
    {
        static GLMeshStatics instance;
        return instance;
    }

    const std::shared_ptr<GLMeshTriArray>& getQuad2d() { return get().s_quad; }

private:
    GLMeshStatics();
    static std::shared_ptr<GLMeshTriArray> s_quad;
};

#endif // MESH_H
