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

add_library(pointcloud MODULE pointcloud.cpp)
add_library(primitives MODULE primitives.cpp)

install(TARGETS pointcloud LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS primitives LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
