#include "binser.hpp"
#include "xmlser.hpp"
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct UserDefinedType
{
    int idx;
    int idy;
    std::string name;
    std::vector<double> data;
    // For binser
    BINSER_SERIALIZABLE(idx, idy, name, data)
    XMLSERIALIZABLE(idx, idy, name, data)
};

void test_binser()
{

    using namespace binser;
    // Arithmetic
    int n0 = 256, n1 = 0;
    serialize(n0, "n.data");
    deserialize(n1, "n.data");
    assert(n0 == n1);
    std::cout << "Arithmetic assertion passed!\n";
    // String
    std::string s0 = "hello world", s1;
    serialize(s0, "s.data");
    deserialize(s1, "s.data");
    assert(s0 == s1);
    std::cout << "String assertion passed!\n";
    // Pair
    std::pair<int, double> p0 = {2, 3.1}, p1;
    serialize(p0, "p.data");
    deserialize(p1, "p.data");
    assert(p0 == p1);
    std::cout << "Pair assertion passed!\n";
    // Vector
    std::vector<int> v0 = {1, 2, 3, 4}, v1;
    serialize(v0, "v.data");
    deserialize(v1, "v.data");
    assert(v0 == v1);
    std::cout << "Vector assertion passed!\n";
    // List
    std::list<std::string> l0 = {"a", "b", "c"}, l1;
    serialize(l0, "l.data");
    deserialize(l1, "l.data");
    assert(l0 == l1);
    std::cout << "List assertion passed!\n";
    // Set
    std::set<int> set0 = {1, 2, 3}, set1;
    serialize(set0, "set.data");
    deserialize(set1, "set.data");
    assert(set0 == set1);
    std::cout << "Set assertion passed!\n";
    // Map
    std::map<std::string, int> m0 = {{"a", 1}, {"b", 2}}, m1;
    serialize(m0, "m.data");
    deserialize(m1, "m.data");
    assert(m0 == m1);
    std::cout << "Map assertion passed!\n";
    // Unique_ptr
    std::unique_ptr<int> up0 = std::make_unique<int>(42), up1;
    serialize(up0, "up.data");
    deserialize(up1, "up.data");
    assert(*up0 == *up1);
    std::cout << "Unique_ptr assertion passed!\n";
    // User defined
    UserDefinedType u0{1, 2, "test", {1.1, 2.2}}, u1;
    // u0.serialize(std::ofstream("u.data", std::ios::binary));
    serialize(u0, "u.data");
    deserialize(u1, "u.data");
    assert(u0.idx == u1.idx && u0.idy == u1.idy && u0.name == u1.name && u0.data == u1.data);
    std::cout << "User Defined Type assertion passed!\n";
}

void test_xmlser()
{
    using namespace xmlser;
    // Arithmetic
    int n0 = 256, n1 = 0;
    serialize_xml(n0, "number", "n.xml");
    deserialize_xml(n1, "number", "n.xml");
    assert(n0 == n1);
    std::cout << "Arithmetic assertion passed!\n";
    // String
    std::string s0 = "hello world", s1;
    serialize_xml(s0, "str", "s.xml");
    deserialize_xml(s1, "str", "s.xml");
    assert(s0 == s1);
    std::cout << "String assertion passed!\n";
    // Pair
    std::pair<int, double> p0 = {2, 3.1}, p1;
    serialize_xml(p0, "pair", "pair.xml");
    deserialize_xml(p1, "pair", "pair.xml");
    assert(p0 == p1);
    std::cout << "Pair assertion passed!\n";
    // Vector
    std::vector<int> v0 = {1, 2, 3, 4}, v1;
    serialize_xml(v0, "vec", "v.xml");
    deserialize_xml(v1, "vec", "v.xml");
    assert(v0 == v1);
    std::cout << "Vector assertion passed!\n";
    // List
    std::list<std::string> l0 = {"a", "b", "c"}, l1;
    serialize_xml(l0, "list", "l.xml");
    deserialize_xml(l1, "list", "l.xml");
    assert(l0 == l1);
    std::cout << "List assertion passed!\n";
    // Set
    std::set<int> set0 = {1, 2, 3}, set1;
    serialize_xml(set0, "set", "set.xml");
    deserialize_xml(set1, "set", "set.xml");
    assert(set0 == set1);
    std::cout << "Set assertion passed!\n";
    // Map
    std::map<std::string, int> m0 = {{"a", 1}, {"b", 2}}, m1;
    serialize_xml(m0, "map", "m.xml");
    deserialize_xml(m1, "map", "m.xml");
    assert(m0 == m1);
    std::cout << "Map assertion passed!\n";
    // Unique_ptr
    std::unique_ptr<int> up0 = std::make_unique<int>(42), up1;
    serialize_xml(up0, "u_ptr", "up.xml");
    deserialize_xml(up1, "u_ptr", "up.xml");
    assert(*up0 == *up1);
    std::cout << "Unique_ptr assertion passed!\n";
    // User defined
    UserDefinedType u0{1, 2, "test", {1.1, 2.2}}, u1;
    serialize_xml(u0, "ud", "u.xml");
    deserialize_xml(u1, "ud", "u.xml");
    assert(u0.idx == u1.idx && u0.idy == u1.idy && u0.name == u1.name && u0.data == u1.data);
    std::cout << "User Defined Type assertion passed!\n";
}

int main()
{

    test_binser();
    std::cout << "Binary serialization tests passed!\n";
    test_xmlser();
    std::cout << "XML serialization tests passed!\n";
    std::cout << "All tests passed!\n";
    return 0;
}
