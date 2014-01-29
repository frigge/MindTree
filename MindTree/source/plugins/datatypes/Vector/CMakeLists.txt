project(vectorcache)
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

add_library(vectorcache SHARED vector_cache.cpp)

target_link_libraries(vectorcache
                    intcache
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
                    ${QT_LIBRARIES}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
                    ${OPENGL_LIBRARIES}
)

install(TARGETS vectorcache LIBRARY DESTINATION ${PROJECT_ROOT}/lib)
