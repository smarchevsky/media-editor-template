#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "helper_general.h"
#include "image.h"

class GLTextureInstance;

class GLTextureBase : NoCopy<GLTextureBase> {
public:
    virtual ~GLTextureBase() = default;
};

/////////////////// TEXTURE 2D ////////////////////

class GLTexture2D : public GLTextureBase {
public:
    // clang-format off
    enum class Format : uint8_t { Undefined, R_8, RGB_8, RGBA_8, RGB_32F, RGBA_32F };
    enum class Wrapping : uint8_t { Repeat, MirrorRepeat, ClampEdge, ClampBorder };
    enum class Filtering : uint8_t { Nearset, NearestMipmap, Linear, LinearMipmap };
    // clang-format on
protected:
    uint32_t m_textureHandle {};
    glm::ivec2 m_size = glm::ivec2(0);
    Format m_format = Format::Undefined;
    Filtering m_filtering = Filtering::Nearset;
    Wrapping m_wrapping = Wrapping::Repeat;
    mutable bool m_mipmapDirty = false; // used for framebuffer

public:
    GLTexture2D() = default;
    GLTexture2D(const Image& img) { fromImage(img); }
    GLTexture2D(glm::ivec2 size, GLTexture2D::Format format, void* data = nullptr) { createFromRawData(size, format, data); }
    GLTexture2D(GLTexture2D&& rhs);
    ~GLTexture2D();

    bool createFromRawData(glm::ivec2 size, GLTexture2D::Format format, void* data);
    void setWrapping(Wrapping);
    void setFiltering(Filtering);

    uint32_t getHandle() const { return m_textureHandle; }
    glm::ivec2 getSize() const { return m_size; }

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
