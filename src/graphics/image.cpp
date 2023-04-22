#include "image.h"
#include <glm/common.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

Image::Image()
{
}

Image::~Image() { clear(); }

void Image::load(const std::filesystem::path& path)
{
    clear();
    // stbi_set_flip_vertically_on_load(true);
    m_data = stbi_load(path.c_str(), &m_size.x, &m_size.y, &m_nrChannels, 0);
    if (!m_data) {
        LOGE("Failed to load texture from path: " << path);
    }
}

void Image::fill(glm::ivec2 size, glm::ivec4 color)
{
    clear();
    assert(size.x > 0 && size.y > 0 && "Image size must be positive");
    m_size = size;
    m_nrChannels = 4;

    color = glm::clamp(color, glm::ivec4(0), glm::ivec4(255));

    size_t img_size = m_size.x * m_size.y * m_nrChannels;

    m_data = (unsigned char*)malloc(img_size);
    assert(m_data && "Image size must be positive");

    for (unsigned char* p = m_data; p < m_data + img_size; p += m_nrChannels) {
        p[0] = color[0], p[1] = color[1], p[2] = color[2], p[3] = color[3];
    }
}

void Image::clear()
{
    if (m_data) {
        stbi_image_free(m_data);
        m_data = 0;
    }
    m_size = glm::vec2(0);
    m_nrChannels = 0;
}

bool Image::isValid() const
{
    return m_data && m_size.x && m_size.y && m_nrChannels;
}

Image::Image(Image&& rhs)
{
    m_size = rhs.m_size;
    m_nrChannels = rhs.m_nrChannels;
    m_data = rhs.m_data;
    rhs.m_data = nullptr;
}