project(mindtree_core)
cmake_minimum_required(VERSION 2.8)
set(LIB_SRC
    data/benchmark.cpp
    data/cache_main.cpp
    data/datatypes.cpp
    data/dnspace.cpp
    data/io.cpp
    data/nodes/data_node.cpp
    data/nodes/arraynode.cpp
    data/nodes/structnode.cpp
    data/nodes/containernode.cpp
    data/mtobject.cpp
    data/nodes/data_node_socket.cpp
    data/nodes/node_db.cpp
    data/project.cpp
    data/properties.cpp
    data/python/console.cpp
    data/python/pycache_main.cpp
    data/python/pyexposable.cpp
    data/python/wrapper.cpp
    data/python/pyutils.cpp
    data/python/system.cpp
    data/reloadable.cpp
    data/signal.cpp
    data/windowfactory.cpp
    data/raytracing/ray.cpp
    graphics/viewer.cpp
    graphics/viewer_dock_base.cpp
    graphics/windowlist.cpp
    graphics/pywindowlist.cpp
    graphics/shapes.cpp
)

set(LIB_HEAD
    data/python/console.h
    data/windowfactory.h
    graphics/viewer_dock_base.h
    graphics/windowlist.h
)

set(CMAKE_AUTOMOC ON)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(PythonLibs 3.4 REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(
            ${PROJECT_SOURCE_DIR}
            /usr/include
            ${PYTHON_INCLUDE_DIRS}
            ${OPENGL_ICNLUDE_DIRS}
)

message("core lib src directory: " ${PROJECT_SOURCE_DIR})
message("core lib bin directory: " ${MINDTREE_CORE_LIB_DIR})
message("boost python lib: " ${Boost_LIBRARIES})

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${Boost_LIBRARY_DIRS}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARY_DIRS}
)

add_library(mindtree_core SHARED ${LIB_SRC} ${LIB_MOC_SRC})
message("mindtree core lib:" mindtree_core)

target_link_libraries(mindtree_core 
  Qt5::Widgets
  Qt5::Core
  Qt5::OpenGL
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

install(TARGETS mindtree_core 
        LIBRARY DESTINATION ${MINDTREE_CORE_LIB_DIR}
        ARCHIVE DESTINATION ${MINDTREE_CORE_LIB_DIR}
)

