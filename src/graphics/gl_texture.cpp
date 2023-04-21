#include "gl_texture.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <iostream>

uint32_t GLTexture::s_currentBindedTexture = 0;

static int getGLTextureFormatInternal(GLTexture::Format format)
{
    switch (format) {
    case GLTexture::Format::RGB:
        return GL_RGB;
    case GLTexture::Format::RGBA:
        return GL_RGBA;
    default:
        LOGE("Texture internal format: " << (int)format << " not supported");
        return 0;
    }
}
static int getGLTextureFormatExternal(int nrChannels)
{
    switch (nrChannels) {
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        LOGE("Texture format with: " << nrChannels << " channels not supported");
        return 0;
    }
}

GLTexture::~GLTexture()
{
    clear();
}

bool GLTexture::createEmpty(glm::ivec2 size)
{
    if (size.x <= 0 || size.y <= 0)
        return false;

    clear();
    glGenTextures(1, &m_textureHandle);
    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0,
        GL_RGB, GL_UNSIGNED_BYTE, nullptr); // input data seems useless

    setFiltering(Filtering::Nearset);

    return true;
}

bool GLTexture::fromImage(const Image& img)
{
    if (!img.isValid()) {
        LOGE("trying to load invalid texture");
        return false;
    }

    int internalTextureFormat = getGLTextureFormatInternal(m_internalFormat);
    if (!internalTextureFormat)
        return false;

    int externalTextureFormat = getGLTextureFormatExternal(img.m_nrChannels);
    if (!externalTextureFormat)
        return false;

    clear();
    glGenTextures(1, &m_textureHandle);
    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, internalTextureFormat, img.m_size.x, img.m_size.y,
        0 /* border? */, externalTextureFormat, GL_UNSIGNED_BYTE, img.m_data);

    setWrapping(Wrapping::Repeat);
    setFiltering(Filtering::LinearMipmap);
    return true;
}

void GLTexture::clear()
{
    if (m_textureHandle) {
        glDeleteTextures(1, &m_textureHandle);
        m_textureHandle = 0;
    }
}

bool GLTexture::setWrapping(Wrapping wrapping)
{
    if (!m_textureHandle) {
        LOGE("No texture handle");
        return false;
    }

    int wrappingGLformat {};
    switch (wrapping) {
    case Wrapping::Repeat: {
        wrappingGLformat = GL_REPEAT;
    } break;
    case Wrapping::MirrorRepeat: {
        wrappingGLformat = GL_MIRRORED_REPEAT;
    } break;
    case Wrapping::ClampEdge: {
        wrappingGLformat = GL_CLAMP_TO_EDGE;
    } break;
    case Wrapping::ClampBorder: {
        wrappingGLformat = GL_CLAMP_TO_BORDER;
    } break;
    default:
        LOGE("Unsuppoerted wrapping type");
        return false;
    }

    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingGLformat);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingGLformat);
    return true;
}

bool GLTexture::setFiltering(Filtering filtering)
{
    if (!m_textureHandle) {
        LOGE("No texture handle");
        return false;
    }

    int minFilter, magFilter;
    switch (filtering) {
    case Filtering::Nearset: {
        minFilter = magFilter = GL_NEAREST;
    } break;
    case Filtering::Linear: {
        minFilter = magFilter = GL_LINEAR;
    } break;
    case Filtering::LinearMipmap: {
        minFilter = GL_LINEAR_MIPMAP_LINEAR;
        magFilter = GL_LINEAR;

        // glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST /*GL_FASTEST*/);
        if (m_textureHandle)
            glGenerateMipmap(GL_TEXTURE_2D);
    } break;
    default:
        LOGE("Unsupportering texture filtering type");
        return false;
    }
    m_filtering = filtering;
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    return true;
}

void GLTexture::bind()
{
#define ALWAYS_BIND
#ifdef ALWAYS_BIND
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
#else
    if (s_currentBindedTexture != m_textureHandle) {
        s_currentBindedTexture = m_textureHandle;
        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    }
#endif
}
