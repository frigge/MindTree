project(scenegraph)
set(SCE_SRC
    pluginentry.cpp
    graphics/viewport.cpp
    graphics/viewport_widget.cpp
)

set(SCE_HEADER
    graphics/viewport.h
    graphics/viewport_widget.h
)

set(CMAKE_AUTOMOC ON)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5OpenGL REQUIRED)

message("main include directory: " ${MAIN_INCLUDE_DIR})
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

add_library(scenegraph SHARED ${SCE_SRC} ${SCE_MOC_SRC})
set_target_properties(scenegraph PROPERTIES PREFIX "")
target_link_libraries(scenegraph 
            objectlib
            widgets3d
            render
            mindtree_core
            ${MINDTREE_CORE_LIB}
            Qt5::Widgets
            Qt5::Core
            Qt5::OpenGL
)

message("Root dir: " ${PROJECT_ROOT})
set(SCENEGRAPH_LIB_DIR ${PROJECT_ROOT}/plugins/scenegraph)
install(TARGETS scenegraph LIBRARY DESTINATION ${SCENEGRAPH_LIB_DIR})
