#ifndef VEC2_H
#define VEC2_H
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <math.h>

typedef sf::Vector2f vec2;
typedef sf::Vector2u uvec2;
typedef sf::Vector2i ivec2;

inline vec2 toFloat(const ivec2& v) { return { (float)v.x, (float)v.y }; }
inline vec2 toFloat(const uvec2& v) { return { (float)v.x, (float)v.y }; }
inline ivec2 toInt(const vec2& v) { return { (int)v.x, (int)v.y }; }
inline ivec2 toInt(const uvec2& v) { return { (int)v.x, (int)v.y }; }
inline ivec2 toInt(const sf::Event::MouseMoveEvent& v) { return { v.x, v.y }; }
inline uvec2 toUInt(const vec2& v) { return { (unsigned)std::max(v.x, 0.f), (unsigned)std::max(v.y, 0.f) }; }
inline uvec2 toUInt(const sf::Event::SizeEvent& event) { return { event.width, event.height }; }
inline vec2 toFloat(const sf::Event::SizeEvent& event) { return { (float)event.width, (float)event.height }; }

inline vec2 operator*(const vec2& a, const vec2& b) { return { a.x * b.x, a.y * b.y }; };
inline vec2 operator*(const vec2& a, const uvec2& b) { return { a.x * b.x, a.y * b.y }; };
inline vec2 operator*(const uvec2& a, const vec2& b) { return { a.x * b.x, a.y * b.y }; };
inline vec2 operator*(const vec2& a, const ivec2& b) { return { a.x * b.x, a.y * b.y }; };
inline vec2 operator*(const ivec2& a, const vec2& b) { return { a.x * b.x, a.y * b.y }; };

inline vec2 operator/(const vec2& a, const vec2& b) { return { a.x / b.x, a.y / b.y }; };
inline vec2 operator/(const vec2& a, const uvec2& b) { return { a.x / b.x, a.y / b.y }; };
inline vec2 operator/(const uvec2& a, const vec2& b) { return { a.x / b.x, a.y / b.y }; };
inline vec2 operator/(const vec2& a, const ivec2& b) { return { a.x / b.x, a.y / b.y }; };
inline vec2 operator/(const ivec2& a, const vec2& b) { return { a.x / b.x, a.y / b.y }; };
inline vec2 operator/(const vec2& a, float b) { return { a.x / b, a.y / b }; };

inline float dot(const vec2& v0, const vec2& v1) { return v0.x * v1.x + v0.y * v1.y; }
inline float operator|(const vec2& v0, const vec2& v1) { return dot(v0, v1); }
inline vec2 perp(const vec2& v0) { return { -v0.y, v0.x }; }
inline float length(const vec2& v0) { return sqrtf(dot(v0, v0)); }
inline float distance(const vec2& v0, const vec2& v1) { return length(v1 - v0); }
inline vec2 normalized(const vec2& v0)
{
    float len = length(v0);
    if (len != 0.f)
        return v0 / len;
    return v0;
}

inline void printVec(const vec2& v0)
{
    std::printf("Vec2: %.3f, %.3f", v0.x, v0.y);
}

#endif // VEC2_H
