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
add_library(icosphere MODULE icosphere.cpp)
target_link_libraries(icosphere mindtree_core objectlib)
add_library(cylinder MODULE cylinder.cpp)
target_link_libraries(cylinder mindtree_core objectlib)
add_library(disc MODULE disc.cpp)
target_link_libraries(disc mindtree_core objectlib)

add_library(meshing MODULE meshing.cpp)
target_link_libraries(meshing mindtree_core objectlib)

add_library(lsystem MODULE lsystem.cpp)
target_link_libraries(lsystem mindtree_core objectlib)

add_library(scattersurface MODULE scattersurface.cpp)
target_link_libraries(scattersurface mindtree_core objectlib)

add_library(catmullclark MODULE catmullclark.cpp)
target_link_libraries(catmullclark mindtree_core objectlib)

add_library(copy MODULE copy.cpp)
target_link_libraries(copy mindtree_core objectlib)

install(TARGETS pointcloud LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS icosphere LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS cylinder LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS disc LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS lsystem LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS meshing LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS scattersurface LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS catmullclark LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
install(TARGETS copy LIBRARY DESTINATION ${PROJECT_ROOT}/processors)
