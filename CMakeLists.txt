cmake_minimum_required(VERSION 3.7)
project(cpp-std-test VERSION 0.1.0)

if(MSVC)
add_compile_options(/std:c++17) 
else()
# GCC
add_compile_options(-std=c++2a)
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

#include(conan.cmake)


conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conan-center
                    SETTINGS ${settings})              

#include(CTest)
enable_testing()

find_package(codecov)

file(GLOB_RECURSE SRCS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(cpp-std-test ${SRCS})
target_link_libraries(cpp-std-test ${CONAN_LIBS} pthread)
install(TARGETS cpp-std-test DESTINATION bin)
add_coverage(cpp-std-test)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/pack)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
        ${PROJECT_SOURCE_DIR}/build
        ${PROJECT_SOURCE_DIR}/pack
        ${PROJECT_SOURCE_DIR}/.git
        ${PROJECT_SOURCE_DIR}/.gitignore
        ${PROJECT_SOURCE_DIR}/.vscode)
include(CPack)

enable_testing()
add_test(NAME mytest COMMAND cpp-std-test)

coverage_evaluate()