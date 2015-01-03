project(objectlib)
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

set(object_source_files
    object.cpp
    lights.cpp
    material.cpp
)

add_library(objectlib SHARED ${object_source_files})

target_link_libraries(objectlib
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
)

project(object)
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

add_library(object SHARED python.cpp)
set_target_properties(object PROPERTIES PREFIX "")

target_link_libraries(object
                    mindtree_core
                    objectlib
                    widgets3d
                    ${MINDTREE_CORE_LIB}
                    ${QT_LIBRARIES}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
                    ${OPENGL_LIBRARIES}
)


install(TARGETS object LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/datatypes/Object)
install(TARGETS objectlib LIBRARY DESTINATION ${PROJECT_ROOT}/lib)
