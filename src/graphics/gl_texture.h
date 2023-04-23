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
    enum class Filtering : uint8_t { Nearset, Linear, LinearMipmap };
    // clang-format on

public:
    GLTexture() = default;
    ~GLTexture();

    bool setWrapping(Wrapping);
    bool setFiltering(Filtering);

    uint32_t getHandle() const { return m_textureHandle; }
    glm::vec2 getSize() const { return m_size; }

protected:
    bool createEmpty(glm::ivec2 size);
    bool fromImage(const Image& img);
    void clear();

protected:
    glm::ivec2 m_size = glm::ivec2(0);
    uint32_t m_textureHandle {};
    Format m_internalFormat = Format::RGBA;
    Filtering m_filtering = Filtering::Nearset;

    static uint32_t s_currentBindedTexture;
    friend class GLTextureInstance;
    friend class GLFrameBuffer;
};

class GLTextureInstance {
    std::shared_ptr<GLTexture> m_texture;

public:
    GLTextureInstance(glm::ivec2 size) { init(), createEmpty(size); }
    GLTextureInstance(const Image& img) { init(), fromImage(img); }
    GLTextureInstance() {};
    // clang-format off
    void init() { m_texture = std::make_shared<GLTexture>(); }
    bool createEmpty(glm::ivec2 size) { init(); return m_texture->createEmpty(size); }
    bool fromImage(const Image& img) { init(); return m_texture->fromImage(img); }
    // clang-format on

    operator std::shared_ptr<GLTexture>() { return m_texture; }

    GLTexture* get() { return m_texture.get(); }
    const GLTexture* get() const { return m_texture.get(); }
};

#endif // GLTEXTURE_H
