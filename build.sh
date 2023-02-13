#!/bin/sh
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
    -DCMAKE_BUILD_TYPE:STRING=Debug -B build \
    -G "Unix Makefiles" \
    -DUSE_BOOST_STACKTRACE=ON # use boost::stacktrace or not

cmake --build build -j4