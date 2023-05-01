#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "helper_general.h"
#include "image.h"

class GLTextureInstance;

class GLTextureBase : NoCopy<GLTextureBase> {
public:
    virtual ~GLTextureBase() = default;
};
/////////////////// RENDER (DEPTH) BUFFER /////////////////

class GLDepthBuffer2D : public GLTextureBase {
    uint32_t m_rbo {};

public:
    uint32_t getHandle() const { return m_rbo; }
    GLDepthBuffer2D(glm::vec2 size);
    ~GLDepthBuffer2D();
};

/////////////////// GLTexture2D ////////////////////

class GLTexture2D : public GLTextureBase {
public:
    // clang-format off
    enum class Format : uint8_t { R8G8B8, R8G8B8A8 };
    enum class Wrapping : uint8_t { Repeat, MirrorRepeat, ClampEdge, ClampBorder };
    enum class Filtering : uint8_t { Nearset, NearestMipmap, Linear, LinearMipmap };
    // clang-format on
protected:
    uint32_t m_textureHandle {};
    glm::ivec2 m_size = glm::ivec2(0);
    Format m_internalFormat = Format::R8G8B8A8;
    Filtering m_filtering = Filtering::Nearset;
    Wrapping m_wrapping = Wrapping ::Repeat;

public:
    GLTexture2D() = default;
    GLTexture2D(const Image& img) { fromImage(img); }
    GLTexture2D(glm::ivec2 size) { create(size); }
    ~GLTexture2D();

    bool setWrapping(Wrapping);
    bool setFiltering(Filtering);
    // Wrapping getWrapping() {return m_w}

    uint32_t getHandle() const { return m_textureHandle; }
    glm::vec2 getSize() const { return m_size; }

protected:
    void generateMipMap();
    bool create(glm::ivec2 size);
    bool fromImage(const Image& img);
    void clear();

    static uint32_t s_currentBindedTexture;
    friend class GLTextureInstance;
    friend class GLFrameBuffer;
};

#endif // GLTEXTURE_H
