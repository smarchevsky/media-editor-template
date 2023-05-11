#include "model3d.h"
// #include "helper_general.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>

// glm::vec3 toV3(const std::vector<tinyobj::real_t> v) { return glm::vec3(v[0], v[1], v[2]); }
// glm::vec2 toV2(const std::vector<tinyobj::real_t> v) { return glm::vec2(v[0], v[1]); }

std::vector<Model3D> MeshReader::read(const std::filesystem::path& path)
{
    std::vector<Model3D> resultMeshes;

    using namespace tinyobj;

    ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    for (size_t s = 0; s < shapes.size(); s++) { // SHAPES
        size_t index_offset = 0;

        Model3D resultMesh;
        std::unordered_map<Vertex, int> pointHashMap;
        int indexOffset = 0;

        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) { // TRIANGLES IN SHAPES
            const size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            assert(fv == 3 && "Why TinyObj did not triangulate?");

            // Loop over vertices in the face.
            glm::ivec3 triangle;
            for (size_t vi = 0; vi < fv; vi++) { // VERTICES IN TRIANGLES
                Vertex currentVertex;

                index_t idx = shapes[s].mesh.indices[index_offset + vi];
                currentVertex.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                currentVertex.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                currentVertex.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    currentVertex.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    currentVertex.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    currentVertex.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    currentVertex.uv.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    currentVertex.uv.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                // Optional: vertex colors
                // real_t r = attrib.colors[3 * size_t(idx.vertex_index) + 0];
                // real_t g = attrib.colors[3 * size_t(idx.vertex_index) + 1];
                // real_t b = attrib.colors[3 * size_t(idx.vertex_index) + 2];

                const auto& foundCell = pointHashMap.find(currentVertex);

                if (foundCell != pointHashMap.end()) { // if found
                    triangle[vi] = foundCell->second;
                } else {
                    pointHashMap.insert({ currentVertex, indexOffset });
                    triangle[vi] = indexOffset;
                    indexOffset++;
                }
            }
            resultMesh.triangles.push_back(triangle);
            index_offset += fv;

            // per-face material
            // shapes[s].mesh.material_ids[f];
        }

        resultMesh.vertices.resize(pointHashMap.size());

        for (const auto& p : pointHashMap)
            resultMesh.vertices[p.second] = p.first;

        resultMeshes.push_back(std::move(resultMesh));
    }
    return resultMeshes;
}
