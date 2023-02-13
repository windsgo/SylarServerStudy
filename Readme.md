# sylar network server study code

## Introduction

A Repo with sylar network server study codes, including self-optimized codes.

## Versions

- ```KUbuntu 20.04```
- ```gcc 9.4.0 x86_64-linux-gnu```
- ```cmake 3.16.3```

## External Dependencies

- libbacktrace(if enabled "```Debug```" or "```RelWithDebInfo```")

    a linux built-in library
    
    It is used and linked by ```libsylar``` while 
    - "```Debug```" or "```RelWithDebInfo```" is configured
    - ```USE_BOOST_BACKTRACE=ON```

    If not compiled with debug info, we use c api ```::backtrace``` and ```::backtrace_symbol``` to get backtrace info.

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
