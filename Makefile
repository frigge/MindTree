all: release

debug: clang_debug

clang_debug: create_clang_debug_project
	cmake --build build/debug_clang --target install -- -j4

gcc_debug: create_gcc_debug_project
	cmake --build build/debug_gcc --target install -- -j4

profile: clang_profile

clang_profile: create_clang_profile_project
	cmake --build build/profile_clang --target install -- -j4

release: clang_release

clang_release: create_clang_release_project
	cmake --build build/release_clang --target install -- -j4

gcc_release: create_gcc_release_project
	cmake --build build/release_gcc --target install -- -j4

create_gcc_release_project:
	test -d build/release_gcc || mkdir -p build/release_gcc; cd build/release_gcc; cmake -DCMAKE_BUILD_TYPE=Release ../..

create_clang_release_project:
	test -d build/release_clang || mkdir -p build/release_clang; cd build/release_clang; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release ../..

debug_address_sanitizer: create_cmake_address_sanitizer_project
	cmake --build build/debug_address_sanitizer --target install -- -j4

debug_thread_sanitizer: create_cmake_thread_sanitizer_project
	cmake --build build/debug_thread_sanitizer --target install -- -j4

create_clang_debug_project:
	test -d build/debug_clang || mkdir -p build/debug_clang; cd build/debug_clang; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ../..

create_clang_profile_project:
	test -d build/profile_clang || mkdir -p build/profile_clang; cd build/profile_clang; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug -DPROFILE=ON ../..

create_gcc_debug_project:
	test -d build/debug_gcc || mkdir -p build/debug_gcc; cd build/debug_gcc; cmake -DCMAKE_BUILD_TYPE=Debug ../..

create_cmake_address_sanitizer_project:
	test -d build/debug_address_sanitizer || mkdir -p build/debug_address_sanitizer; cd build/debug_address_sanitizer; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug -DCLANG_SANITIZE_ADDRESS=ON ../..

create_cmake_thread_sanitizer_project:
	test -d build/debug_thread_sanitizer || mkdir -p build/debug_thread_sanitizer; cd build/debug_thread_sanitizer; C=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release -DCLANG_SANITIZE_THREAD=ON ../..
