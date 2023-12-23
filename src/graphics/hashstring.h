#ifndef HASHEDNAME_H
#define HASHEDNAME_H

#include "common.h"

#include <string>
#include <unordered_map>

struct HashString {
    constexpr HashString(const char* name)
        : m_hash(constexpr_hash(name))
    {
        //        auto it = s_strings.find(m_hash);
        //        if (it == s_strings.end())
        //            s_strings.insert({ m_hash, name });
    }
    constexpr HashString(const HashString& other) = default;
    constexpr bool operator==(const HashString& rhs) const { return m_hash == rhs.m_hash; }
    // const std::string& getString() const { return s_strings[m_hash]; }

    const std::size_t m_hash;
    // static std::unordered_map<size_t, std::string> s_strings;
};
// inline std::unordered_map<size_t, std::string> HashString::s_strings;

namespace std {
template <>
struct hash<HashString> {
    std::size_t operator()(const HashString& hashedString) const { return hashedString.m_hash; }
};
}

#endif // HASHEDNAME_H
