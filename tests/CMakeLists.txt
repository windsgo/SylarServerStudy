# tests/CMakeLists.txt

add_executable(test test.cc)
add_dependencies(test sylar)
target_include_directories(test PUBLIC 
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(test 
    sylar
)

add_executable(test_config test_config.cc)
add_dependencies(test_config sylar)
target_include_directories(test_config PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${YAML_CPP_INCLUDE_DIR}
)
target_link_libraries(test_config 
    sylar
    ${YAML_CPP_LIBRARIES}
)