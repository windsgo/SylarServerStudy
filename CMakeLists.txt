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


add_compile_options(-Wall -Werror)

function(print_variable x)
    list(LENGTH ${x} len)
    if (${len} GREATER 1 AND ${len} LESS 10)
        message(STATUS "[*PRINT*] \${${x}}:")
        foreach(_x ${${x}})
            message(STATUS "          ${_x}")
        endforeach()
    else()
        message(STATUS "[*PRINT*] \${${x}}:${${x}}")
    endif()
endfunction()

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

add_subdirectory(sylar)

add_subdirectory(tests)

