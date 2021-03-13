cmake_minimum_required(VERSION 3.7)
project(cpp-std-test VERSION 0.1.0)

if(MSVC)
add_compile_options(/std:c++17) 
else()
# GCC
add_compile_options(-std=c++2a)
endif()

#if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
#  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
#  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
#                "${CMAKE_BINARY_DIR}/conan.cmake"
#                TLS_VERIFY ON)
#endif()
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-conan" "${CMAKE_SOURCE_DIR}/CMake-codecov/cmake" ${CMAKE_MODULE_PATH})
include(conan)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE ${PROJECT_SOURCE_DIR}
                    BUILD missing
                    REMOTE conan-center
                    SETTINGS ${settings})   
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)                    
conan_basic_setup()

# conan_cmake_run(CONANFILE conanfile.txt
#                 BASIC_SETUP 
#                 BUILD missing)

#include(CTest)
enable_testing()

find_package(codecov)

file(GLOB_RECURSE SRCS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(cpp-std-test ${SRCS})
target_link_libraries(cpp-std-test ${CONAN_LIBS})
if(NOT MSVC)
target_link_libraries(cpp-std-test pthread)
endif()
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