#include "doctest/doctest.h"


DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <string>
#include <memory> // std::make_unique
#include <chrono>
#include <array>
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END

TEST_CASE("Binary literals") {
    CHECK(0b110 == 6); // == 6
    CHECK(0b1111'1111 == 255); // == 255
}

TEST_CASE("Generic lambda expressions") {
    auto identity = [](auto x) { return x; };
    int three = identity(3); // == 3
    CHECK(three == 3);
    std::string foo = identity("foo"); // == "foo"
    CHECK(foo == "foo");
}


// This allows creating lambda captures initialized with arbitrary expressions. The name given to the captured value does not need to be related to any variables in the enclosing scopes and introduces a new name inside the lambda body. The initializing expression is evaluated when the lambda is created (not when it is invoked).
int factory(int i) { return i * 10; }
TEST_CASE("Lambda capture initializers") {

    SUBCASE("base") {
        auto f = [x = factory(2)] { return x; }; // returns 20

        auto generator = [x = 0] () mutable {
        // this would not compile without 'mutable' as we are modifying x on each call
        return x++;
        };
        auto a = generator(); // == 0
        auto b = generator(); // == 1
        auto c = generator(); // == 2  
    }
    SUBCASE("move") {
        auto p = std::make_unique<int>(1);

        //auto task1 = [=] { *p = 5; }; // ERROR: std::unique_ptr cannot be copied
        // vs.
        auto task2 = [p = std::move(p)] { *p = 5; }; // OK: p is move-constructed into the closure object
        // the original p is empty after task2 is created
    }
    SUBCASE("different name") {
        auto x = 1;
        auto f = [&r = x, x = x * 10] {
        ++r;
        return r + x;
        };
        f(); // sets x to 2 and returns 12
    }
}


// Deduce return type as `int`.
auto fun(int i) {
 return i;
}

template <typename T>
auto& f(T& t) {
  return t;
}

TEST_CASE("Return type deduction") {
    // Returns a reference to a deduced type.
    auto g = [](auto& x) -> auto& { return f(x); };
    int y = 123;
    int& z = g(y); // reference to `y`
    z = 456;
    CHECK(y == 456);
    CHECK(z == 456);
}

// The decltype(auto) type-specifier also deduces a type like auto does. However, it deduces return types while keeping their references and cv-qualifiers, while auto will not.

// Note: Especially useful for generic code!

// Return type is `int`.
auto f1(const int& i) {
 return i;
}

// Return type is `const int&`.
decltype(auto) g(const int& i) {
 return i;
}

TEST_CASE("decltype(auto)") {

    SUBCASE("var") {
        const int x = 0;
        auto x1 = x; // int
        decltype(auto) x2 = x; // const int
        int y = 0;
        int& y1 = y;
        auto y2 = y1; // int
        decltype(auto) y3 = y1; // int&
        int&& z = 0;
        auto z1 = std::move(z); // int
        decltype(auto) z2 = std::move(z); // int&&        
    }
    SUBCASE("return") {
        int x = 123;
        static_assert(std::is_same<const int&, decltype(f1(x))>::value == 0);
        static_assert(std::is_same<int, decltype(f1(x))>::value == 1);
        static_assert(std::is_same<const int&, decltype(g(x))>::value == 1);
    }
}

// In C++11, constexpr function bodies could only contain a very limited set of syntaxes, including (but not limited to): typedefs, usings, and a single return statement. In C++14, the set of allowable syntaxes expands greatly to include the most common syntax such as if statements, multiple returns, loops, etc.
constexpr int factorial(int n) {
  if (n <= 1) {
    return 1;
  } else {
    return n * factorial(n - 1);
  }
}
TEST_CASE("Relaxing constraints on constexpr functions") {
    static_assert(factorial(5) == 120); // == 120
}

// https://blog.csdn.net/lanchunhui/article/details/49835213

template<class T>
constexpr T pi = T(3.1415926535897932385L);  // variable template
 
template<class T>
T circular_area(T r) // function template
{
    return pi<T> * r * r; // pi<T> is a variable template instantiation
}
TEST_CASE("Variable templates") {
    CHECK(circular_area(2) == 2*2*3);
    CHECK(circular_area(2.0) == doctest::Approx(2.0*2.0*3.1415926535897932385L));
}

[[deprecated]]
void old_method() {};

[[deprecated("Use new_method instead")]]
void legacy_method() {};
TEST_CASE("[[deprecated]] attribute") {
    old_method();
}

// New user-defined literals for standard library types, including new built-in literals for chrono and basic_string. These can be constexpr meaning they can be used at compile-time. Some uses for these literals include compile-time integer parsing, binary literals, and imaginary number literals.
TEST_CASE("User-defined literals for standard library types") {
    using namespace std::chrono_literals;
    auto day = 24h;
    CHECK(day.count() == 24); // == 24
    CHECK(std::chrono::duration_cast<std::chrono::minutes>(day).count() == 1440); // == 1440    
}

template<typename Array, std::size_t... I>
decltype(auto) a2t_impl(const Array& a, std::integer_sequence<std::size_t, I...>) {
  return std::make_tuple(a[I]...);
}

template<typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
decltype(auto) a2t(const std::array<T, N>& a) {
  return a2t_impl(a, Indices());
}
TEST_CASE("Compile-time integer sequences") {
    auto t = a2t(std::array<int, 3> {1,2,3});
    CHECK(t == std::make_tuple(1,2,3));
}


TEST_CASE("std::make_unique") {


}