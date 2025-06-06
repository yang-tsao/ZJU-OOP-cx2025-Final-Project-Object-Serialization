#pragma once
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <tinyxml2.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace xmlser
{
    using namespace tinyxml2;

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
    constexpr bool is_vector_v = is_specialization_of_v<T, std::vector>;
    template <typename T>
    constexpr bool is_list_v = is_specialization_of_v<T, std::list>;
    template <typename T>
    constexpr bool is_set_v = is_specialization_of_v<T, std::set>;
    template <typename T>
    constexpr bool is_map_v = is_specialization_of_v<T, std::map>;

    template <typename T>
    constexpr bool is_container_v = is_vector_v<T> || is_list_v<T> || is_set_v<T>;

    template <typename T>
    constexpr bool is_natively_supported_v = std::is_arithmetic_v<T> || is_string_v<T> || is_pair_v<T> || is_container_v<T> || is_map_v<T>;

    // --- Serialization for arithmetic types ---
    template <typename T>
    std::enable_if_t<std::is_arithmetic_v<T>>
    serialize_xml(const T &obj, XMLElement *elem, XMLDocument &doc)
    {
        elem->SetAttribute("val", obj);
    }
    template <typename T>
    std::enable_if_t<std::is_arithmetic_v<T>>
    deserialize_xml(T &obj, const XMLElement *elem)
    {
        elem->QueryAttribute("val", &obj);
    }

    // --- Serialization for std::string ---
    inline void serialize_xml(const std::string &obj, XMLElement *elem, XMLDocument &doc)
    {
        elem->SetText(obj.c_str());
    }
    inline void deserialize_xml(std::string &obj, const XMLElement *elem)
    {
        const char *txt = elem->GetText();
        obj = txt ? txt : "";
    }

    // --- Serialization for std::pair ---
    template <typename T1, typename T2>
    void serialize_xml(const std::pair<T1, T2> &obj, XMLElement *elem, XMLDocument &doc)
    {
        XMLElement *first = doc.NewElement("first");
        serialize_xml(obj.first, first, doc);
        elem->InsertEndChild(first);
        XMLElement *second = doc.NewElement("second");
        serialize_xml(obj.second, second, doc);
        elem->InsertEndChild(second);
    }
    template <typename T1, typename T2>
    void deserialize_xml(std::pair<T1, T2> &obj, const XMLElement *elem)
    {
        const XMLElement *first = elem->FirstChildElement("first");
        deserialize_xml(obj.first, first);
        const XMLElement *second = elem->FirstChildElement("second");
        deserialize_xml(obj.second, second);
    }

    // --- Serialization for standard containers (vector, list, set) ---
    template <typename Container>
    std::enable_if_t<is_container_v<Container>>
    serialize_xml(const Container &obj, XMLElement *elem, XMLDocument &doc)
    {
        for (const auto &item : obj)
        {
            XMLElement *child = doc.NewElement("item");
            serialize_xml(item, child, doc);
            elem->InsertEndChild(child);
        }
    }
    template <typename Container>
    std::enable_if_t<is_container_v<Container>>
    deserialize_xml(Container &obj, const XMLElement *elem)
    {
        obj.clear();
        for (const XMLElement *child = elem->FirstChildElement("item"); child; child = child->NextSiblingElement("item"))
        {
            typename Container::value_type val;
            deserialize_xml(val, child);
            obj.insert(obj.end(), val);
        }
    }

    // --- Serialization for std::map ---
    template <typename K, typename V>
    void serialize_xml(const std::map<K, V> &obj, XMLElement *elem, XMLDocument &doc)
    {
        for (const auto &p : obj)
        {
            XMLElement *child = doc.NewElement("item");
            serialize_xml(p, child, doc);
            elem->InsertEndChild(child);
        }
    }
    template <typename K, typename V>
    void deserialize_xml(std::map<K, V> &obj, const XMLElement *elem)
    {
        obj.clear();
        for (const XMLElement *child = elem->FirstChildElement("item"); child; child = child->NextSiblingElement("item"))
        {
            std::pair<K, V> p;
            deserialize_xml(p, child);
            obj.insert(p);
        }
    }

    // --- Serialization for std::unique_ptr ---
    template <typename T>
    void serialize_xml(const std::unique_ptr<T> &obj, XMLElement *elem, XMLDocument &doc)
    {
        // Serialize presence as attribute
        elem->SetAttribute("has_value", static_cast<bool>(obj));
        if (obj)
        {
            XMLElement *child = doc.NewElement("value");
            serialize_xml(*obj, child, doc);
            elem->InsertEndChild(child);
        }
    }
    template <typename T>
    void deserialize_xml(std::unique_ptr<T> &obj, const XMLElement *elem)
    {
        bool has_value = false;
        elem->QueryBoolAttribute("has_value", &has_value);
        if (has_value)
        {
            const XMLElement *child = elem->FirstChildElement("value");
            if (!child)
                throw std::runtime_error("Missing <value> for unique_ptr");
            obj = std::make_unique<T>();
            deserialize_xml(*obj, child);
        }
        else
            obj.reset();
    }

    // --- User-defined type support macro ---
#define XMLSERIALIZABLE(...)                                                                                                              \
    void serialize_xml(tinyxml2::XMLElement *elem, tinyxml2::XMLDocument &doc) const { xmlser::serialize_xml_r(elem, doc, __VA_ARGS__); } \
    void deserialize_xml(const tinyxml2::XMLElement *elem) { xmlser::deserialize_xml_r(elem->FirstChildElement(), __VA_ARGS__); }

    // Helper for user-defined types: recursive serialization
    inline void serialize_xml_r(XMLElement *elem, XMLDocument &doc) {}
    inline void deserialize_xml_r(const XMLElement * /*elem*/) {}
    template <typename First, typename... Rest>
    void serialize_xml_r(XMLElement *elem, XMLDocument &doc, const First &first, const Rest &...rest)
    {
        XMLElement *child = doc.NewElement(typeid(First).name());
        serialize_xml(first, child, doc);
        elem->InsertEndChild(child);
        serialize_xml_r(elem, doc, rest...);
    }
    template <typename First, typename... Rest>
    void deserialize_xml_r(const XMLElement *child, First &first, Rest &...rest)
    {
        if (!child)
            return;
        deserialize_xml(first, child);
        deserialize_xml_r(child->NextSiblingElement(), rest...);
    }

    // --- User-defined type fallback ---
    template <typename T>
    std::enable_if_t<!is_natively_supported_v<T>>
    serialize_xml(const T &obj, XMLElement *elem, XMLDocument &doc)
    {
        obj.serialize_xml(elem, doc);
    }
    template <typename T>
    std::enable_if_t<!is_natively_supported_v<T>>
    deserialize_xml(T &obj, const XMLElement *elem)
    {
        obj.deserialize_xml(elem);
    }

    // --- File entry points ---
    template <typename T>
    void serialize_xml(const T &obj, const std::string &name, const std::string &filename)
    {
        XMLDocument doc;
        XMLElement *root = doc.NewElement("serialization");
        doc.InsertFirstChild(root);
        XMLElement *elem = doc.NewElement(name.c_str());
        serialize_xml(obj, elem, doc);
        root->InsertEndChild(elem);
        doc.SaveFile(filename.c_str());
    }
    template <typename T>
    void deserialize_xml(T &obj, const std::string &name, const std::string &filename)
    {
        XMLDocument doc;
        doc.LoadFile(filename.c_str());
        const XMLElement *root = doc.FirstChildElement("serialization");
        if (!root)
            throw std::runtime_error("No <serialization> root");
        const XMLElement *elem = root->FirstChildElement(name.c_str());
        if (!elem)
            throw std::runtime_error("No element with given name");
        deserialize_xml(obj, elem);
    }

} // namespace xmlser
