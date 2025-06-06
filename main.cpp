#include "binser.hpp"
#include "xmlser.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <memory>
#include <string>

struct UserDefinedType {
    int idx;
    std::string name;
    std::vector<double> data;
    // For binser
    template<typename Archive> void serialize(Archive& ar) const { ar & idx & name & data; }
    template<typename Archive> void deserialize(Archive& ar) { ar & idx & name & data; }
};

int main() {
    using namespace binser;
    using namespace xmlser;
    // Arithmetic
    int n0 = 256, n1 = 0;
    serialize(n0, "n.data");
    deserialize(n1, "n.data");
    assert(n0 == n1);
    // String
    std::string s0 = "hello world", s1;
    serialize(s0, "s.data");
    deserialize(s1, "s.data");
    assert(s0 == s1);
    // Pair
    std::pair<int, double> p0 = {2, 3.1}, p1;
    serialize(p0, "p.data");
    deserialize(p1, "p.data");
    assert(p0 == p1);
    // Vector
    std::vector<int> v0 = {1,2,3,4}, v1;
    serialize(v0, "v.data");
    deserialize(v1, "v.data");
    assert(v0 == v1);
    // List
    std::list<std::string> l0 = {"a","b","c"}, l1;
    serialize(l0, "l.data");
    deserialize(l1, "l.data");
    assert(l0 == l1);
    // Set
    std::set<int> set0 = {1,2,3}, set1;
    serialize(set0, "set.data");
    deserialize(set1, "set.data");
    assert(set0 == set1);
    // Map
    std::map<std::string, int> m0 = {{"a",1},{"b",2}}, m1;
    serialize(m0, "m.data");
    deserialize(m1, "m.data");
    assert(m0 == m1);
    // Unique_ptr
    std::unique_ptr<int> up0 = std::make_unique<int>(42), up1;
    serialize(up0, "up.data");
    deserialize(up1, "up.data");
    assert(*up0 == *up1);
    // User defined
    UserDefinedType u0{1, "test", {1.1, 2.2}}, u1;
    serialize(u0, "u.data");
    deserialize(u1, "u.data");
    assert(u0.idx == u1.idx && u0.name == u1.name && u0.data == u1.data);

    // // XML tests
    // serialize_xml(p0, "std_pair", "pair.xml");
    // deserialize_xml(p1, "std_pair", "pair.xml");
    // assert(p0 == p1);
    // serialize_xml(u0, "user", "user.xml");
    // deserialize_xml(u1, "user", "user.xml");
    // assert(u0.idx == u1.idx && u0.name == u1.name && u0.data == u1.data);
    std::cout << "All tests passed!\n";
    return 0;
}
