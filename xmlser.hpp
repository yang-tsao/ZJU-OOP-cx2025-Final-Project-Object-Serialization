#pragma once
#include "base64.hpp"
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tinyxml2.h>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace xmlser
{
    // Removed 'using namespace tinyxml2;' to avoid namespace pollution

    // Forward declarations
    template <typename T>
    void serialize_xml(const T &obj, const std::string &name, const std::string &filename);
    template <typename T>
    void deserialize_xml(T &obj, const std::string &name, const std::string &filename);

    // --- Generic type trait for template specializations ---
    template <typename, template <typename...> class>
    constexpr bool is_specialization_of_v = false;
    template <typename... Args, template <typename...> class Template>
    constexpr bool is_specialization_of_v<Template<Args...>, Template> = true;

    // --- Helpers for type traits ---
    template <typename T>
    constexpr bool is_string_v = std::is_same_v<std::decay_t<T>, std::string>;

    template <typename T>
    constexpr bool is_pair_v = is_specialization_of_v<T, std::pair>;

    template <typename T>
    concept Pair = is_pair_v<T>;

    template <typename T>
    constexpr bool is_vector_v = is_specialization_of_v<T, std::vector>;
    template <typename T>
    constexpr bool is_list_v = is_specialization_of_v<T, std::list>;
    template <typename T>
    constexpr bool is_set_v = is_specialization_of_v<T, std::set>;
    template <typename T>
    constexpr bool is_map_v = is_specialization_of_v<T, std::map>;

    template <typename T>
    constexpr bool is_deque_v = is_specialization_of_v<T, std::deque>;
    template <typename T>
    constexpr bool is_unordered_set_v = is_specialization_of_v<T, std::unordered_set>;

    template <typename T>
    constexpr bool is_container_v = is_vector_v<T> || is_list_v<T> || is_set_v<T> || is_deque_v<T> || is_unordered_set_v<T>;

    template <typename T>
    concept Container = is_container_v<T>;

    template <typename T>
    constexpr bool is_natively_supported_v = std::is_trivially_copyable_v<T> || is_string_v<T> || is_pair_v<T> || is_container_v<T> || is_map_v<T>;

    // --- Serialization for arithmetic types ---
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T>>
    serialize_xml(const T &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        if constexpr (std::is_arithmetic_v<T>) {
            elem->SetAttribute("val", obj);
        } else {
            // base64 encode
            std::string b64;
            base64::base64_encode(b64, reinterpret_cast<const uint8_t *>(&obj), sizeof(T));
            elem->SetText(b64.c_str());
            elem->SetAttribute("encoding", "base64");
            elem->SetAttribute("size", static_cast<size_t>(sizeof(T)));
        }
    }
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T>>
    deserialize_xml(T &obj, const tinyxml2::XMLElement *elem)
    {
        if constexpr (std::is_arithmetic_v<T>) {
            elem->QueryAttribute("val", &obj);
        } else {
            // base64 decode
            const char *txt = elem->GetText();
            if (!txt) throw std::runtime_error("Missing base64 text for trivially copyable type");
            std::string b64 = txt;
            std::string decoded;
            base64::base64_decode(decoded, b64);
            size_t sz = 0;
            elem->QueryUnsigned64Attribute("size", &sz);
            if (sz != sizeof(T) || decoded.size() != sizeof(T))
                throw std::runtime_error("Base64 decoded size mismatch for trivially copyable type");
            std::memcpy(&obj, decoded.data(), sizeof(T));
        }
    }

    // --- Serialization for std::string ---
    inline void serialize_xml(const std::string &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        elem->SetText(obj.c_str());
    }
    inline void deserialize_xml(std::string &obj, const tinyxml2::XMLElement *elem)
    {
        const char *txt = elem->GetText();
        obj = txt ? txt : "";
    }

    // --- Serialization for std::pair ---
    template <typename T1, typename T2>
    void serialize_xml(const std::pair<T1, T2> &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        tinyxml2::XMLElement *first = doc.NewElement("first");
        serialize_xml(obj.first, first, doc);
        elem->InsertEndChild(first);
        tinyxml2::XMLElement *second = doc.NewElement("second");
        serialize_xml(obj.second, second, doc);
        elem->InsertEndChild(second);
    }
    template <Pair T>
    void deserialize_xml(T &obj, const tinyxml2::XMLElement *elem)
    {
        const tinyxml2::XMLElement *first = elem->FirstChildElement("first");
        deserialize_xml(obj.first, first);
        const tinyxml2::XMLElement *second = elem->FirstChildElement("second");
        deserialize_xml(obj.second, second);
    }

    // --- Serialization for standard containers (vector, list, set) using concepts ---
    template <Container T>
    void serialize_xml(const T &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        for (const auto &item : obj)
        {
            tinyxml2::XMLElement *child = doc.NewElement("item");
            serialize_xml(item, child, doc);
            elem->InsertEndChild(child);
        }
    }

    template <Container T>
    void deserialize_xml(T &obj, const tinyxml2::XMLElement *elem)
    {
        obj.clear();
        for (const tinyxml2::XMLElement *child = elem->FirstChildElement("item"); child; child = child->NextSiblingElement("item"))
        {
            typename T::value_type val;
            deserialize_xml(val, child);
            obj.insert(obj.end(), val);
        }
    }

    // --- Serialization for std::map ---
    template <typename K, typename V>
    void serialize_xml(const std::map<K, V> &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        for (const auto &p : obj)
        {
            tinyxml2::XMLElement *child = doc.NewElement("item");
            serialize_xml(p, child, doc);
            elem->InsertEndChild(child);
        }
    }
    template <typename K, typename V>
    void deserialize_xml(std::map<K, V> &obj, const tinyxml2::XMLElement *elem)
    {
        obj.clear();
        for (const tinyxml2::XMLElement *child = elem->FirstChildElement("item"); child; child = child->NextSiblingElement("item"))
        {
            std::pair<K, V> p;
            deserialize_xml(p, child);
            obj.insert(p);
        }
    }

    // --- Serialization for std::unique_ptr ---
    template <typename T>
    void serialize_xml(const std::unique_ptr<T> &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        // Serialize presence as attribute
        elem->SetAttribute("has_value", static_cast<bool>(obj));
        if (obj)
        {
            tinyxml2::XMLElement *child = doc.NewElement("value");
            serialize_xml(*obj, child, doc);
            elem->InsertEndChild(child);
        }
    }
    template <typename T>
    void deserialize_xml(std::unique_ptr<T> &obj, const tinyxml2::XMLElement *elem)
    {
        bool has_value = false;
        elem->QueryBoolAttribute("has_value", &has_value);
        if (has_value)
        {
            const tinyxml2::XMLElement *child = elem->FirstChildElement("value");
            if (!child)
                throw std::runtime_error("Missing <value> for unique_ptr in element '" + std::string(elem->Name()) + "' while processing file.");
            obj = std::make_unique<T>();
            deserialize_xml(*obj, child);
        }
        else
            obj.reset();
    }

    // --- User-defined type support macro ---
    // This macro simplifies the implementation of serialization and deserialization
    // for user-defined types. It generates `serialize_xml` and `deserialize_xml` methods
    // that recursively process the provided member variables.
    // Usage:
    // struct MyStruct {
    //     int a;
    //     std::string b;
    //     XMLSERIALIZABLE(a, b)
    // };
    // This will allow `MyStruct` to be serialized and deserialized using the xmlser library.
#define XMLSERIALIZABLE(...)                                                                                                              \
    void serialize_xml(tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) const { xmlser::serialize_xml_r(elem, doc, __VA_ARGS__); } \
    void deserialize_xml(const tinyxml2::XMLElement *elem) { xmlser::deserialize_xml_r(elem->FirstChildElement(), __VA_ARGS__); }

    // Helper for user-defined types: recursive serialization
    inline void serialize_xml_r(tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) {}
    inline void deserialize_xml_r(const tinyxml2::XMLElement * /*elem*/) {}
    template <typename First, typename... Rest>
    void serialize_xml_r(tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc, const First &first, const Rest &...rest)
    {
        tinyxml2::XMLElement *child = doc.NewElement(typeid(First).name());
        serialize_xml(first, child, doc);
        elem->InsertEndChild(child);
        serialize_xml_r(elem, doc, rest...);
    }
    template <typename First, typename... Rest>
    void deserialize_xml_r(const tinyxml2::XMLElement *child, First &first, Rest &...rest)
    {
        if (!child)
            return;
        deserialize_xml(first, child);
        deserialize_xml_r(child->NextSiblingElement(), rest...);
    }
    // --- User-defined type fallback ---
    // Assumes that the type implements `serialize_xml` and `deserialize_xml`.
    // Static assertions enforce this requirement.
    template <typename T>
        requires(!is_natively_supported_v<T> && requires(const T &t, tinyxml2::XMLElement *e, tinyxml2::XMLDocument &d) { t.serialize_xml(e, d); })
    void serialize_xml(const T &obj, tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc)
    {
        obj.serialize_xml(elem, doc);
    }
    template <typename T>
        requires(!is_natively_supported_v<T> && requires(T &t, const tinyxml2::XMLElement *e) { t.deserialize_xml(e); })
    void deserialize_xml(T &obj, const tinyxml2::XMLElement *elem)
    {
        obj.deserialize_xml(elem);
    }

    // --- File entry points ---
    template <typename T>
    void serialize_xml(const T &obj, const std::string &name, const std::string &filename)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement *root = doc.NewElement("serialization");
        doc.InsertFirstChild(root);
        tinyxml2::XMLElement *elem = doc.NewElement(name.c_str());
        serialize_xml(obj, elem, doc);
        root->InsertEndChild(elem);
        doc.SaveFile(filename.c_str());
    }
    template <typename T>
    void deserialize_xml(T &obj, const std::string &name, const std::string &filename)
    {
        tinyxml2::XMLDocument doc;
        doc.LoadFile(filename.c_str());
        const tinyxml2::XMLElement *root = doc.FirstChildElement("serialization");
        if (!root)
            throw std::runtime_error("No <serialization> root in file: " + filename);
        const tinyxml2::XMLElement *elem = root->FirstChildElement(name.c_str());
        if (!elem)
            throw std::runtime_error("No element with name '" + name + "' in file '" + filename + "'");
        deserialize_xml(obj, elem);
    }

} // namespace xmlser
