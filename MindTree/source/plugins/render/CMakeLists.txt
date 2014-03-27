project(render)
set(RENDER_SRC
    render.cpp
    glwrapper.cpp
)

set(RENDER_HEADER
    render.h
    glwrapper.h
)

find_package(Boost COMPONENTS python REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(OpenGL REQUIRED)

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

add_library(render SHARED ${RENDER_SRC})

target_link_libraries(render 
            objectlib
            mindtree_core
            ${MINDTREE_CORE_LIB}
            ${Boost_LIBRARIES}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARIES}
)

install(TARGETS render LIBRARY DESTINATION ${PROJECT_ROOT}/lib)
