#ifndef GLMESH_H
#define GLMESH_H

#include "helper_general.h"
#include <cstdint> // uintXX_t
#include <vector>

typedef std::vector<uint8_t> ByteArray;

class GLMesh : NoCopy<GLMesh> {
    friend class GLMeshStatics;
    static uint32_t s_currentBindedMeshHandle;
    unsigned int m_VBO {}, m_VAO {}, m_vertCount {};

public:
    GLMesh();
    ~GLMesh();

    void draw() const;
    void bind() const;
};

class GLMeshStatics {
public:
    static GLMeshStatics& get()
    {
        static GLMeshStatics instance;
        return instance;
    }
    GLMesh& getQuad2d() { return get().s_quad; };

private:
    GLMeshStatics();
    static GLMesh s_quad;
};

#endif // MESH_H
