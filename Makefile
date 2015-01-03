all: create_cmake_project
	cmake --build build/debug --target install -- -j4

create_cmake_project:
	test -d build/debug || mkdir -p build/debug; cd build/debug; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ../..
