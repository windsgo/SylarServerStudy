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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

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

set(LIB_SRC
    sylar/log.cc
    sylar/util.cc
    sylar/config.cc
)

# YAML-CPP
find_package(YAML-CPP REQUIRED)
print_variable(YAML_CPP_INCLUDE_DIR)
print_variable(YAML_CPP_LIBRARIES)
print_variable(YAML-CPP_VERSION)

add_library(sylar SHARED ${LIB_SRC})
target_include_directories(sylar PUBLIC ${PROJECT_SOURCE_DIR}/sylar ${PROJECT_SOURCE_DIR} ${YAML_CPP_INCLUDE_DIR})
target_link_libraries(sylar ${YAML_CPP_LIBRARIES})

add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(test sylar)

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
target_include_directories(test_config PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(test_config sylar)