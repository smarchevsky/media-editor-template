#ifndef IMAGE_H
#define IMAGE_H
#include "helper_general.h"
#include <filesystem>
#include <glm/vec2.hpp>

class Image : NoCopy<Image> {
public:
    Image();
    Image(const std::filesystem::path& path) { load(path); }
    Image(Image&& rhs);
    ~Image();

    void load(const std::filesystem::path& path);
    bool isValid() const;

private:
    glm::ivec2 m_size = glm::ivec2(0);
    int m_nrChannels {};
    unsigned char* m_data {};

    friend class GLTexture;
};

#endif // IMAGE_H
