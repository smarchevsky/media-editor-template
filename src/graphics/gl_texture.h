#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "common.h"
#include "image.h"
#include "texelformat.h"

#include <vector>

class GLTextureInstance;

class GLTextureBase : NoCopy<GLTextureBase> {
public:
    virtual ~GLTextureBase() = default;
};

/////////////////// TEXTURE 2D ////////////////////

class GLTexture2D : public GLTextureBase {
public:
    // clang-format off
    enum class Wrapping : uint8_t { Repeat, MirrorRepeat, ClampEdge, ClampBorder };
    enum class Filtering : uint8_t { Nearest, NearestMipmap, Linear, LinearMipmap };
    // clang-format on
protected:
    uint32_t m_textureHandle {};
    glm::ivec2 m_size = glm::ivec2(0);

    TexelFormat m_format = TexelFormat::Undefined;
    Filtering m_filtering = Filtering::Nearest;
    Wrapping m_wrapping = Wrapping::Repeat;

    mutable bool m_mipmapDirty = false; // used for framebuffer

public:
    GLTexture2D& operator=(GLTexture2D&& rhs) // it seems, you can swap textures with no problems
    {
        m_textureHandle = rhs.m_textureHandle, rhs.m_textureHandle = 0;

        m_size = rhs.m_size;
        m_format = rhs.m_format;
        m_filtering = rhs.m_filtering;
        m_wrapping = rhs.m_wrapping;

        m_mipmapDirty = rhs.m_mipmapDirty;
        return *this;
    }

    GLTexture2D() = default;
    GLTexture2D(const Image& img) { fromImage(img); }
    GLTexture2D(glm::ivec2 size, TexelFormat format, void* data = nullptr) { createFromRawData(size, format, data); }
    GLTexture2D(GLTexture2D&& rhs) { *this = std::move(rhs); }
    ~GLTexture2D();

    void createFromRawData(glm::ivec2 size, TexelFormat format, const void* data);

    // update texture from CPU, data must be size of  width * height * format size (byte/float * channelsNum)
    void updateData(void* data);

    void setWrapping(Wrapping);
    void setFiltering(Filtering);

    Filtering getFiltering() const { return m_filtering; }
    Wrapping getWrapping() const { return m_wrapping; }
    TexelFormat getFormat() const { return m_format; }
    uint32_t getHandle() const { return m_textureHandle; }
    glm::ivec2 getSize() const { return m_size; }

    Image toImage(TexelFormat format = TexelFormat::Undefined); // if undefined - use texture format, else convert
    static Image getImage(uint32_t glTextureTarget, glm::ivec2 size, TexelFormat format);

protected:
    void generateMipMapsIfDirty();
    bool fromImage(const Image& img);

    // static uint32_t s_currentBindedTextureHandle;
    friend class GLFrameBuffer;
    friend class GLShader;
};

/////////////////// RENDER (DEPTH) BUFFER /////////////////

class GLDepthBuffer2D : public GLTextureBase {
    uint32_t m_rbo {};

public:
    void create(glm::ivec2 size);
    uint32_t getHandle() const { return m_rbo; }
    GLDepthBuffer2D(glm::ivec2 size) { create(size); }
    ~GLDepthBuffer2D();
};

#endif // GLTEXTURE_H
