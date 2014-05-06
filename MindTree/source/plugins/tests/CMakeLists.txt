project(cpp_tests)

set(cpp_tests_src
    cpp_tests.cpp)

include_directories(
            ${PROJECT_SOURCE_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${Boost_INCLUDE_DIRS}
            ${PYTHON_INCLUDE_DIRS}
)

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/source/lib
            ${MINDTREE_CORE_LIB_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${MAIN_INCLUDE_DIR}
            ${Boost_LIBRARY_DIRS}
            ${PYTHON_LIBRARIES}
)

add_library(cpp_tests SHARED ${cpp_tests_src})
set_target_properties(cpp_tests PROPERTIES PREFIX "")
target_link_libraries(cpp_tests
                    mindtree_core
                    objectlib
                    ${MINDTREE_CORE_LIB}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
)

install(TARGETS cpp_tests LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/tests/)
