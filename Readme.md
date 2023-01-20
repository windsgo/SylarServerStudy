# sylar network server study code

## Introduction

A Repo with sylar network server study codes, including self-optimized codes.

## Versions

- ```KUbuntu 20.04```
- ```gcc 9.4.0 x86_64-linux-gnu```
- ```cmake 3.16.3```

## External Dependencies

- boost

    ```bash
    sudo apt-get install libboost-all-dev
    ```

- yaml-cpp (pre-compiled in 3rd_party)

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