#ifndef MESH_H
#define MESH_H

#include <cstdint> // uintXX_t
#include <glm/glm.hpp>
#include <vector>

namespace GL {
typedef std::vector<uint8_t> ByteArray;

class Mesh {
    friend class MeshStatics;
    unsigned int m_VBO = -1, m_VAO = -1, m_vertCount = 0;

public:
    void draw();
    Mesh();
    ~Mesh();
};

class MeshStatics {
public:
    static MeshStatics& get()
    {
        static MeshStatics instance;
        return instance;
    }
    Mesh& getQuad() { return get().s_quad; };

private:
    MeshStatics();
    static Mesh s_quad;
};

}
#endif // MESH_H
