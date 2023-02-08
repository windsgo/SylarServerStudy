# sylar/CMakeLists.txt

set(LIB_SRC
    log.cc
    util.cc
    config.cc
    thread.cc
)

add_library(sylar SHARED ${LIB_SRC})
target_include_directories(sylar PUBLIC 
    ${PROJECT_SOURCE_DIR}/sylar 
    ${PROJECT_SOURCE_DIR} 
    ${YAML_CPP_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)
target_link_libraries(sylar ${YAML_CPP_LIBRARIES} ${Boost_LIBRARIES} pthread)