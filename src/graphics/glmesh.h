#ifndef GLMESH_H
#define GLMESH_H

#include "helper_general.h"
#include <cstdint> // uintXX_t
#include <vector>

typedef std::vector<uint8_t> ByteArray;

class GLMesh : NoCopy<GLMesh> {
    friend class GLMeshStatics;
    unsigned int m_VBO {}, m_VAO {}, m_vertCount {};

public:
    void draw();
    GLMesh();
    ~GLMesh();
};

class GLMeshStatics {
public:
    static GLMeshStatics& get()
    {
        static GLMeshStatics instance;
        return instance;
    }
    GLMesh& getQuad() { return get().s_quad; };

private:
    GLMeshStatics();
    static GLMesh s_quad;
};

#endif // MESH_H
