#include "image.h"
#include <glm/common.hpp>

// #define TINYEXR_USE_STB_ZLIB
// #include "tinyexr.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

// #define LOG_VERBOSE

void Image::STBIDeleter::operator()(uint8_t* data) const { stbi_image_free(data); }

Image::Image(glm::ivec2 size, TexelFormat format)
{
    m_size = size;
    m_format = format;
    assert(m_format != TexelFormat::Undefined);

    TexelFormatInfo texelInfo(m_format);
    size_t imgSizeBytes = m_size.x * m_size.y * texelInfo.sizeInBytes;
    m_data.reset((uint8_t*)stbi__malloc(imgSizeBytes));
}

Image::~Image() { clear(); }

void Image::load(const std::filesystem::path& path)
{
    clear();
    // stbi_set_flip_vertically_on_load(true);
    int nrChannels = 0;
    m_data.reset(stbi_load(path.c_str(), &m_size.x, &m_size.y, &nrChannels, 0));

    if (m_data) {
        switch (nrChannels) {
        case 1: {
            m_format = TexelFormat::R_8;
        } break;
        case 3: {
            m_format = TexelFormat::RGB_8;
        } break;
        case 4: {
            m_format = TexelFormat::RGBA_8;
        } break;
        default: {
            assert(false && "Unsupported num channels (probably 2)");
        } break;
        }

        m_name = path.filename();

#ifdef LOG_VERBOSE
        LOG("Image created: file name: " << m_name << " , nrChannels: " << m_nrChannels);
#endif

    } else {
        LOGE("Failed to load texture from path: " << path);
    }
}

void Image::fill(glm::ivec2 size, int32_t packedColor)
{
    clear();
    assert(size.x > 0 && size.y > 0 && "Image size must be positive");
    m_size = size;
    m_format = TexelFormat::RGBA_8;
    TexelFormatInfo texelInfo(m_format);

    glm::ivec4 color(
        (packedColor & 0xFF000000) >> 24,
        (packedColor & 0x00FF0000) >> 16,
        (packedColor & 0x0000FF00) >> 8,
        (packedColor & 0x000000FF));

    size_t img_size = m_size.x * m_size.y * texelInfo.sizeInBytes;

    m_data.reset((uint8_t*)stbi__malloc(img_size));
    assert(m_data && "Cannot allocate memory");

    for (uint8_t* p = m_data.get(); p < m_data.get() + img_size; p += texelInfo.sizeInBytes) {
        p[0] = color[0], p[1] = color[1], p[2] = color[2], p[3] = color[3];
    }

    m_name = "Img: size: " + glm::to_string(size)
        + " , color: " + glm::to_string(color)
        + ", channels: " + std::to_string(texelInfo.numChannels);

#ifdef LOG_VERBOSE
    LOG("Image created: " << m_name);
#endif
}

void Image::clear()
{
    if (m_data) {
        m_data.reset();
        m_size = glm::ivec2(0);
        m_format = TexelFormat::Undefined;

#ifdef LOG_VERBOSE
        LOG("Image destroyed: " << m_name);
#endif
        m_name.clear();
    }
}

bool Image::isValid() const { return m_data && m_size.x > 0 && m_size.y > 0 && (m_format != TexelFormat::Undefined); }

bool Image::writeToFile(const std::filesystem::path& path, bool flipVertically) const
{
    if (!isValid())
        return false;

    TexelFormatInfo texelInfo(m_format);
    stbi_flip_vertically_on_write(flipVertically);
    if (path.extension() == ".jpg") {
        return stbi_write_jpg(path.c_str(), m_size.x, m_size.y, texelInfo.numChannels, getData(), 96 /* default stbi quality is 90 */);
    } else if (path.extension() == ".png") {
        return stbi_write_png(path.c_str(), m_size.x, m_size.y, texelInfo.numChannels, getData(), m_size.x * texelInfo.numChannels);
    }

    return false;
}
size_t Image::getDataSize() const { return m_size.x * m_size.y * TexelFormatInfo(m_format).sizeInBytes; }
