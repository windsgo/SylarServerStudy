all:configure build

configure:
	cmake -B build -DCMAKE_BUILD_TYPE=DEBUG

build: configure
	cmake --build build -j4

clean:
	rm -rf build

.PHONY:all configure build clean
