#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <vector>
#include <memory> // std::unique_ptr
#include <numeric> // std::accumulate
#include <cmath> // std::llround
#include <map>
#include <type_traits> // std::remove_reference
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm> // std::sort
#include <future> // std::async
#include <array>
#include <typeindex>
#include <string> // std::stoi
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END



template <typename T>
typename std::remove_reference<T>::type&& my_move(T&& arg) {
  return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

TEST_CASE("Move semantics && std::move") {
    SUBCASE("std::move") {
        std::vector<int> vec = {1,2,3,4};
        CHECK(vec.size() == 4);
        std::vector<int> vec1 = std::move(vec);
        CHECK(vec.empty());
        CHECK(vec1.size() == 4);
    }

    SUBCASE("std::uniqure_ptr") {
        std::unique_ptr<int> p(new int(1));
        CHECK(*p == 1);
        //std::unique_ptr<int> p1 = p;  // compile error
        std::unique_ptr<int> p2 = std::move(p);
        CHECK(*p2 == 1);
        CHECK(p.get() == nullptr);
    }
}

TEST_CASE("Rvalue references") {
    int x = 0; // `x` is an lvalue of type `int`
    int& xl = x; // `xl` is an lvalue of type `int&`
    //int&& xr = x; // compiler error -- `x` is an lvalue
    int&& xr2 = 0; // `xr2` is an lvalue of type `int&&` -- binds to the rvalue temporary, `0` 
    CHECK(xr2 == 0);
    xr2 = 3;   
    CHECK(xr2 == 3);
    int&& xr3 = (1 + 2);
    CHECK(xr3 == 3);
    xr3 = 4;
    CHECK(xr3 == 4);
}


// T& & becomes T&
// T& && becomes T&
// T&& & becomes T&
// T&& && becomes T&&

// Since C++14 or later:
//std::string f(auto&& t) {
//    return std::type_index(typeid(t));
//}

// Since C++11 or later:
template <typename T>
decltype(auto) f(T&& t) { // c++14: decltype(auto)
    return std::forward<T>(t);
}

struct BB {

};

#if defined _MSC_VER && _MSC_VER <= 1924 // MSVC2017不支持auto&&
template<typename T, typename V>
bool is_same_type(V&& v) { 
  return std::type_index(typeid(v)) == std::type_index(typeid(T));
}
#else
template<typename T>
bool is_same_type(auto&& v) { // c++17: auto&&
  return std::type_index(typeid(v)) == std::type_index(typeid(T));
}
#endif

template<typename T1, typename T2>
bool is_same_type() {
  return std::type_index(typeid(T1)) == std::type_index(typeid(T2));
}

TEST_CASE("Forwarding references") {
    SUBCASE("auto deduction") {
        int x = 0; // `x` is an lvalue of type `int`
        auto&& al = x; // `al` is an lvalue of type `int&` -- binds to the lvalue, `x`
        auto&& ar = 0; // `ar` is an lvalue of type `int&&` -- binds to the rvalue temporary, `0`
        CHECK(is_same_type<int&>(al));
        CHECK(is_same_type<int&&>(ar));
    }
    SUBCASE("template") {
        int x = 0;
        CHECK(is_same_type<int&&>(f(0))); // deduces as f(int&&)
        CHECK(is_same_type<int&>(f(x))); // deduces as f(int&)

        int& y = x;
        CHECK(is_same_type<int&>(f(y))); // deduces as f(int& &&) => f(int&)

        int&& z = 0; // NOTE: `z` is an lvalue with type `int&&`.
        CHECK(is_same_type<int&&>(std::move(z)));
        CHECK(is_same_type<int&>(f(z))); // deduces as f(int&& &) => f(int&) 理解：只要是左值就deduce为int&，只要是右值就deduce为int&&
        CHECK(is_same_type<int&&>(f(std::move(z)))); // deduces as f(int&& &&) => f(int&&)

        CHECK(is_same_type<BB&&>(f(BB{})));
    }
}


template <typename... T>
struct arity {
  constexpr static int value = sizeof...(T);
};

template <typename First, typename... Args>
auto sum(const First first, const Args... args) -> decltype(first) {
  const auto values = {first, args...}; // 这里保证了first和args类型一致，否则编译报错，所以decltype(first)也是ok的
  return std::accumulate(values.begin(), values.end(), First{0}); // TODO:First{0}初始化方式只能针对数字?
}

TEST_CASE("Variadic templates") {
    SUBCASE("sizeof") {
        static_assert(arity<>::value == 0);
        static_assert(arity<char, short, int>::value == 3);
        int n = arity<>::value;
        CHECK(n == 0);
        n = arity<char, short, int>::value;
        CHECK(n == 3);
    }
    SUBCASE("sum") {
        CHECK(sum(1, 2, 3, 4, 5) == 15);
        CHECK(sum(1, 2, 3) == 6);
        CHECK(sum(1.5, 2.0, 3.7) == 7.2);
    }
}

int sum(const std::initializer_list<int>& list) {
  int total = 0;
  for (auto& e : list) {
    total += e;
  }

  return total;
}
TEST_CASE("Initializer lists") {
    auto list = {1, 2, 3};
    CHECK(sum(list) == 6);
    CHECK(sum({1, 2, 3}) == 6);
    CHECK(sum({}) == 0);
}

TEST_CASE("Static assertions") {
    constexpr int x = 0;
    constexpr int y = 1;
    static_assert(x != y, "x == y");    
}

template <typename X, typename Y>
auto add(X x, Y y) -> decltype(x + y) { // c++14: decltype(auto).
  return x + y;
}

TEST_CASE("auto") {
    SUBCASE("") {
        auto a = 3.14; // double
        CHECK(is_same_type<double>(a));
        auto b = 1; // int
        CHECK(is_same_type<int>(b));
        auto& c = b; // int&
        CHECK(is_same_type<int&>(c));
        auto d = { 0 }; // std::initializer_list<int>
        CHECK(is_same_type<std::initializer_list<int>>(d));
        auto&& e = 1; // int&&
        CHECK(is_same_type<int&&>(e));
        auto&& f = b; // int&
        CHECK(is_same_type<int&>(f));
        auto g = new auto(123); // int*
        CHECK(is_same_type<int*>(g));
        const auto h = 1; // const int
        CHECK(is_same_type<int const>(h));
        auto i = 1, j = 2, k = 3; // int, int, int
        CHECK(is_same_type<int>(i));
        CHECK(is_same_type<int>(j));
        CHECK(is_same_type<int>(k));
        //auto l = 1, m = true, n = 1.61; // error -- `l` deduced to be int, `m` is bool
        //auto o; // error -- `o` requires initializer        
    }
    SUBCASE("vector") {
        std::vector<int> v = {1,2,3};
        // std::vector<int>::const_iterator cit = v.cbegin();
        auto cit = v.cbegin();
        CHECK(is_same_type<std::vector<int>::const_iterator>(cit));
    }
    SUBCASE("return") {
        auto r1 = add(1, 2); // == 3
        CHECK(r1 == 3);
        CHECK(is_same_type<int>(r1));
        auto r2 = add(1, 2.0); // == 3.0
        CHECK(r2 == 3.0);
        CHECK(is_same_type<double>(r2));
        auto r3 = add(1.5, 1.5); // == 3.0
        CHECK(r3 == 3.0);
        CHECK(is_same_type<double>(r3));
    }
}

// [] - captures nothing.
// [=] - capture local objects (local variables, parameters) in scope by value.
// [&] - capture local objects (local variables, parameters) in scope by reference.
// [this] - capture this pointer by value.
// [a, &b] - capture objects a by value, b by reference.

TEST_CASE("Lambda expressions") {
    int x = 1;

    auto getX = [=] { return x; };
    CHECK(getX() == 1); // == 1

    auto addX = [=](int y) { return x + y; };
    CHECK(addX(1) == 2); // == 2

    auto getXRef = [&]() -> int& { return x; };
    getXRef() = 2; // int& to `x`
    CHECK(x == 2); 

    auto f1 = [&x] { x = 3; }; // OK: x is a reference and modifies the original
    f1();
    CHECK(x == 3);

    //auto f2 = [x] { x = 4; }; // ERROR: the lambda can only perform const-operations on the captured value
    // vs.
    auto f3 = [x]() mutable { x = 4; }; // OK: the lambda can perform any operations on the captured value    
    f3();
    CHECK(x == 3); // 不会改变原值
}

// decltype和auto都可以用来推断类型，但是二者有几处明显的差异：

// 1.auto忽略顶层const，decltype保留顶层const；

// 2.对引用操作，auto推断出原有类型，decltype推断出引用；

// 3.对解引用操作，auto推断出原有类型，decltype推断出引用；

// 4.auto推断时会实际执行，decltype不会执行，只做分析。总之在使用中过程中和const、引用和指针结合时需要特别小心。

TEST_CASE("decltype") {

    int a = 1; // `a` is declared as type `int`
    decltype(a) b = a; // `decltype(a)` is `int`
    CHECK(is_same_type<decltype(a), int>());
    const int& c = a; // `c` is declared as type `const int&`
    decltype(c) d = a; // `decltype(c)` is `const int&`
    CHECK(is_same_type<decltype(c), int const&>());
    decltype(123) e = 123; // `decltype(123)` is `int`
    CHECK(is_same_type<decltype(123), int>());
    int&& f = 1; // `f` is declared as type `int&&`
    decltype(f) g = 1; // `decltype(f) is `int&&`
    CHECK(is_same_type<decltype(f), int&&>());
    decltype((a)) h = g; // `decltype((a))` is int& (双层括号表示引用)
    CHECK(is_same_type<decltype((a)), int&>());

    auto r = add(1, 2.0); // `decltype(x + y)` => `decltype(3.0)` => `double`
    CHECK(is_same_type<decltype(r), double>());
}

template <typename T>
using Vec = std::vector<T>;
TEST_CASE("Type aliases") {
    Vec<int> v; // std::vector<int>
    CHECK(is_same_type<Vec<int>, std::vector<int>>());
    using String = std::string;
    String s {"foo"};
    CHECK(is_same_type<String, std::string>());    
}

//https://www.sohu.com/a/339977072_216613
std::string foo(int) { return "int"; }
std::string foo(char*) { return "char*"; }
TEST_CASE("nullptr") {
    #ifdef _MSC_VER
    CHECK(is_same_type<decltype(NULL), int>()); // msvc cpp: 0,c: ((void *)0)
    #else
    CHECK(is_same_type<decltype(NULL), long long>()); // gcc cpp 64: 0LL,cpp 32: 0,c<3: ((void *)0)
    #endif
    //foo(NULL); // foo(0LL); error -- ambiguous, 0L和0LL都会隐式转换为int或char*
    CHECK(foo(0) == "int");
    CHECK(foo(nullptr) == "char*"); // calls foo(char*)    
}

// Specifying underlying type as `unsigned int`
enum class Color : unsigned int { Red = 0xff0000, Green = 0xff00, Blue = 0xff };
// `Red`/`Green` in `Alert` don't conflict with `Color`
enum class Alert : bool { Red, Green }; // 第一个默认值为false，第二个为true，元素个数不能超过bool的个数

TEST_CASE("Strongly-typed enums") {
    Color c = Color::Red;
    Alert a = Alert::Green;
    CHECK(is_same_type<decltype(Alert::Green), Alert>());
    bool r = (bool)Alert::Red; // 不会隐式转换，只能强制转换
    CHECK(r == false);
    bool g = (bool)Alert::Green;
    CHECK(g == true);
}


// https://en.cppreference.com/w/cpp/language/attributes
// [[noreturn]]
// [[carries_dependency]]
// [[deprecated]] （C++14）
// [[deprecated(“reason”)]]（C++14）
// [[fallthrough]]（C++17）
// [[nodiscard]]（C++17）
// [[maybe_unused]]（C++17）
// [[likely]]（C++20）
// [[unlikely]](C++20)
// [[no_unique_address]]（C++20）


// `noreturn` attribute indicates `f` doesn't return.
[[ noreturn ]] void f() { // 编译器会检查，如果有return语句会报警告
  throw "error";
}
TEST_CASE("Attributes") {

}


constexpr int square(int x) {
  return x * x;
}

int square2(int x) {
  return x * x;
}

struct Complex {
  constexpr Complex(double r, double i) : re{r}, im{i} { }
  constexpr double real() const { return re; }
  constexpr double imag() const { return im; }

private:
  double re;
  double im;
};

TEST_CASE("constexpr") {
    int a = square(2);  // mov DWORD PTR [rbp-4], 4

    int b = square2(2); // mov edi, 2
                        // call square2(int)
                        // mov DWORD PTR [rbp-8], eax

    static_assert(square(2) == 4);
    //static_assert(square2(2) == 4); // 编译错误：non-constant condition for static assertion

    const int x = 123;
    //constexpr const int& y = x; // error -- constexpr variable `y` must be initialized by a constant expression


    constexpr Complex I(0, 1);
    static_assert(I.real() == 0);
}


struct Foo {
  int foo;
  Foo(int foo) : foo{foo} {}
  Foo() : Foo(0) {}
  void bar() {}
};
TEST_CASE("Delegating constructors") {
    Foo foo;
    CHECK(foo.foo == 0); // == 0
}


//Any literal names not starting with an underscore are reserved and won't be invoked.

// `unsigned long long` parameter required for integer literal.
long long operator "" _celsius(unsigned long long tempCelsius) {
  return std::llround(tempCelsius * 1.8 + 32);
}

// `const char*` and `std::size_t` required as parameters.
int operator "" _int(const char* str, std::size_t) {
  return std::stoi(str);
}
TEST_CASE("User-defined literals") {
    CHECK(24_celsius == 75);
    CHECK("123"_int == 123);
}


// Specifies that a virtual function overrides another virtual function. If the virtual function does not override a parent's virtual function, throws a compiler error.
struct A1 {
  virtual void foo();
  void bar();
};

struct B1 : A1 {
  void foo() override; // correct -- B::foo overrides A::foo
  //void bar() override; // error -- A::bar is not virtual
  //void baz() override; // error -- B::baz does not override A::baz
};
TEST_CASE("Explicit virtual overrides") {


}


// Specifies that a virtual function cannot be overridden in a derived class or that a class cannot be inherited from.

struct A2 {
  virtual void foo();
};

struct B2 : A2 {
  virtual void foo() final;
};

struct C2 : B2 {
  //virtual void foo(); // error -- declaration of 'foo' overrides a 'final' function
};

struct A3 final {};
//struct B3 : A3 {}; // error -- base 'A' is marked 'final'

TEST_CASE("Final specifier") {

}

// A more elegant, efficient way to provide a default implementation of a function, such as a constructor.
// https://blog.csdn.net/Fluxay2008/article/details/99632874
struct A4 {
  A4() = default;
  A4(int x) : x{x} {}
  int x {1};
};

struct B4 {
  B4() : x{1} {}
  int x;
};

struct C4 : B4 {
  // Calls B::B
  C4() = default;
};

TEST_CASE("Default functions") {
    A4 a; // a.x == 1
    CHECK(a.x == 1);
    A4 a2 {123}; // a.x == 123
    CHECK(a2.x == 123);
    C4 c; // c.x == 1
    CHECK(c.x == 1);
}


// A more elegant, efficient way to provide a deleted implementation of a function. Useful for preventing copies on objects.
class A5 {
  int x;

public:
  A5(int x) : x{x} {};
  A5(const A5&) = delete;
  A5& operator=(const A5&) = delete;
};

TEST_CASE("Deleted functions") {
    A5 x {123};
    //A5 y = x; // error -- call to deleted copy constructor
    //y = x; // error -- operator= deleted
}



TEST_CASE("Range-based for loops") {
    std::array<int, 5> a {1, 2, 3, 4, 5};
    for (int x : a) x *= 2;
    // a == { 1, 2, 3, 4, 5 }
    CHECK(a == std::array<int, 5> {1,2,3,4,5});
    for (int& x : a) x *= 2;
    // a == { 2, 4, 6, 8, 10 }
    CHECK(a == std::array<int, 5> {2,4,6,8,10});
    for (auto& x : a) x *= 2;
    CHECK(a == std::array<int, 5> {4,8,12,16,20});
}

struct A6 {
  std::string s;
  A6() : s{"test"} {}
  A6(const A6& o) : s{o.s} {}
  A6(A6&& o) : s{std::move(o.s)} {}
  A6& operator=(A6&& o) {
   s = std::move(o.s);
   return *this;
  }
};

A6 f(A6 a) {
  return a;
}
TEST_CASE("Special member functions for move semantics") {
    A6 a1 = f(A6{}); // move-constructed from rvalue temporary
    A6 a2 = std::move(a1); // move-constructed using std::move
    A6 a3 = A6{};
    a2 = std::move(a3); // move-assignment using std::move
    a1 = f(A6{}); // move-assignment from rvalue temporary
}

struct A7 {
  A7(int) {}
  A7(int, int) {}
  A7(int, int, int) {}
};

struct A8 {
  A8(int) {}
  A8(int, int) {}
  A8(int, int, int) {}
  A8(std::initializer_list<int>) {}
};

TEST_CASE("Converting constructors") {
    A7 a {0, 0}; // calls A::A(int, int)
    A7 b(0, 0); // calls A::A(int, int)
    A7 c = {0, 0}; // calls A::A(int, int)
    A7 d {0, 0, 0}; // calls A::A(int, int, int)
    //A7 e {1.1}; // // Error narrowing conversion from double to int
    A7 f(1.1); // OK

    A8 a1 {0, 0}; // calls A::A(std::initializer_list<int>)
    A8 b1(0, 0); // calls A::A(int, int)
    A8 c1 = {0, 0}; // calls A::A(std::initializer_list<int>)
    A8 d1 {0, 0, 0}; // calls A::A(std::initializer_list<int>)    
}




struct A9 {
  operator bool() const { return true; }
};

struct B9 {
  explicit operator bool() const { return true; }
};
TEST_CASE("Explicit conversion functions") {
    A9 a;
    if (a); // OK calls A::operator bool()
    bool ba = a; // OK copy-initialization selects A::operator bool()

    B9 b;
    if (b); // OK calls B::operator bool()
    //bool bb = b; // error copy-initialization does not consider B::operator bool()
}


// All members of an inline namespace are treated as if they were part of its parent namespace, allowing specialization of functions and easing the process of versioning. This is a transitive property, if A contains B, which in turn contains C and both B and C are inline namespaces, C's members can be used as if they were on A.

namespace Program {
  namespace Version1 {
    int getVersion() { return 1; }
    bool isFirstVersion() { return true; }
  }
  inline namespace Version2 {
    int getVersion() { return 2; }
  }
}
TEST_CASE("Inline namespaces") {
    int version {Program::getVersion()};              // Uses getVersion() from Version2
    CHECK(version == 2);
    int oldVersion {Program::Version1::getVersion()}; // Uses getVersion() from Version1
    CHECK(oldVersion == 1);
    //bool firstVersion {Program::isFirstVersion()};    // Does not compile when Version2 is added
}

// Default initialization prior to C++11
class Human1 {
    Human1() : age{0} {}
  private:
    unsigned age;
};
// Default initialization on C++11
class Human2 {
  public:
    unsigned getAge() const { return age; }
  private:
    unsigned age {3};
};
TEST_CASE("Non-static data member initializers") {
  Human2 h;
  CHECK(h.getAge() == 3);

}


typedef std::map<int, std::map <int, std::map <int, int> > > cpp98LongTypedef;
typedef std::map<int, std::map <int, std::map <int, int>>>   cpp11LongTypedef;
TEST_CASE("Right angle brackets") {

}

// Member functions can now be qualified depending on whether *this is an lvalue or rvalue reference.
struct Bar {
  // ...
  int x;
};

struct Foo2 {
  Bar getBar() & { return bar; }
  Bar getBar() const& { return bar; }
  Bar getBar() && { return std::move(bar); }
  Bar getBar() const&& { return std::move(bar); }
//private:
  Bar bar;
};
TEST_CASE("Ref-qualified member functions") {
  Foo2 foo{};
  Bar bar = foo.getBar(); // calls `Bar getBar() &`

  const Foo2 foo2{};
  Bar bar2 = foo2.getBar(); // calls `Bar Foo::getBar() const&`

  Foo2{}.getBar(); // calls `Bar Foo::getBar() &&`
  std::move(foo).getBar(); // calls `Bar Foo::getBar() &&`

  std::move(foo2).getBar(); // calls `Bar Foo::getBar() const&&`
}



int f1() {
  return 123;
}
// vs.
auto f2() -> int {
  return 123;
}

auto f3 = []() -> int {
  return 123;
};

// NOTE: This does not compile!
// template <typename T, typename U>
// decltype(a + b) add(T a, U b) {
//     return a + b;
// }

// Trailing return types allows this:
template <typename T, typename U>
auto add2(T a, U b) -> decltype(a + b) { // c++14: decltype(auto)
    return a + b;
}

TEST_CASE("Trailing return types") {

}

void func1() noexcept;        // does not throw
void func2() noexcept(true);  // does not throw
void func3() throw();         // does not throw

void func4() noexcept(false) {} // may throw

void g() noexcept {
    func4();          // valid, even if f throws
    throw 42;     // valid, effectively a call to std::terminate
}
TEST_CASE("Noexcept specifier") {
  //g();  //std::terminate
}


template <typename T>
T&& my_forward(typename std::remove_reference<T>::type& arg) {
  return static_cast<T&&>(arg);
}

struct A10 {
  A10() = default;
  A10(const A10& o) { std::cout << "copied" << std::endl; }
  A10(A10&& o) { std::cout << "moved" << std::endl; }
};
template <typename T>
A10 wrapper(T&& arg) {
  return A10{std::forward<T>(arg)}; // std::move和std::forward的区别是move总是返回右值引用，而forward根据输入，只有输入是又值才返回右值引用
}
TEST_CASE("std::forward") {
  wrapper(A10{}); // moved
  A10 a;
  wrapper(a); // copied
  wrapper(std::move(a)); // moved
}

// emplace_back: https://blog.csdn.net/p942005405/article/details/84764104

void thread_fun(bool clause) { /* do something... */ }
TEST_CASE("std::thread") {
  std::vector<std::thread> threadsVector;
  threadsVector.emplace_back([]() {
    // Lambda function that will be invoked
  });
  threadsVector.emplace_back(thread_fun, true);  // thread will run foo(true)
  for (auto& thread : threadsVector) {
    thread.join(); // Wait for threads to finish
  }
}


TEST_CASE("std::to_string") {
  CHECK(std::to_string(1.2) == "1.200000"); // == "1.2"  有精度问题
  CHECK(std::to_string(123) == "123"); // == "123"
}


TEST_CASE("Type traits") {
  static_assert(std::is_integral<int>::value);
  static_assert(std::is_same<int, int>::value);
  static_assert(std::is_same<std::conditional<true, int, double>::type, int>::value);
}

// Note: Prefer using the std::make_X helper functions as opposed to using constructors. See the sections for std::make_unique(C++14) and std::make_shared(C++11).

TEST_CASE("Smart pointers") {
  SUBCASE("unique_ptr") {
    std::unique_ptr<Foo> p1 {new Foo{}};  // `p1` owns `Foo`
    if (p1) {
      p1->bar();
    }

    {
      std::unique_ptr<Foo> p2 {std::move(p1)};  // Now `p2` owns `Foo`
      f(*p2);
      CHECK(p1.get() == nullptr);

      p1 = std::move(p2);  // Ownership returns to `p1` -- `p2` gets destroyed
      CHECK(p2.get() == nullptr);
    }

    if (p1) {
      p1->bar();
    }
    // `Foo` instance is destroyed when `p1` goes out of scope    
  }
  SUBCASE("shared_ptr") {
    std::shared_ptr<int> p1 {new int{}};
    // Perhaps these take place in another threads?
    // foo(p1);
    // bar(p1);
    // baz(p1);    
  }
}

TEST_CASE("std::chrono") {
  std::chrono::time_point<std::chrono::steady_clock> start, end;
  start = std::chrono::steady_clock::now();
  // Some computations...
  using namespace std::chrono_literals; // C++14
  std::this_thread::sleep_for(10ms);
  end = std::chrono::steady_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;
  double t = elapsed_seconds.count(); // t number of seconds, represented as a `double`  
  CHECK(t >= 0.01);
}


// Tuples are a fixed-size collection of heterogeneous values. Access the elements of a std::tuple by unpacking using std::tie, or using std::get.


TEST_CASE("Tuples") {
  // `playerProfile` has type `std::tuple<int, const char*, const char*>`.
  auto playerProfile = std::make_tuple(51, "Frans Nielsen", "NYI");
  CHECK(std::get<0>(playerProfile) == 51); // 51
  #ifndef _MSC_VER // MSVC不通过 ???
  CHECK(std::get<1>(playerProfile) == "Frans Nielsen"); // "Frans Nielsen" 
  CHECK(std::get<2>(playerProfile) == "NYI"); // "NYI"
  #endif

  int t1;
  std::string t2,t3;

  t1 = std::get<0>(playerProfile);
  t2 = std::get<1>(playerProfile);
  t3 = std::get<2>(playerProfile);
  CHECK(t1 == 51);
  CHECK(t2 == "Frans Nielsen");
  CHECK(t3 == "NYI");

  std::tie(t1,t2,t3) = playerProfile;
  CHECK(t1 == 51);
  CHECK(t2 == "Frans Nielsen");
  CHECK(t3 == "NYI");

}


// or ignored values. In C++17, structured bindings should be used instead.
TEST_CASE("std::tie") {
  // With tuples...
  std::string playerName;
  std::tie(std::ignore, playerName, std::ignore) = std::make_tuple(91, "John Tavares", "NYI");
  CHECK(playerName == "John Tavares");

  // With pairs...
  std::string yes, no;
  std::tie(yes, no) = std::make_pair("yes", "no");  
  CHECK(yes == "yes");
  CHECK(no == "no");
}

TEST_CASE("std::array") {
  std::array<int, 3> a = {2, 1, 3};
  std::sort(a.begin(), a.end()); // a == { 1, 2, 3 }
  for (int& x : a) x *= 2; // a == { 2, 4, 6 }
  CHECK(a == std::array<int, 3> {2,4,6});
}

// unordered_set
// unordered_multiset
// unordered_map
// unordered_multimap
TEST_CASE("Unordered containers") {


}


TEST_CASE("std::make_shared") {


}

// std::ref(val) is used to create object of type std::reference_wrapper that holds reference of val. Used in cases when usual reference passing using & does not compile or & is dropped due to type deduction. std::cref is similar but created reference wrapper holds a const reference to val.
TEST_CASE("std::ref") {

  // create a container to store reference of objects.
  auto val = 99;
  auto _ref = std::ref(val);
  _ref++;
  auto _cref = std::cref(val);
  //_cref++; does not compile
  std::vector<std::reference_wrapper<int>>vec; // vector<int&>vec does not compile
  vec.push_back(_ref); // vec.push_back(&i) does not compile
  CHECK(val == 100); 
  CHECK(vec[0] == 100); 
  CHECK(_cref == 100); 
  _ref++;
  CHECK(val == 101); 
  CHECK(vec[0] == 101); 
  CHECK(_cref == 101); 
}

int async_task() {
  /* Do something here, then return the result. */
  return 1000;
}
TEST_CASE("Memory model") {
  auto handle = std::async(std::launch::async, async_task);  // create an async task
  auto result = handle.get();  // wait for the result
  CHECK(result == 1000);
}

template <typename T>
int CountTwos(const T& container) {
  return std::count_if(std::begin(container), std::end(container), [](int item) {
    return item == 2;
  });
}
TEST_CASE("std::begin/end") {
  std::vector<int> vec = {2, 2, 43, 435, 4543, 534};
  int arr[8] = {2, 43, 45, 435, 32, 32, 32, 32};
  auto a = CountTwos(vec); // 2
  auto b = CountTwos(arr);  // 1
  CHECK(a == 2);
  CHECK(b == 1);
}