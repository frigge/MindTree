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
target_link_libraries(pointcloud mindtree_core objectlib)
add_library(primitives MODULE primitives.cpp)
target_link_libraries(primitives mindtree_core objectlib)
add_library(lsystem MODULE lsystem.cpp)
target_link_libraries(lsystem mindtree_core objectlib)

install(TARGETS pointcloud LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS primitives LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS lsystem LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
