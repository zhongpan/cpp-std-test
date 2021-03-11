#include "doctest/doctest.h"

DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN
#include <utility> // std::integer_sequence
#include <unordered_map> // std::unordered_map
#include <variant> // std::variant
#include <any> // std::any
#include <functional> // std::invoke
//#include <filesystem> // gcc8.1.0编译不过
#include <cstddef> // std::to_integer
#include <map> // std::map
#include <string>
#include <set>
#include <algorithm>
#include <optional>
//#include <execution> // std::execution::par 不支持
DOCTEST_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END


// Automatic template argument deduction much like how it's done for functions, but now including class constructors.
template <typename T = float>
struct MyContainer {
  T val;
  MyContainer() : val{} {}
  MyContainer(T val) : val{val} {}
  // ...
};

TEST_CASE("Template argument deduction for class templates") {
    MyContainer c1 {1}; // OK MyContainer<int>
    MyContainer c2; // OK MyContainer<float>
}

template <auto... seq>
struct my_integer_sequence {
  // Implementation here ...
};
TEST_CASE("Declaring non-type template parameters with auto") {
    // Explicitly pass type `int` as template argument.
    auto seq = std::integer_sequence<int, 0, 1, 2>();
    // Type is deduced to be `int`.
    auto seq2 = my_integer_sequence<0, 1, 2>();  
}


// A fold expression performs a fold of a template parameter pack over a binary operator.

// 1. An expression of the form (... op e) or (e op ...), where op is a fold-operator and e is an unexpanded parameter pack, are called unary folds.
// 2. An expression of the form (e1 op ... op e2), where op are fold-operators, is called a binary fold. Either e1 or e2 is an unexpanded parameter pack, but not both.

template <typename... Args>
bool logicalAnd(Args... args) {
    // Binary folding.
    return (true && ... && args);
}

template <typename... Args>
auto sum(Args... args) {
    // Unary folding.
    return (... + args);
}
TEST_CASE("Folding expressions") {
  bool b = true;
  bool& b2 = b;
  CHECK(logicalAnd(b, b2, true) == true); // == true

  CHECK(sum(1.0, 2.0f, 3) == 6.0); // == 6.0
}

// Changes to auto deduction when used with the uniform initialization syntax. Previously, auto x {3}; deduces a std::initializer_list<int>, which now deduces to int.

TEST_CASE("New rules for auto deduction from braced-init-list") {

  //auto x1 {1, 2, 3}; // error: not a single element
  auto x2 = {1, 2, 3}; // x2 is std::initializer_list<int>
  auto x3 {3}; // x3 is int
  auto x4 {3.0}; // x4 is double
}


constexpr int addOne(int n) {
  return [n] { return n + 1; }();
}
TEST_CASE("constexpr lambda") {
  auto identity = [](int n) constexpr { return n; };
  static_assert(identity(123) == 123);
  constexpr auto add = [](int x, int y) {
    auto L = [=] { return x; };
    auto R = [=] { return y; };
    return [=] { return L() + R(); };
  };

  static_assert(add(1, 2)() == 3);


  static_assert(addOne(1) == 2);  
}


struct MyObj {
  int value {123};
  auto getValueCopy() {
    return [*this] { return value; };
  }
  auto getValueRef() {
    return [this] { return value; };
  }
};

TEST_CASE("Lambda capture this by value") {

  MyObj mo;
  auto valueCopy = mo.getValueCopy();
  auto valueRef = mo.getValueRef();
  mo.value = 321;
  CHECK(valueCopy() == 123); // 123
  CHECK(valueRef() == 321); // 321  
}

// The inline specifier can be applied to variables as well as to functions. A variable declared inline has the same semantics as a function declared inline.

// 只能用在全局作用域，不能用在局部作用域

// Disassembly example using compiler explorer.
struct S1 { int x; };
inline S1 x1 = S1{321}; // mov esi, dword ptr [x1]
                        // x1: .long 321

S1 x2 = S1{123};        // mov eax, dword ptr [.L_ZZ4mainE2x2]
                        // mov dword ptr [rbp - 8], eax
                        // .L_ZZ4mainE2x2: .long 123

// It can also be used to declare and define a static member variable, such that it does not need to be initialized in the source file.
struct S2 {
  S2() : id{count++} {}
  ~S2() { count--; }
  int id;
  static inline int count{0}; // declare and initialize count to 0 within the class
};
TEST_CASE("Inline variables") {
  
}


// Using the namespace resolution operator to create nested namespace definitions.


