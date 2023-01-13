cmake_minimum_required(VERSION 3.15)

project(sylar LANGUAGES CXX)

set(CXX_STANDARD_REQUIRED 17)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# if (NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE Debug)
# endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

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
    utils/log.cc
)

add_library(sylar SHARED ${LIB_SRC})

add_executable(test tests/test.cc)
add_dependencies(test sylar)
target_include_directories(test PUBLIC utils/)
target_link_libraries(test sylar)