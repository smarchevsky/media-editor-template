#ifndef MODEL3D_H
#define MODEL3D_H

// #include "image.h"

#include <filesystem>
#include <vector>

#define GLM_EXT_INCLUDED
#include <glm/gtx/hash.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec2 uv = glm::vec2(0);

    bool operator==(const Vertex& rhs) const
    {
        return position == rhs.position
            && normal == rhs.normal
            && uv == rhs.uv;
    }
};

namespace std {
template <>
struct hash<Vertex> {
    std::size_t operator()(const Vertex& k) const
    {
        size_t seed = 0;
        glm::detail::hash_combine(seed, std::hash<glm::vec3>()(k.position));
        glm::detail::hash_combine(seed, std::hash<glm::vec3>()(k.normal));
        glm::detail::hash_combine(seed, std::hash<glm::vec2>()(k.uv));
        return seed;
    }
};
}

struct Model3D {
    std::vector<Vertex> vertices;
    std::vector<glm::ivec3> triangles;
};

class MeshReader {
public:
    enum class ReadMode {
        XYZ,
        XZY
    };

    static std::vector<Model3D> read(const std::filesystem::path& path, ReadMode readMode = ReadMode::XYZ);

protected:
};

#endif // MODEL3D_H
