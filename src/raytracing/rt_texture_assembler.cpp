#include "rt_texture_assembler.h"

#include "../src/graphics/gl_texture.h"
#include "../src/helper_general.h"

#include "utils.h"
#include <glm/gtc/packing.hpp>
#include <iostream>

// reinterpret integers (tri indices, vert indices) to float texture
// advantage - create bigger than 16777216 pixel texture
// drawbacks - some drivers do calculation with NEAREST texture sample as well, so data can be corrupted
// to fetch int data in shader - use floatBitsToInt(...)
// apply the same macro in raytracing shader!

#define REINTERPRET_FLOAT_DATA
// Nvidia propietary 530 driver works fine, Intel Mesa - does some mess.

struct RTTextureData32 {
    GLTexture2D::Format format;
    std::vector<float> buffer;
    glm::ivec2 size;
};

struct RTTextureData16 {
    GLTexture2D::Format format;
    std::vector<int16_t> buffer;
    glm::ivec2 size;
};

static RTTextureData32 createGeometryTexture(const BVH::BVHBuilder& bvh, const Model3D& model)
{
    constexpr int floatsPerPixel = 4;
    constexpr int nFloatsInNode = 8;
    constexpr int nFloatsInIndex = 4;
    constexpr int nFloatsInVertex = 8;
    constexpr int nPixelPerNode = nFloatsInNode / floatsPerPixel;
    constexpr int nPixelPerVertex = nFloatsInVertex / floatsPerPixel;

    // calculate index buffer size
    int numOfFloatsInNodeArray = bvh.getNodes().size() * nFloatsInNode;
    int nodePixelCount = numOfFloatsInNodeArray / floatsPerPixel;

    // calculate index buffer size
    int numOfFloatsInIndexArray = model.triangles.size() * nFloatsInIndex;
    int indexPixelCount = numOfFloatsInIndexArray / floatsPerPixel;

    // calculate vertex buffer size
    int numOfFloatsInVertexArray = model.vertices.size() * nFloatsInVertex;
    int vertexPixelCount = numOfFloatsInVertexArray / floatsPerPixel;

    int overallPixelCount = nodePixelCount + indexPixelCount + vertexPixelCount;
    overallPixelCount = Utils::powerOfTwo(overallPixelCount);

#ifndef REINTERPRET_FLOAT_DATA
    assert(overallPixelCount <= 16777216 && "You can not sample more than 16777216 index with float indices");
#endif

    const int textureWidth = Utils::powerOfTwo(sqrtf(overallPixelCount));
    const int textureHeight = overallPixelCount / textureWidth;

    int floatOffset = 0;

    std::vector<float> buffer;
    buffer.resize(textureHeight * textureWidth * floatsPerPixel, 0);

    const int nodeIndexPixelOffset = nodePixelCount;

    for (int i = 0; i < bvh.getNodes().size(); ++i) {
        const auto& n = bvh.getNodes()[i];

        int leftChildIndex = (n.leftChild <= 0)
            ? n.leftChild - nodeIndexPixelOffset // if triangle
            : n.leftChild * nPixelPerNode; //  if node

        int rightChildIndex = (n.rightChild <= 0)
            ? n.rightChild - nodeIndexPixelOffset
            : n.rightChild * nPixelPerNode;

        // first pixel
#ifdef REINTERPRET_FLOAT_DATA
        buffer[i * nFloatsInNode + 0] = reinterpret_cast<float&>(leftChildIndex);
        buffer[i * nFloatsInNode + 1] = reinterpret_cast<float&>(rightChildIndex);
#else
        buffer[i * nFloatsInNode + 0] = float(leftChildIndex);
        buffer[i * nFloatsInNode + 1] = float(rightChildIndex);
#endif

        buffer[i * nFloatsInNode + 2] = n.aabb.getMin().x;
        buffer[i * nFloatsInNode + 3] = n.aabb.getMin().y;

        // second pixel
        buffer[i * nFloatsInNode + 4] = n.aabb.getMin().z;
        buffer[i * nFloatsInNode + 5] = n.aabb.getMax().x;
        buffer[i * nFloatsInNode + 6] = n.aabb.getMax().y;
        buffer[i * nFloatsInNode + 7] = n.aabb.getMax().z;
    }
    floatOffset += numOfFloatsInNodeArray;

    const int triIndexPixelOffset = nodePixelCount + indexPixelCount;

    for (int i = 0; i < model.triangles.size(); ++i) {
        const auto& t = model.triangles[i];

        int t0 = t[0] * nPixelPerVertex + triIndexPixelOffset;
        int t1 = t[1] * nPixelPerVertex + triIndexPixelOffset;
        int t2 = t[2] * nPixelPerVertex + triIndexPixelOffset;

#ifdef REINTERPRET_FLOAT_DATA
        buffer[floatOffset + i * nFloatsInIndex + 0] = reinterpret_cast<float&>(t0);
        buffer[floatOffset + i * nFloatsInIndex + 1] = reinterpret_cast<float&>(t1);
        buffer[floatOffset + i * nFloatsInIndex + 2] = reinterpret_cast<float&>(t2);
#else
        buffer[floatOffset + i * nFloatsInIndex + 0] = float(t0);
        buffer[floatOffset + i * nFloatsInIndex + 1] = float(t1);
        buffer[floatOffset + i * nFloatsInIndex + 2] = float(t2);
#endif

        buffer[floatOffset + i * nFloatsInIndex + 3] = 0;
    }
    floatOffset += numOfFloatsInIndexArray;

    for (int i = 0; i < model.vertices.size(); ++i) {
        const auto& v = model.vertices[i];

        // first pixel
        buffer[floatOffset + i * nFloatsInVertex + 0] = v.position.x;
        buffer[floatOffset + i * nFloatsInVertex + 1] = v.position.y;
        buffer[floatOffset + i * nFloatsInVertex + 2] = v.position.z;
        buffer[floatOffset + i * nFloatsInVertex + 3] = v.normal.x;

        // second pixel
        buffer[floatOffset + i * nFloatsInVertex + 4] = v.normal.y;
        buffer[floatOffset + i * nFloatsInVertex + 5] = v.normal.z;
        buffer[floatOffset + i * nFloatsInVertex + 6] = v.uv.x;
        buffer[floatOffset + i * nFloatsInVertex + 7] = v.uv.y;
    }

    LOG("Node pixel count: " << nodePixelCount
                             << ", Index pixel count: " << indexPixelCount
                             << ", Vertex pixel count: " << vertexPixelCount);

    LOG("TextureResolution: " << textureWidth << "x" << textureHeight);

    RTTextureData32 data;
    data.format = GLTexture2D::Format ::RGBA_32F;
    data.buffer = std::move(buffer);
    data.size.x = textureWidth;
    data.size.y = textureHeight;
    return data;
}

