#ifndef HASHEDNAME_H
#define HASHEDNAME_H

#include <string>
#include <unordered_map>

static std::size_t constexpr const_hash(char const* input)
{
    return *input ? static_cast<std::size_t>(*input) + 33 * const_hash(input + 1) : 5381;
}

struct HashString {
    HashString(const char* name)
        : m_hash(const_hash(name))
    {
        auto it = s_strings.find(m_hash);
        if (it == s_strings.end())
            s_strings.insert({ m_hash, name });
    }
    constexpr HashString(const HashString& other) = default;
    constexpr bool operator==(const HashString& rhs) const { return m_hash == rhs.m_hash; }
    const std::string& getString() const { return s_strings[m_hash]; }

    const std::size_t m_hash;
    static std::unordered_map<size_t, std::string> s_strings;
};
inline std::unordered_map<size_t, std::string> HashString::s_strings;

namespace std {
template <>
struct hash<HashString> {
    std::size_t operator()(const HashString& hashedString) const { return hashedString.m_hash; }
};
}

#endif // HASHEDNAME_H
