all:
	cmake -B build -DCMAKE_BUILD_TYPE=DEBUG
	cmake --build build -j4

clean:
	rm -rf build
