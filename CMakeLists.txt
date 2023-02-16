cmake_minimum_required(VERSION 3.15)

project(sylar LANGUAGES CXX)

set(CXX_STANDARD_REQUIRED 17)
# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
endif()

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(sylar/utils.cmake)

# add_compile_options(-Wall -Werror)
add_compile_definitions(__ROOT_DIR__="${PROJECT_SOURCE_DIR}/")

# options
option(USE_BOOST_STACKTRACE "use boost::stacktrace and libbacktrace to get lines while enabling debug compilings" ON)

# YAML-CPP
find_package(YAML-CPP REQUIRED)
print_variable(YAML_CPP_INCLUDE_DIR)
print_variable(YAML_CPP_LIBRARIES)
print_variable(YAML-CPP_VERSION)

# Boost
find_package(Boost REQUIRED COMPONENTS filesystem)
print_variable(Boost_INCLUDE_DIR)
print_variable(Boost_LIBRARIES)
print_variable(Boost_VERSION)

# check if use libbacktrace and boost::stacktrace to print lines
message(STATUS "Checking if is going to use boost::backtrace")
if ((${CMAKE_BUILD_TYPE} STREQUAL Release) OR (${CMAKE_BUILD_TYPE} STREQUAL MinSizeRel))
    set(USE_BOOST_STACKTRACE OFF)
endif()
print_variable(CMAKE_BUILD_TYPE)
print_variable(USE_BOOST_STACKTRACE)
if (USE_BOOST_STACKTRACE)
    add_compile_definitions(BOOST_STACKTRACE_USE_BACKTRACE) 
    set(USE_BOOST_STACKTRACE_LINK_LIBRARIES dl backtrace)
    print_variable(USE_BOOST_STACKTRACE_LINK_LIBRARIES)
    message(STATUS "Result: USE boost::stacktrace and libbacktrace")
endif()
message(STATUS "Checked")

add_subdirectory(sylar)

add_subdirectory(tests)

