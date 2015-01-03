project(simulation)

set(simulation_src
    simnode.cpp
    plugin.cpp
    )

include_directories(
            ${PROJECT_SOURCE_DIR}
            ${MINDTREE_SRC_LIB_DIR}
)

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/source/lib
            ${MINDTREE_CORE_LIB_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${MAIN_INCLUDE_DIR}
)

add_library(simulation SHARED ${simulation_src})
set_target_properties(simulation PROPERTIES PREFIX "")
target_link_libraries(simulation 
            mindtree_core
            ${MINDTREE_CORE_LIB}
)

install(TARGETS simulation LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/simulation)
