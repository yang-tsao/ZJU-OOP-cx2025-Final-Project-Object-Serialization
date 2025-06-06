#pragma once
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace binser
{
    // Serialize arithmetic types

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    void serialize(const T &obj, std::ostream &os)
    {
        os.write(reinterpret_cast<const char *>(&obj), sizeof(T));
    }

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    void deserialize(T &obj, std::istream &is)
    {
        is.read(reinterpret_cast<char *>(&obj), sizeof(T));
    }

    // Serialize std::string
    inline void serialize(const std::string &str, std::ostream &os)
    {
        size_t sz = str.size();
        serialize(sz, os);
        os.write(str.data(), sz);
    }

    inline void deserialize(std::string &str, std::istream &is)
    {
        size_t sz;
        deserialize(sz, is);
        str.resize(sz);
        is.read(&str[0], sz);
    }

    // Serialize std::pair

    template <typename T1, typename T2>
    void serialize(const std::pair<T1, T2> &p, std::ostream &os)
    {
        serialize(p.first, os);
        serialize(p.second, os);
    }

    template <typename T1, typename T2>
    void deserialize(std::pair<T1, T2> &p, std::istream &is)
    {
        deserialize(p.first, is);
        deserialize(p.second, is);
    }

    // Generic serialize for containers with size() and iterable (vector, list, set, etc.)
    template <std::ranges::sized_range Container>
    void serialize(const Container &c, std::ostream &os)
    {
        serialize(c.size(), os);
        for (const auto &e : c)
            serialize(e, os);
    }

    template <typename T>
    void deserialize(std::vector<T> &v, std::istream &is)
    {
        size_t sz;
        deserialize(sz, is);
        v.resize(sz);
        for (auto &e : v)
            deserialize(e, is);
    }

    template <typename Container>
        requires requires(Container container) {
            container.clear();                                                                                              // Must have clear()
            container.insert(std::declval<typename Container::iterator>(), std::declval<typename Container::value_type>()); // Must have insert(iterator, value)
            container.end();                                                                                                // Must have end()
        }
    void deserialize(Container &c, std::istream &is)
    {
        using T = typename Container::value_type;
        size_t sz;
        deserialize(sz, is);
        c.clear();
        for (size_t i = 0; i < sz; ++i)
        {
            T e;
            deserialize(e, is);
            c.insert(c.end(), e);
        }
    }

    template <typename K, typename V>
    void deserialize(std::map<K, V> &m, std::istream &is)
    {
        size_t sz;
        deserialize(sz, is);
        m.clear();
        for (size_t i = 0; i < sz; ++i)
        {
            std::pair<K, V> p;
            deserialize(p, is);
            m.insert(p);
        }
    }

    // Smart pointer (unique_ptr)
    template <typename T>
    void serialize(const std::unique_ptr<T> &ptr, std::ostream &os)
    {
        bool has = (bool)ptr;
        serialize(has, os);
        if (has)
            serialize(*ptr, os);
    }

    template <typename T>
    void deserialize(std::unique_ptr<T> &ptr, std::istream &is)
    {
        bool has;
        deserialize(has, is);
        if (has)
        {
            ptr = std::make_unique<T>();
            deserialize(*ptr, is);
        }
        else
            ptr.reset();
    }

    inline void serialize_r(std::ostream &os)
    {
        return;
    }
    inline void deserialize_r(std::istream &is)
    {
        return;
    }

    // User-defined type macro
    // Recursive case: process the first argument, then recurse
    template <typename First, typename... Rest>
    void serialize_r(std::ostream &os, First &&first, Rest &&...rest)
    {
        serialize(first, os);
        serialize_r(os, std::forward<Rest>(rest)...);
    }
    template <typename First, typename... Rest>
    void deserialize_r(std::istream &is, First &&first, Rest &&...rest)
    {
        deserialize(first, is);
        deserialize_r(is, std::forward<Rest>(rest)...);
    }

#define BINSER_SERIALIZABLE(...)                                                     \
    void serialize(std::ostream &os) const { binser::serialize_r(os, __VA_ARGS__); } \
    void deserialize(std::istream &is) { binser::deserialize_r(is, __VA_ARGS__); }

    template <typename T>
        requires requires(const T &obj, std::ostream &os) { obj.serialize(os); }
    void serialize(const T &obj, std::ostream &os)
    {
        obj.serialize(os);
    }

    template <typename T>
        requires requires(T &obj, std::istream &is) { obj.deserialize(is); }
    void deserialize(T &obj, std::istream &is)
    {
        obj.deserialize(is);
    }

    // File helpers
    template <typename T>
    void serialize(const T &obj, const std::string &filename)
    {
        std::ofstream ofs(filename, std::ios::binary);
        serialize(obj, ofs);
    }

    template <typename T>
    void deserialize(T &obj, const std::string &filename)
    {
        std::ifstream ifs(filename, std::ios::binary);
        deserialize(obj, ifs);
    }

} // namespace binser
