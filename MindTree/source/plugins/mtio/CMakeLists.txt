project(objio)
include_directories(
            ${PROJECT_SOURCE_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${Boost_INCLUDE_DIRS}
            ${PYTHON_INCLUDE_DIRS}
            ${OPENGL_ICNLUDE_DIRS}
)

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/source/lib
            ${MINDTREE_CORE_LIB_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${MAIN_INCLUDE_DIR}
            ${Boost_LIBRARY_DIRS}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARY_DIRS}
)

add_library(objio SHARED obj.cpp pymodule.cpp)
set_target_properties(objio PROPERTIES PREFIX "")

target_link_libraries(objio
                    objectlib
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
                    ${QT_LIBRARIES}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
                    ${OPENGL_LIBRARIES}
)

install(TARGETS objio LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/mtio)

project(textio)
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

add_library(textio SHARED textio.cpp)
set_target_properties(textio PROPERTIES PREFIX "")

target_link_libraries(textio
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
)

install(TARGETS textio LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/mtio)
