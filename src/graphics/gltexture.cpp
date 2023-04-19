#include "gltexture.h"

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <iostream>

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
    glDeleteTextures(1, &m_textureHandle);
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

    const GLenum texelType = GL_UNSIGNED_BYTE; // stbImage always gives uint8 image format

    glGenTextures(1, &m_textureHandle);

    bind();
    // set the texture wrapping parameters
    setOutOfBoundsBehavior(OutOfBoundsBehavior::Repeat);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps

    glTexImage2D(GL_TEXTURE_2D, 0,
        internalTextureFormat,
        img.m_size.x, img.m_size.y,
        0 /* border? */,
        externalTextureFormat, texelType, img.m_data);

    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST /*GL_FASTEST*/);
    glGenerateMipmap(GL_TEXTURE_2D);
    return true;
}

bool GLTexture::setOutOfBoundsBehavior(OutOfBoundsBehavior behavior)
{
    int GLBehaviorHandle {};
    switch (behavior) {
    case OutOfBoundsBehavior::Repeat: {
        GLBehaviorHandle = GL_REPEAT;
    } break;
    case OutOfBoundsBehavior::MirrorRepeat: {
        GLBehaviorHandle = GL_MIRRORED_REPEAT;
    } break;
    case OutOfBoundsBehavior::ClampEdge: {
        GLBehaviorHandle = GL_CLAMP_TO_EDGE;
    } break;
    case OutOfBoundsBehavior::ClampBorder: {
        GLBehaviorHandle = GL_CLAMP_TO_BORDER;
    } break;
    default:
        LOGE("Unsuppoerted OutOfBoundsBehavior type");
        return false;
    }
    // bind ?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLBehaviorHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLBehaviorHandle);
    return true;
}

void GLTexture::bind() { glBindTexture(GL_TEXTURE_2D, m_textureHandle); }
