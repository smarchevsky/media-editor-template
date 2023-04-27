#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "helper_general.h"
#include "image.h"

class GLTextureInstance;
class GLTexture : NoCopy<GLTexture> {
public:
    // clang-format off
    enum class Format : uint8_t { RGB, RGBA };
    enum class Wrapping : uint8_t { Repeat, MirrorRepeat, ClampEdge, ClampBorder };
    enum class Filtering : uint8_t { Nearset, NearestMipmap, Linear, LinearMipmap };
    // clang-format on

public:
    GLTexture() = default;
    GLTexture(const Image& img) { fromImage(img); }
    ~GLTexture();

    bool setWrapping(Wrapping);
    bool setFiltering(Filtering);
    // Wrapping getWrapping() {return m_w}

    uint32_t getHandle() const { return m_textureHandle; }
    glm::vec2 getSize() const { return m_size; }

protected:
    void generateMipMap();
    bool createEmpty(glm::ivec2 size);
    bool fromImage(const Image& img);
    void clear();

protected:
    glm::ivec2 m_size = glm::ivec2(0);
    uint32_t m_textureHandle {};
    Format m_internalFormat = Format::RGBA;
    Filtering m_filtering = Filtering::Nearset;
    Wrapping m_wrapping = Wrapping ::Repeat;

    static uint32_t s_currentBindedTexture;
    friend class GLTextureInstance;
    friend class GLFrameBuffer;
};

#define SHARED_TEXTURE(x) std::make_shared<GLTexture>(x)

#endif // GLTEXTURE_H
