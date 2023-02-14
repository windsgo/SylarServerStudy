<div align="center">

# Sylar Network Server Study Code

<br>
<div>
    <img alt="C++" src="https://img.shields.io/badge/c++-17-%2300599C?logo=cplusplus">
    <img alt="platform" src="https://img.shields.io/badge/platform-Linux-blueviolet">
</div>
<div>
    <!-- <img alt="license" src="https://img.shields.io/github/license/windsgo/SylarServerStudy"> -->
    <img alt="commit" src="https://img.shields.io/github/commit-activity/m/windsgo/SylarServerStudy?color=%23ff69b4">
    <img alt="stars" src="https://img.shields.io/github/stars/windsgo/SylarServerStudy?style=social">
</div>
<br>

</div>

## Introduction

A Repo with sylar network server study codes, including self-optimized codes.

## Versions

- ```KUbuntu 20.04```
- ```gcc 9.4.0 x86_64-linux-gnu```
- ```cmake 3.16.3```

## External Dependencies

- libbacktrace and libdl (if enabled "```Debug```" or "```RelWithDebInfo```")

    ```libbacktrace``` and ```libdl``` are a linux built-in libraries.
    
    > Note: ```libdl``` should be linked whenever ```boost::stacktrace``` is used with ```gcc```
    
    It is used and linked by ```libsylar``` while 
    - "```Debug```" or "```RelWithDebInfo```" is configured
    - ```USE_BOOST_BACKTRACE=ON```

    If not compiled with debug info, we use c api ```::backtrace``` and ```::backtrace_symbol``` to get backtrace info.
    
    If we use c api ```::backtrace``` and ```::backtrace_symbol```, we get things like:
    
    ```
    2023-02-13 17:59:54 [ERROR][root](t:146705)(f:0) tests/test_util.cc:11  ASSERTION: false
    backtrace:
     -- 0# bin/test_util(+0x2f93) [0x55f682682f93]
     -- 1# bin/test_util(+0x273f) [0x55f68268273f]
     -- 2# /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf3) [0x7f781e0c5083]
     -- 3# bin/test_util(+0x29be) [0x55f6826829be]
    ```
    
    But if we use ```boost::stacktrace``` and ```libbacktrace```, we can get prettier and more readable backtrace as following:
    
    ```
    2023-02-13 18:01:46 [ERROR][root](t:146991)(f:0) tests/test_util.cc:11  ASSERTION: false
    backtrace:
     -- 0# test_assert() at /home/windsgo/Documents/codes/SylarServerStudy/tests/test_util.cc:11
     -- 1# main at /home/windsgo/Documents/codes/SylarServerStudy/tests/test_util.cc:23
     -- 2# __libc_start_main at ../csu/libc-start.c:342
     -- 3# _start in bin/test_util
    ```

- boost
    
    - ```boost::lexical_cast```
    - ```boost::filesystem```
    - ```boost::stacktrace```(if enabled "```Debug```" or "```RelWithDebInfo```")
    
    
    ```bash
    sudo apt-get install libboost-all-dev
    ```

- yaml-cpp

    ```bash
    sudo apt-get install libyaml-cpp-dev
    ```
    or
    ```
    git clone https://github.com/jbeder/yaml-cpp.git
    mkdir build && cd build
    cmake .. -DCMALE_INSTALL_PREFIX=/usr/local
    make && make install
    ```