namespace A {
  namespace B {
    namespace C {
      int i;
    }
  }
}
// vs.
namespace A::B::C {
  int j;
}
TEST_CASE("Nested namespaces") {

}

// A proposal for de-structuring initialization, that would allow writing auto [ x, y, z ] = expr; where the type of expr was a tuple-like object, whose elements would be bound to the variables x, y, and z (which this construct declares). Tuple-like objects include std::tuple, std::pair, std::array, and aggregate structures.

using Coordinate = std::pair<int, int>;
Coordinate origin() {
  return Coordinate{0, 0};
}

TEST_CASE("Structured bindings") {
  const auto [ x, y ] = origin();
  CHECK(x == 0); // == 0
  CHECK(y == 0); // == 0


  std::unordered_map<std::string, int> mapping {
    {"a", 1},
    {"b", 2},
    {"c", 3}
  };

  // Destructure by reference.
  for (const auto& [key, value] : mapping) {
    // Do something with key and value
  }
}

#if 0
{
  std::lock_guard<std::mutex> lk(mx);
  if (v.empty()) v.push_back(val);
}
// vs.
if (std::lock_guard<std::mutex> lk(mx); v.empty()) {
  v.push_back(val);
}

Foo gadget(args);
switch (auto s = gadget.status()) {
  case OK: gadget.zip(); break;
  case Bad: throw BadFoo(s.message());
}
// vs.
switch (Foo gadget(args); auto s = gadget.status()) {
  case OK: gadget.zip(); break;
  case Bad: throw BadFoo(s.message());
}
#endif
TEST_CASE("Selection statements with initializer") {

  
}

// Write code that is instantiated depending on a compile-time condition.

template <typename T>
constexpr bool isIntegral() {
  if constexpr (std::is_integral<T>::value) {
    return true;
  } else {
    return false;
  }
}

struct S3 {};
TEST_CASE("constexpr if") {

  static_assert(isIntegral<int>() == true);
  static_assert(isIntegral<char>() == true);
  static_assert(isIntegral<double>() == false);

  static_assert(isIntegral<S3>() == false);
}


TEST_CASE("UTF-8 character literals") {
  char x = u8'x';
}

enum byte : unsigned char {};
TEST_CASE("Direct list initialization of enums") {
  byte b {0}; // OK
  //byte c {-1}; // ERROR
  byte d = byte{1}; // OK
  //byte e = byte{256}; // ERROR
}


#if 0
switch (n) {
  case 1: [[fallthrough]]
    // ...
  case 2:
    // ...
    break;
}

[[nodiscard]] bool do_something() {
  return is_success; // true for success, false for failure
}

do_something(); // warning: ignoring return value of 'bool do_something()',
                // declared with attribute 'nodiscard'
// Only issues a warning when `error_info` is returned by value.
struct [[nodiscard]] error_info {
  // ...
};

error_info do_something() {
  error_info ei;
  // ...
  return ei;
}

do_something(); // warning: ignoring returned value of type 'error_info',
                // declared with attribute 'nodiscard'

void my_callback(std::string msg, [[maybe_unused]] bool error) {
  // Don't care if `msg` is an error message, just log it.
  log(msg);
}

#endif

TEST_CASE("fallthrough, nodiscard, maybe_unused attributes") {

}


TEST_CASE("std::variant") {
  std::variant<int, double> v {12};
  CHECK(std::get<int>(v) == 12); // == 12
  CHECK(std::get<0>(v) == 12); // == 12
  v = 12.1;
  CHECK(std::get<double>(v) == 12.1); // == 12.1
  CHECK(std::get<1>(v) == 12.1); // == 12.1
  //CHECK_THROWS_WITH(std::get<int>(v) == 12, "Unexpected index"); // gcc exception: "Unexpected index"
  CHECK_THROWS(std::get<int>(v) == 12);
}

std::optional<std::string> create(bool b) {
  if (b) {
    return "Godzilla";
  } else {
    return {};
  }
}
TEST_CASE("std::optional") {
  CHECK(create(false).value_or("empty") == "empty"); // == "empty"
  CHECK(create(true).value() == "Godzilla"); // == "Godzilla"
  // optional-returning factory functions are usable as conditions of while and if
  if (auto str = create(true)) {
    // ...
  }
}


TEST_CASE("std::any") {
  std::any x {5};
  CHECK(x.has_value()); // == true
  CHECK(std::any_cast<int>(x) == 5); // == 5
  std::any_cast<int&>(x) = 10;
  CHECK(std::any_cast<int>(x) == 10); // == 10
}


