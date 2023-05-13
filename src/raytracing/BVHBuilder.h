#pragma once

#include "../src/graphics/model3d.h"
#include <functional>
#include <glm/vec3.hpp> //GLM
#include <vector>

#include <glm/glm.hpp>

namespace BVH {

struct AABB {
private:
    glm::vec3 min, max;

public:
    AABB()
        : min({ 0, 0, 0 })
        , max({ 0, 0, 0 })
    {
    }

    AABB(glm::vec3 min, glm::vec3 max)
        : min(min)
        , max(max)
    {
    }

    void surrounding(AABB const& aabb)
    {
        min = glm::min(min, aabb.min);
        max = glm::max(max, aabb.max);
    }

    const glm::vec3& getMin() const { return min; }
    const glm::vec3& getMax() const { return max; }
};

struct Triangle {
private:
    glm::vec3 vertex1;
    glm::vec3 vertex2;
    glm::vec3 vertex3;
    int index;
    glm::vec3 center;
    AABB aabb;

public:
    Triangle(glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3 vertex3,
        int index, glm::ivec3 triIndex)
        : vertex1(vertex1)
        , vertex2(vertex2)
        , vertex3(vertex3)
        , index(index)
    {
        aabb = genAABB();
        center = genCenter();
    }

    Triangle()
        : Triangle(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 0), 0, glm::ivec3(0)) {};

    glm::vec3& getCenter() { return center; }
    AABB& getAABB() { return aabb; }
    int getIndex() { return index; }

    glm::vec3 getCenter() const { return center; }
    AABB getAABB() const { return aabb; }
    int getIndex() const { return index; }

private:
    glm::vec3 genCenter()
    {
        glm::vec3 sum = vertex1 + vertex2 + vertex3;
        glm::vec3 centerDim = sum / 3.0f;
        return centerDim;
    }

    AABB genAABB()
    {
        return AABB(
            glm::min(glm::min(vertex1, vertex2), vertex3),
            glm::max(glm::max(vertex1, vertex2), vertex3));
    }
};

struct Node {
    int leftChild, rightChild;
    AABB aabb;

    Node()
        : leftChild(0)
        , rightChild(0)
        , aabb(glm::vec3(0), glm::vec3(0))
    {
        constexpr int size = sizeof(*this) / sizeof(float);
    }
};

struct Node;
class BVHBuilder {
public:
    BVHBuilder();
    void build(const Model3D& model);

    const std::vector<Node>& getNodes() const { return nodeList; }

private:
    void buildRecurcive(int nodeIndex, std::vector<Triangle> const& vecTriangle);

    int texSize;
    std::vector<Node> nodeList;
    std::vector<Triangle> vecTriangle;
};
}
