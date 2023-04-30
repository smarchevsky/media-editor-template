#ifndef IMAGE_H
#define IMAGE_H
#include "helper_general.h"
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Image : NoCopy<Image> {
public:
    Image();
    Image(const std::filesystem::path& path) { load(path); }
    Image(glm::ivec2 size, glm::ivec4 color) { fill(size, color); }
    Image(Image&& rhs);
    ~Image();

    void load(const std::filesystem::path& path);
    void fill(glm::ivec2 size, glm::ivec4 color);
    void clear();
    bool isValid() const;

private:
    std::string m_name;
    glm::ivec2 m_size = glm::ivec2(0);
    int m_nrChannels {};
    unsigned char* m_data {};

    friend class GLTexture2D;
};

#endif // IMAGE_H
