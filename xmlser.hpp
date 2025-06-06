#pragma once
#include <tinyxml2.h>
#include <type_traits>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <utility>
#include <memory>
#include <sstream>

namespace xmlser {
using namespace tinyxml2;

// Serialize arithmetic types

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
serialize_xml(const T& obj, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    elem->SetAttribute("val", obj);
    parent->InsertEndChild(elem);
}

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type
deserialize_xml(T& obj, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    elem->QueryAttribute("val", &obj);
}

// Serialize std::string
inline void serialize_xml(const std::string& str, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    elem->SetText(str.c_str());
    parent->InsertEndChild(elem);
}

inline void deserialize_xml(std::string& str, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    str = elem->GetText() ? elem->GetText() : "";
}

// Serialize std::pair

template<typename T1, typename T2>
void serialize_xml(const std::pair<T1, T2>& p, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    serialize_xml(p.first, "first", doc, elem);
    serialize_xml(p.second, "second", doc, elem);
    parent->InsertEndChild(elem);
}

template<typename T1, typename T2>
void deserialize_xml(std::pair<T1, T2>& p, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    deserialize_xml(p.first, "first", elem);
    deserialize_xml(p.second, "second", elem);
}

// Serialize std::vector

template<typename T>
void serialize_xml(const std::vector<T>& v, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    for (const auto& e : v) serialize_xml(e, "item", doc, elem);
    parent->InsertEndChild(elem);
}

template<typename T>
void deserialize_xml(std::vector<T>& v, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    v.clear();
    for (XMLElement* it = elem->FirstChildElement("item"); it; it = it->NextSiblingElement("item")) {
        T e;
        deserialize_xml(e, "item", elem);
        v.push_back(e);
    }
}

// Serialize std::list

template<typename T>
void serialize_xml(const std::list<T>& l, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    for (const auto& e : l) serialize_xml(e, "item", doc, elem);
    parent->InsertEndChild(elem);
}

template<typename T>
void deserialize_xml(std::list<T>& l, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    l.clear();
    for (XMLElement* it = elem->FirstChildElement("item"); it; it = it->NextSiblingElement("item")) {
        T e;
        deserialize_xml(e, "item", elem);
        l.push_back(e);
    }
}

// Serialize std::set

template<typename T>
void serialize_xml(const std::set<T>& s, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    for (const auto& e : s) serialize_xml(e, "item", doc, elem);
    parent->InsertEndChild(elem);
}

template<typename T>
void deserialize_xml(std::set<T>& s, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    s.clear();
    for (XMLElement* it = elem->FirstChildElement("item"); it; it = it->NextSiblingElement("item")) {
        T e;
        deserialize_xml(e, "item", elem);
        s.insert(e);
    }
}

// Serialize std::map

template<typename K, typename V>
void serialize_xml(const std::map<K, V>& m, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    for (const auto& p : m) serialize_xml(p, "pair", doc, elem);
    parent->InsertEndChild(elem);
}

template<typename K, typename V>
void deserialize_xml(std::map<K, V>& m, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    m.clear();
    for (XMLElement* it = elem->FirstChildElement("pair"); it; it = it->NextSiblingElement("pair")) {
        std::pair<K, V> p;
        deserialize_xml(p, "pair", elem);
        m.insert(p);
    }
}

// Smart pointer (unique_ptr)
template<typename T>
void serialize_xml(const std::unique_ptr<T>& ptr, const char* name, XMLDocument& doc, XMLElement* parent) {
    XMLElement* elem = doc.NewElement(name);
    elem->SetAttribute("has", (bool)ptr);
    if (ptr) serialize_xml(*ptr, "value", doc, elem);
    parent->InsertEndChild(elem);
}

template<typename T>
void deserialize_xml(std::unique_ptr<T>& ptr, const char* name, XMLElement* parent) {
    XMLElement* elem = parent->FirstChildElement(name);
    bool has = false;
    elem->QueryBoolAttribute("has", &has);
    if (has) {
        ptr = std::make_unique<T>();
        deserialize_xml(*ptr, "value", elem);
    } else {
        ptr.reset();
    }
}

// User-defined type macro
#define XMLSER_SERIALIZABLE(...) \
    template<typename Archive> void serialize_xml(Archive& ar, const char* name, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent) const { \
        XMLElement* elem = doc.NewElement(name); \
        int _[] = { (ar(*this, #__VA_ARGS__, doc, elem), 0)... }; (void)_; \
        parent->InsertEndChild(elem); \
    } \
    template<typename Archive> void deserialize_xml(Archive& ar, const char* name, tinyxml2::XMLElement* parent) { \
        XMLElement* elem = parent->FirstChildElement(name); \
        int _[] = { (ar(*this, #__VA_ARGS__, elem), 0)... }; (void)_; \
    }

// File helpers

template<typename T>
void serialize_xml(const T& obj, const char* name, const std::string& filename) {
    XMLDocument doc;
    XMLElement* root = doc.NewElement("serialization");
    doc.InsertFirstChild(root);
    serialize_xml(obj, name, doc, root);
    doc.SaveFile(filename.c_str());
}

template<typename T>
void deserialize_xml(T& obj, const char* name, const std::string& filename) {
    XMLDocument doc;
    doc.LoadFile(filename.c_str());
    XMLElement* root = doc.FirstChildElement("serialization");
    deserialize_xml(obj, name, root);
}

} // namespace xmlser
