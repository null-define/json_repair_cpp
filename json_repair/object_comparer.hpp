#ifndef OBJECT_COMPARER_HPP
#define OBJECT_COMPARER_HPP

#include <map>
#include <string>
#include <type_traits>
#include <vector>

class ObjectComparer {
public:
    template < typename T, typename U > static bool is_same_object(const T& obj1, const U& obj2);

    template < typename T > static bool is_strictly_empty(const T& value);
};

template < typename T, typename U >
bool ObjectComparer::is_same_object(const T& obj1, const U& obj2) {
    return obj1 == obj2;
}

template < typename T > bool ObjectComparer::is_strictly_empty(const T& value) {
    return value.empty();
}

#endif