// A non-owning reference to a string. Useful for providing an abstraction on top of strings (e.g. for parsing).

TEST_CASE("std::string_view") {
  // Regular strings.
  std::string_view cppstr {"foo"};
  // Wide strings.
  std::wstring_view wcstr_v {L"baz"};
  // Character arrays.
  char array[3] = {'b', 'a', 'r'};
  std::string_view array_v(array, std::size(array));
  std::string str {"   trim me"};
  std::string_view v {str};
  v.remove_prefix(std::min(v.find_first_not_of(" "), v.size()));
  CHECK(str == "   trim me"); //  == "   trim me"
  CHECK(v == "trim me"); // == "trim me"
}

template <typename Callable>
class Proxy {
  Callable c;
public:
  Proxy(Callable c): c(c) {}
  template <class... Args>
  decltype(auto) operator()(Args&&... args) {
    // ...
    return std::invoke(c, std::forward<Args>(args)...);
  }
};
TEST_CASE("std::invoke") {
  auto add = [](int x, int y) {
    return x + y;
  };
  Proxy<decltype(add)> p {add};
  CHECK(p(1, 2) == 3); // == 3
}


TEST_CASE("std::apply") {

  auto add = [](int x, int y) {
    return x + y;
  };
  CHECK(std::apply(add, std::make_tuple(1, 2)) == 3); // == 3  
}


TEST_CASE("std::filesystem") {
#if 0
  const auto bigFilePath {"bigFileToCopy"};
  if (std::filesystem::exists(bigFilePath)) {
    const auto bigFileSize {std::filesystem::file_size(bigFilePath)};
    std::filesystem::path tmpPath {"/tmp"};
    if (std::filesystem::space(tmpPath).available > bigFileSize) {
      std::filesystem::create_directory(tmpPath.append("example"));
      std::filesystem::copy_file(bigFilePath, tmpPath.append("newFile"));
    }
  }
#endif
}

// The new std::byte type provides a standard way of representing data as a byte. Benefits of using std::byte over char or unsigned char is that it is not a character type, and is also not an arithmetic type; while the only operator overloads available are bitwise operations.

// Note that std::byte is simply an enum, and braced initialization of enums become possible thanks to direct-list-initialization of enums.

TEST_CASE("std::byte") {
  std::byte a {0};
  std::byte b {0xFF};
  int i = std::to_integer<int>(b); // 0xFF
  CHECK(i == 0xff);
  std::byte c = a & b;
  int j = std::to_integer<int>(c); // 0
  CHECK(j == 0);
}


TEST_CASE("Splicing for maps and sets") {
  // Moving elements from one map to another:
  std::map<int, std::string> src {{1, "one"}, {2, "two"}, {3, "buckle my shoe"}};
  std::map<int, std::string> dst {{3, "three"}};
  dst.insert(src.extract(src.find(1))); // Cheap remove and insert of { 1, "one" } from `src` to `dst`.
  dst.insert(src.extract(2)); // Cheap remove and insert of { 2, "two" } from `src` to `dst`.
  // dst == { { 1, "one" }, { 2, "two" }, { 3, "three" } };

  // Inserting an entire set:
  std::set<int> src1 {1, 3, 5};
  std::set<int> dst1 {2, 4, 5};
  dst1.merge(src1);
  // src == { 5 }
  // dst == { 1, 2, 3, 4, 5 }

  // Inserting elements which outlive the container:
#if 0  
  auto elementFactory() {
    std::set<...> s;
    s.emplace(...);
    return s.extract(s.begin());
  }
  s2.insert(elementFactory());
#endif

  // Changing the key of a map element:

  std::map<int, std::string> m {{1, "one"}, {2, "two"}, {3, "three"}};
  auto e = m.extract(2);
  e.key() = 4;
  m.insert(std::move(e));
  // m == { { 1, "one" }, { 3, "three" }, { 4, "two" } }
}

// Many of the STL algorithms, such as the copy, find and sort methods, started to support the parallel execution policies: seq, par and par_unseq which translate to "sequentially", "parallel" and "parallel unsequenced".

TEST_CASE("Parallel algorithms") {
#if 0 // gcc 8.1不支持 https://zh.cppreference.com/w/cpp/compiler_support 依赖Intel tbb
  std::vector<int> longVector;
  // Find element using parallel execution policy
  auto result1 = std::find(std::execution::par, std::begin(longVector), std::end(longVector), 2);
  // Sort elements using sequential execution policy
  auto result2 = std::sort(std::execution::seq, std::begin(longVector), std::end(longVector));
#endif
}