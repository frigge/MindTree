project(objio)
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

add_library(objio SHARED obj.cpp pymodule.cpp)
set_target_properties(objio PROPERTIES PREFIX "")

target_link_libraries(objio
                    objectlib
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
)

install(TARGETS objio LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/mtio)

project(textio)
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

add_library(textio SHARED textio.cpp)
set_target_properties(textio PROPERTIES PREFIX "")

target_link_libraries(textio
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
)

install(TARGETS textio LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/mtio)
