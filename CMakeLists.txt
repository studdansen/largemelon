# @file
# @brief CMake build and packaging configuration for this single-header
#   library.
#
# @todo Make building of unit tests user-toggleable.

cmake_minimum_required(VERSION 3.28)
project(largemelon
	DESCRIPTION "Single-header transitive reduction library"
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

include(CTest)
enable_testing()

# include C++ doctest framework, and fetch Git repo if it's not installed
find_package(doctest 2.4.12 QUIET)
if(doctest_FOUND)
	set(_INCLUDE_DIRS ${PROJECT_ROOT_DIR})
else()
	FetchContent_Declare(doctest
		GIT_REPOSITORY https://github.com/doctest/doctest.git
		GIT_TAG v2.4.12
		GIT_SHALLOW TRUE
	)
	message(DEBUG "Getting doctest from Git repo...")
	FetchContent_GetProperties(doctest)
	FetchContent_MakeAvailable(doctest)
	set(_INCLUDE_DIRS ${PROJECT_ROOT_DIR} ${doctest_SOURCE_DIR})
endif()

add_executable(test_largemelon "test/test_largemelon.cpp")
target_include_directories(test_largemelon PRIVATE ${_INCLUDE_DIRS})
target_link_libraries(test_largemelon PUBLIC doctest::doctest)
target_compile_definitions(test_largemelon PUBLIC
	DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
add_test(NAME test_largemelon COMMAND test_largemelon)
