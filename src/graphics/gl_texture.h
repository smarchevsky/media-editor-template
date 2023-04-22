#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "helper_general.h"
#include "image.h"

class GLTexture : NoCopy<GLTexture> {
public:
    // clang-format off
    enum class Format : uint8_t { RGB, RGBA };
    enum class Wrapping : uint8_t { Repeat, MirrorRepeat, ClampEdge, ClampBorder };
    enum class Filtering : uint8_t { Nearset, Linear, LinearMipmap };
    // clang-format on

public:
    GLTexture() = default;
    GLTexture(const Image& img) { fromImage(img); }
    ~GLTexture();

    bool createEmpty(glm::ivec2 size);
    bool fromImage(const Image& img);
    void clear();

    bool setWrapping(Wrapping);
    bool setFiltering(Filtering);

    uint32_t getHandle() const { return m_textureHandle; }

protected:
    uint32_t m_textureHandle {};
    Format m_internalFormat = Format::RGBA;
    Filtering m_filtering = Filtering::Nearset;

    static uint32_t s_currentBindedTexture;
};

#endif // GLTEXTURE_H
