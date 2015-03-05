all: create_cmake_debug_project
	cmake --build build/debug --target install -- -j4

debug_address_sanitizer: create_cmake_address_sanitizer_project
	cmake --build build/debug_address_sanitizer --target install -- -j4

debug_thread_sanitizer: create_cmake_address_sanitizer_project
	cmake --build build/debug_thread_sanitizer --target install -- -j4

create_cmake_debug_project:
	test -d build/debug || mkdir -p build/debug; cd build/debug; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ../..

create_cmake_address_sanitizer_project:
	test -d build/debug_address_sanitizer || mkdir -p build/debug_address_sanitizer; cd build/debug_address_sanitizer; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug -DCLANG_SANITIZE_ADDRESS=ON ../..

create_cmake_thread_sanitizer_project:
	test -d build/debug_thread_sanitizer || mkdir -p build/debug_thread_sanitizer; cd build/debug_thread_sanitizer; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug -DCLANG_SANITIZE_THREAD=ON ../..
