project(basics)

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

set(basics_src
    basics.cpp
    create_list_node.cpp
    )

add_library(basics SHARED ${basics_src})
set_target_properties(basics PROPERTIES PREFIX "")

target_link_libraries(basics
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
)


install(TARGETS basics LIBRARY DESTINATION ${PROJECT_ROOT}/intern/basics/)
