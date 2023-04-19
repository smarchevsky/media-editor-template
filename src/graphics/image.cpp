#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

Image::Image()
{
}

Image::~Image()
{
    if (m_data)
        stbi_image_free(m_data);
}

void Image::load(const std::filesystem::path& path)
{
    // stbi_set_flip_vertically_on_load(true);
    m_data = stbi_load(path.c_str(), &m_size.x, &m_size.y, &m_nrChannels, 0);
    if (!m_data) {
        LOGE("Failed to load texture from path: " << path);
    }
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
