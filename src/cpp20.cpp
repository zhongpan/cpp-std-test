#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <vector>
#include <iostream>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

#ifndef _MSC_VER

TEST_CASE("Concepts") {


}

struct A {
  int x;
  int y;
  int z = 123;
};
TEST_CASE("Designated initializers") {
    A a {.x = 1, .z = 2}; // a.x == 1, a.y == 0, a.z == 2
    CHECK(a.x == 1);
    CHECK(a.y == 0);
    CHECK(a.z == 2);
}

TEST_CASE("Template syntax for lambdas") {

    auto f = []<typename T>(std::vector<T> v) {
    // ...
    };
    std::vector<int> v {1,2,3};
    f(v);
}


TEST_CASE("Range-based for loop with initializer") {
#if 0
    for (std::vector v{1, 2, 3}; auto& e : v) {
        std::cout << e;
    }
    // prints "123" 
#endif   
}

#endif