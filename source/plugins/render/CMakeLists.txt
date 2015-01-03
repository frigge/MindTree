project(render)
set(RENDER_SRC
    camera_renderer.cpp
    empty_renderer.cpp
    geoobject_renderer.cpp
    glwrapper.cpp
    light_renderer.cpp
    polygon_renderer.cpp
    primitive_renderer.cpp
    render.cpp
    render_setup.cpp
    rendermanager.cpp
    renderpass.cpp
    shader_render_node.cpp
)

find_package(OpenGL REQUIRED)

include_directories(
            ${PROJECT_SOURCE_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${OPENGL_ICNLUDE_DIRS}
)

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/source/lib
            ${MINDTREE_CORE_LIB_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${MAIN_INCLUDE_DIR}
            ${OPENGL_LIBRARY_DIRS}
)

add_library(render SHARED ${RENDER_SRC})

target_link_libraries(render 
            objectlib
            mindtree_core
            ${OPENGL_LIBRARIES}
)

install(TARGETS render LIBRARY DESTINATION ${PROJECT_ROOT}/lib)
