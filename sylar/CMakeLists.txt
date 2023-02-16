# sylar/CMakeLists.txt

set(LIB_SRC
    fiber.cc
    log.cc
    util.cc
    config.cc
    thread.cc
)

# add_compile_definitions(SYLAR_LOG_MUTEX) 

add_library(sylar SHARED ${LIB_SRC})

# SYLAR_LOG_MUTEX or SYLAR_LOG_SPINLOCK or SYLAR_LOG_CASLOCK
target_compile_definitions(sylar PUBLIC SYLAR_LOG_MUTEX) 

# SYLAR_CONFIG_RWMUTEX
target_compile_definitions(sylar PUBLIC SYLAR_CONFIG_RWMUTEX) 

# SYLAR_LOG_FILE_APPEND, if defined, filelog will append to existing file
target_compile_definitions(sylar PUBLIC SYLAR_LOG_FILE_APPEND) 

# SYLAR_FIBER_RETURN_USE_UCLINK
# if defined, use ucb->uc_link to return fiber to main_fiber
target_compile_definitions(sylar PUBLIC SYLAR_FIBER_RETURN_USE_UCLINK)

target_include_directories(sylar PUBLIC 
    ${PROJECT_SOURCE_DIR}/sylar 
    ${PROJECT_SOURCE_DIR} 
    ${YAML_CPP_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(sylar 
    ${YAML_CPP_LIBRARIES} 
    ${Boost_LIBRARIES} 
    pthread 
    ${USE_BOOST_STACKTRACE_LINK_LIBRARIES}# used in posix platforms, not msvc; libbacktrace used to get lines 
)
