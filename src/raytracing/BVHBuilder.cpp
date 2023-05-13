#include "BVHBuilder.h"
#include <algorithm>
#include <stack>
using glm::vec3;

namespace BVH {
BVHBuilder::BVHBuilder() { }

void BVHBuilder::build(const Model3D& model)
{
    size_t nodeSize = sizeof(Node);
    nodeList.push_back(Node());

    const auto& v = model.vertices;
    for (int i = 0; i < model.triangles.size(); ++i) {
        const int i0 = model.triangles[i].x;
        const int i1 = model.triangles[i].y;
        const int i2 = model.triangles[i].z;
        vecTriangle.emplace_back(v[i0].position, v[i1].position, v[i2].position, i, model.triangles[i]);
    }

    nodeList.reserve(vecTriangle.size());
    buildRecurcive(0, vecTriangle);
}

void BVHBuilder::buildRecurcive(int nodeIndex, std::vector<Triangle> const& vecTriangle)
{
    // Build Bpun box for triangles in vecTriangle
    AABB tempaabb = vecTriangle[0].getAABB();
    for (Triangle const& tri : vecTriangle)
        tempaabb.surrounding(tri.getAABB());

    Node& node = nodeList[nodeIndex];
    node.aabb = tempaabb;

    if (vecTriangle.size() == 2) {
        node.leftChild = -vecTriangle[0].getIndex();
        node.rightChild = -vecTriangle[1].getIndex();
        return;
    }

    // seach max dimenson for split
    vec3 maxVec = vecTriangle[0].getCenter();
    vec3 minVec = vecTriangle[0].getCenter();
    vec3 centerSum(0, 0, 0);

    for (Triangle const& tri : vecTriangle) {
        maxVec = glm::max(tri.getCenter(), maxVec);
        minVec = glm::min(tri.getCenter(), minVec);
        centerSum += tri.getCenter();
    }
    vec3 midPoint = centerSum / (float)vecTriangle.size();
    vec3 len = glm::abs(maxVec - minVec);

    int axis = 0;

    if (len.y > len.x)
        axis = 1;

    if (len.z > len.y && len.z > len.x)
        axis = 2;

    std::vector<Triangle> tempLeftTriangleList;
    std::vector<Triangle> tempRightTriangleList;

    auto splitByAxis = [&](std::function<float(vec3 const& point)> getElement) {
        for (Triangle const& tri : vecTriangle) {
            if (getElement(tri.getCenter()) < getElement(midPoint))
                tempLeftTriangleList.push_back(tri);
            else
                tempRightTriangleList.push_back(tri);
        }
        assert(tempLeftTriangleList.size());
        assert(tempRightTriangleList.size());
    };

    using namespace std::placeholders;

    if (axis == 0)
        splitByAxis(bind(&vec3::x, _1));

    if (axis == 1)
        splitByAxis(bind(&vec3::y, _1));

    if (axis == 2)
        splitByAxis(bind(&vec3::z, _1));

    const auto& tl0 = tempLeftTriangleList[0];
    const auto& tr0 = tempRightTriangleList[0];
    if (tempLeftTriangleList.size() == 1) {
        node.leftChild = -tl0.getIndex();

    } else {
        node.leftChild = (int)nodeList.size();
        nodeList.emplace_back();
        buildRecurcive(nodeList.size() - 1, tempLeftTriangleList);
    }

    if (tempRightTriangleList.size() == 1) {
        node.rightChild = -tr0.getIndex();

    } else {
        node.rightChild = (int)nodeList.size();
        nodeList.emplace_back();
        buildRecurcive(nodeList.size() - 1, tempRightTriangleList);
    }
}
}
