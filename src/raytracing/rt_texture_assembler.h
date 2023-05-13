#ifndef RTTEXTUREASSEMBLER_H
#define RTTEXTUREASSEMBLER_H
#include "../src/graphics/model3d.h"
#include "BVHBuilder.h"
#include "graphics/gl_texture.h"

class RTTextureAssembler {
public:
    static GLTexture2D assemble(const Model3D& model, BVH::BVHBuilder& bvh);
};

#endif // RTTEXTUREASSEMBLER_H
