project(mathnodes)
set(math_src mathnodes.cpp)
set(math_head mathnodes.h)
    
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

add_library(mathnodes SHARED ${math_src} ${math_moc_src})

set_target_properties(mathnodes PROPERTIES PREFIX "")
target_link_libraries(mathnodes 
            mindtree_core
            ${MINDTREE_CORE_LIB}
            ${Boost_LIBRARIES}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARIES}
)

set(math_lib_dir ${PROJECT_ROOT}/plugins/mathnodes)
install(TARGETS mathnodes LIBRARY DESTINATION ${math_lib_dir})