static RTTextureData16 createGeometryTextureNormalized(const BVH::BVHBuilder& bvh, const Model3D& model)
{
    glm::vec3 scale, offset;

    auto bbMin = bvh.getNodes()[0].aabb.getMin();
    auto bbMax = bvh.getNodes()[0].aabb.getMax();
    //(x - min(x)) / ( max(x) - min(x) )

    auto normalize = [&](glm::vec3 pos) {
        return (pos - bbMin) / (bbMax - bbMin);
    };

    constexpr int nUints16PerPixel = 8;
    constexpr int nUints16InNode = 8;
    constexpr int nUints16InIndex = 8;
    constexpr int nUints16InVertex = 8;
    constexpr int nPixelPerNode = nUints16InNode / nUints16PerPixel;
    constexpr int nPixelPerVertex = nUints16InVertex / nUints16PerPixel;

    // calculate index buffer size
    int numOfUints16InNodeArray = bvh.getNodes().size() * nUints16InNode;
    int nodePixelCount = numOfUints16InNodeArray / nUints16PerPixel;

    // calculate index buffer size
    // int numOfUints16InIndexArray = ((int)model.triangles.size() - 1) / 2 + 1;
    int numOfUints16InIndexArray = model.triangles.size() * nUints16InIndex;
    int indexPixelCount = numOfUints16InIndexArray / nUints16PerPixel;

    // calculate vertex buffer size
    int numOfUints16InVertexArray = model.vertices.size() * nUints16InVertex;
    int vertexPixelCount = numOfUints16InVertexArray / nUints16PerPixel;

    int overallPixelCount = nodePixelCount + indexPixelCount + vertexPixelCount;
    overallPixelCount = Utils::powerOfTwo(overallPixelCount);

    const int textureWidth = Utils::powerOfTwo(sqrtf(overallPixelCount));
    const int textureHeight = overallPixelCount / textureWidth;

    std::vector<int16_t> buffer;
    buffer.resize(textureHeight * textureWidth * nUints16PerPixel, 0);

    const int nodeIndexPixelOffset = nodePixelCount;

    for (int i = 0; i < bvh.getNodes().size(); ++i) {
        const auto& n = bvh.getNodes()[i];

        int childIndexL = (n.leftChild <= 0)
            ? n.leftChild - nodeIndexPixelOffset // if triangle
            : n.leftChild * nPixelPerNode; //  if node

        int childIndexR = (n.rightChild <= 0)
            ? n.rightChild - nodeIndexPixelOffset
            : n.rightChild * nPixelPerNode;

        assert(childIndexL >= -32768 && childIndexL <= 32767);
        assert(childIndexR >= -32768 && childIndexR <= 32767);

        auto normalizedMin = normalize(n.aabb.getMin());
        auto normalizedMax = normalize(n.aabb.getMax());

        // R
        buffer[i * nUints16InNode + 0] = (int16_t)childIndexL;
        buffer[i * nUints16InNode + 1] = (int16_t)childIndexR;

        glm::ivec3 aabbMin(
            (int16_t)glm::packUnorm1x16(normalizedMin.x),
            (int16_t)glm::packUnorm1x16(normalizedMin.y),
            (int16_t)glm::packUnorm1x16(normalizedMin.z));

        glm::ivec3 aabbMax(
            (int16_t)glm::packUnorm1x16(normalizedMax.x),
            (int16_t)glm::packUnorm1x16(normalizedMax.y),
            (int16_t)glm::packUnorm1x16(normalizedMax.z));

        // GBA
        buffer[i * nUints16InNode + 2] = aabbMin.x;
        buffer[i * nUints16InNode + 3] = aabbMin.y;
        buffer[i * nUints16InNode + 4] = aabbMin.z;
        buffer[i * nUints16InNode + 5] = aabbMax.x;
        buffer[i * nUints16InNode + 6] = aabbMax.y;
        buffer[i * nUints16InNode + 7] = aabbMax.z;
    }

    int dataOffset = numOfUints16InNodeArray;

    const int triIndexPixelOffset = nodePixelCount + indexPixelCount;

    for (int i = 0; i < model.triangles.size(); ++i) {
        const auto& t = model.triangles[i];

        int t0 = t[0] * nPixelPerVertex + triIndexPixelOffset;
        int t1 = t[1] * nPixelPerVertex + triIndexPixelOffset;
        int t2 = t[2] * nPixelPerVertex + triIndexPixelOffset;

        assert(t0 <= 32767 && t1 <= 32767 && t2 <= 32767);
        assert(t0 >= -32768 && t1 >= -32768 && t2 >= -32768);

        // R
        buffer[dataOffset + i * nUints16InIndex + 0] = (int16_t)t0;
        buffer[dataOffset + i * nUints16InIndex + 1] = (int16_t)t1;
        buffer[dataOffset + i * nUints16InIndex + 2] = (int16_t)t2;
        buffer[dataOffset + i * nUints16InIndex + 3] = 0;
    }
    dataOffset += numOfUints16InIndexArray;

    for (int i = 0; i < model.vertices.size(); ++i) {
        const auto& v = model.vertices[i];

        auto normalizedPos = normalize(v.position);

        buffer[dataOffset + i * nUints16InVertex + 0] = glm::packUnorm1x16(normalizedPos.x);
        buffer[dataOffset + i * nUints16InVertex + 1] = glm::packUnorm1x16(normalizedPos.y);
        buffer[dataOffset + i * nUints16InVertex + 2] = glm::packUnorm1x16(normalizedPos.z);
        buffer[dataOffset + i * nUints16InVertex + 3] = glm::packUnorm1x16(v.normal.x);
        buffer[dataOffset + i * nUints16InVertex + 4] = glm::packUnorm1x16(v.normal.y);
        buffer[dataOffset + i * nUints16InVertex + 5] = glm::packUnorm1x16(v.normal.z);
        buffer[dataOffset + i * nUints16InVertex + 6] = glm::packUnorm1x16(v.uv.x);
        buffer[dataOffset + i * nUints16InVertex + 7] = glm::packUnorm1x16(v.uv.y);
    }

    LOG("Node pixel count: " << nodePixelCount
                             << ", Index pixel count: " << indexPixelCount
                             << ", Vertex pixel count: " << vertexPixelCount);

    LOG("TextureResolution: " << textureWidth << "x" << textureHeight);

    RTTextureData16 data;
    data.format = GLTexture2D::Format ::RGBA_32F;
    data.buffer = std::move(buffer);
    data.size.x = textureWidth;
    data.size.y = textureHeight;
    return data;
}

GLTexture2D RTTextureAssembler::assemble(const Model3D& model, BVH::BVHBuilder& bvh)
{
    bvh.build(model);
    RTTextureData16 textureData = createGeometryTextureNormalized(bvh, model);
    return GLTexture2D(textureData.size, textureData.format, textureData.buffer.data());
}
