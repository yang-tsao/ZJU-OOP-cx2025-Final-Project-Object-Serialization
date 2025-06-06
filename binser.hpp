#pragma once
#include <fstream>
#include <type_traits>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <utility>
#include <memory>

namespace binser {
// Serialize arithmetic types

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
serialize(const T& obj, std::ostream& os) {
    os.write(reinterpret_cast<const char*>(&obj), sizeof(T));
}

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
deserialize(T& obj, std::istream& is) {
    is.read(reinterpret_cast<char*>(&obj), sizeof(T));
}

// Serialize std::string
inline void serialize(const std::string& str, std::ostream& os) {
    size_t sz = str.size();
    serialize(sz, os);
    os.write(str.data(), sz);
}

inline void deserialize(std::string& str, std::istream& is) {
    size_t sz;
    deserialize(sz, is);
    str.resize(sz);
    is.read(&str[0], sz);
}

// Serialize std::pair

template<typename T1, typename T2>
void serialize(const std::pair<T1, T2>& p, std::ostream& os) {
    serialize(p.first, os);
    serialize(p.second, os);
}

template<typename T1, typename T2>
void deserialize(std::pair<T1, T2>& p, std::istream& is) {
    deserialize(p.first, is);
    deserialize(p.second, is);
}

// Serialize std::vector

template<typename T>
void serialize(const std::vector<T>& v, std::ostream& os) {
    size_t sz = v.size();
    serialize(sz, os);
    for (const auto& e : v) serialize(e, os);
}

template<typename T>
void deserialize(std::vector<T>& v, std::istream& is) {
    size_t sz;
    deserialize(sz, is);
    v.resize(sz);
    for (auto& e : v) deserialize(e, is);
}

// Serialize std::list

template<typename T>
void serialize(const std::list<T>& l, std::ostream& os) {
    size_t sz = l.size();
    serialize(sz, os);
    for (const auto& e : l) serialize(e, os);
}

template<typename T>
void deserialize(std::list<T>& l, std::istream& is) {
    size_t sz;
    deserialize(sz, is);
    l.clear();
    for (size_t i = 0; i < sz; ++i) {
        T e;
        deserialize(e, is);
        l.push_back(e);
    }
}

// Serialize std::set

template<typename T>
void serialize(const std::set<T>& s, std::ostream& os) {
    size_t sz = s.size();
    serialize(sz, os);
    for (const auto& e : s) serialize(e, os);
}

template<typename T>
void deserialize(std::set<T>& s, std::istream& is) {
    size_t sz;
    deserialize(sz, is);
    s.clear();
    for (size_t i = 0; i < sz; ++i) {
        T e;
        deserialize(e, is);
        s.insert(e);
    }
}

// Serialize std::map

template<typename K, typename V>
void serialize(const std::map<K, V>& m, std::ostream& os) {
    size_t sz = m.size();
    serialize(sz, os);
    for (const auto& p : m) serialize(p, os);
}

template<typename K, typename V>
void deserialize(std::map<K, V>& m, std::istream& is) {
    size_t sz;
    deserialize(sz, is);
    m.clear();
    for (size_t i = 0; i < sz; ++i) {
        std::pair<K, V> p;
        deserialize(p, is);
        m.insert(p);
    }
}

// Smart pointer (unique_ptr)
template<typename T>
void serialize(const std::unique_ptr<T>& ptr, std::ostream& os) {
    bool has = (bool)ptr;
    serialize(has, os);
    if (has) serialize(*ptr, os);
}

template<typename T>
void deserialize(std::unique_ptr<T>& ptr, std::istream& is) {
    bool has;
    deserialize(has, is);
    if (has) {
        ptr = std::make_unique<T>();
        deserialize(*ptr, is);
    } else {
        ptr.reset();
    }
}

// User-defined type macro
#define BINSER_SERIALIZABLE(...) \
    template<typename Archive> void serialize(Archive& ar) const { \
        int _[] = { (ar & __VA_ARGS__, 0)... }; (void)_; \
    } \
    template<typename Archive> void deserialize(Archive& ar) { \
        int _[] = { (ar & __VA_ARGS__, 0)... }; (void)_; \
    }

// File helpers
template<typename T>
void serialize(const T& obj, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    serialize(obj, ofs);
}

template<typename T>
void deserialize(T& obj, const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    deserialize(obj, ifs);
}

} // namespace binser
