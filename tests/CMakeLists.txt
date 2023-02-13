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

add_executable(test_thread test_thread.cc)
add_dependencies(test_thread sylar)
target_include_directories(test_thread PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${YAML_CPP_INCLUDE_DIR}
)
target_link_libraries(test_thread 
    sylar
    ${YAML_CPP_LIBRARIES}
    pthread
)

add_executable(test_util test_util.cc)
add_dependencies(test_util sylar)
target_include_directories(test_util PUBLIC 
    ${PROJECT_SOURCE_DIR}
)
target_link_libraries(test_util 
    sylar
    pthread
)