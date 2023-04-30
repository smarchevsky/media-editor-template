#ifndef GLMESH_H
#define GLMESH_H

#include "helper_general.h"
#include <cstdint> // uintXX_t
#include <vector>

class Model3D;

//////////////////////// GL MESH BASE /////////////////////////

class GLMeshBase : NoCopy<GLMeshBase> {
    static uint32_t s_currentBindedMeshHandle;

protected:
    void bind(uint32_t handle) const;
    virtual ~GLMeshBase() = default;
    virtual void draw() const = 0;
};

//////////////////////// GL MESH TRI ARRAY /////////////////////////

class GLMeshTriArray : public GLMeshBase {
    friend class GLMeshStatics;
    uint32_t m_VBO {}, m_VAO {}, m_vertCount {};

public:
    GLMeshTriArray() = default;
    ~GLMeshTriArray();

    void draw() const override;
};

//////////////////////// GL MESH TRI INDICES /////////////////////////

class GLMeshTriIndices : public GLMeshBase {
    uint32_t m_VAO {}, m_VBO {}, m_EBO {};
    uint32_t m_meshElementArraySize {};

public:
    ~GLMeshTriIndices ();
    GLMeshTriIndices(const Model3D& model);
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
    GLMeshTriArray& getQuad2d() { return get().s_quad; };

private:
    GLMeshStatics();
    static GLMeshTriArray s_quad;
};

#endif // MESH_H
