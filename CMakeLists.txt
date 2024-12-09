# @file
# CMake build and packaging configuration for this single-header library.

cmake_minimum_required(VERSION 3.28)
project(largemelon
	DESCRIPTION "Single-header transitive reduction library"
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()
find_package(doctest REQUIRED)

add_executable(test_largemelon "test/test_largemelon.cpp")
target_include_directories(test_largemelon PRIVATE ${PROJECT_ROOT_DIR})
set_target_properties(test_largemelon PROPERTIES COMPILE_DEFINITIONS
	DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
add_test(NAME test_largemelon COMMAND test_largemelon)
