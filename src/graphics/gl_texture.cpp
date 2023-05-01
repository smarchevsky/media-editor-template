#include "gl_texture.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <iostream>
// #include <cassert>

uint32_t GLTexture2D::s_currentBindedTexture = 0;

static int getGLTextureFormatInternal(GLTexture2D::Format format)
{
    switch (format) {
    case GLTexture2D::Format::R8G8B8:
        return GL_RGB;
    case GLTexture2D::Format::R8G8B8A8:
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

GLDepthBuffer2D::GLDepthBuffer2D(glm::vec2 size)
{
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);

}

GLDepthBuffer2D::~GLDepthBuffer2D()
{
    if (m_rbo) {
        glDeleteTextures(1, &m_rbo);
        m_rbo = 0;
        LOG("Render (depth) buffer destroyed");
    }
}

GLTexture2D::~GLTexture2D()
{
    clear();
}

bool GLTexture2D::create(glm::ivec2 size)
{
    if (size.x <= 0 || size.y <= 0)
        return false;

    m_size = size;

    clear();
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0,
        GL_RGB, GL_UNSIGNED_BYTE, nullptr); // input data seems useless

    setFiltering(Filtering::Nearset);
    LOG("Texture successfully created empty");

    return true;
}

bool GLTexture2D::fromImage(const Image& img)
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
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    m_size = img.m_size;
    glTexImage2D(GL_TEXTURE_2D, 0, internalTextureFormat, m_size.x, m_size.y,
        0 /* border? */, externalTextureFormat, GL_UNSIGNED_BYTE, img.m_data);

    setWrapping(Wrapping::Repeat);
    setFiltering(Filtering::LinearMipmap);
    LOG("Texture successfully loaded from image");
    return true;
}

void GLTexture2D::clear()
{
    if (m_textureHandle) {
        glDeleteTextures(1, &m_textureHandle);
        m_textureHandle = 0;
        m_size = glm::ivec2(0);
        LOG("Texture destroyed");
    }
}

bool GLTexture2D::setWrapping(Wrapping wrapping)
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

    m_wrapping = wrapping;
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingGLformat);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingGLformat);
    return true;
}
static bool isMipMap(GLTexture2D::Filtering filtering)
{
    return filtering == GLTexture2D::Filtering::NearestMipmap
        || filtering == GLTexture2D::Filtering::LinearMipmap;
}

void GLTexture2D::generateMipMap()
{
    // glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST /*GL_FASTEST*/);
    if (m_textureHandle && isMipMap(m_filtering)) {
        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

bool GLTexture2D::setFiltering(Filtering filtering)
{
    if (!m_textureHandle) {
        LOGE("No texture handle");
        return false;
    }

    int minFilter, magFilter;
    switch (filtering) {
    case Filtering::Nearset: {
        minFilter = GL_NEAREST;
        magFilter = GL_NEAREST;
    } break;

    case Filtering::NearestMipmap: {
        minFilter = GL_NEAREST_MIPMAP_LINEAR;
        magFilter = GL_NEAREST;
    } break;

    case Filtering::Linear: {
        minFilter = GL_LINEAR;
        magFilter = GL_LINEAR;
    } break;

    case Filtering::LinearMipmap: {
        minFilter = GL_LINEAR_MIPMAP_LINEAR;
        magFilter = GL_LINEAR;

    } break;
    default:
        LOGE("Unsupportering texture filtering type");
        return false;
    }

    m_filtering = filtering;

    generateMipMap();
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    return true;
}
