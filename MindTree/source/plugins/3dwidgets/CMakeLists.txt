project(widgets3d)

set(w3d_src
    widgets.cpp
    translate_widgets.cpp
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

add_library(widgets3d SHARED ${w3d_src})
set_target_properties(widgets3d PROPERTIES PREFIX "")
target_link_libraries(widgets3d 
            render
            mindtree_core
            ${MINDTREE_CORE_LIB}
)

install(TARGETS widgets3d LIBRARY DESTINATION ${PROJECT_ROOT}/lib)
