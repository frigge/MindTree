cmake_minimum_required(VERSION 2.8)
set(LIB_SRC
    data/cache_main.cpp
    data/data_nodelink.cpp
    data/datatypes.cpp
    data/dnspace.cpp
    data/frg.cpp
    #data/nodes/buildin_nodes.cpp
    data/nodes/data_node.cpp
    data/mtobject.cpp
    data/nodes/data_node_socket.cpp
    data/nodes/node_db.cpp
    data/nodes/nodetype.cpp
    data/nodes/socket_properties.cpp
    data/project.cpp
    data/properties.cpp
    data/python/console.cpp
    data/python/pycache_main.cpp
    data/python/pyexposable.cpp
    data/python/wrapper.cpp
    data/signal.cpp
    data/windowfactory.cpp
    graphics/prop_vis.cpp
    graphics/viewer.cpp
    graphics/viewer_dock_base.cpp
    graphics/windowlist.cpp
    graphics/pywindowlist.cpp
)

set(LIB_HEAD
    data/cache_main.h
    data/data_nodelink.h
    data/datatypes.h
    data/dnspace.h
    data/frg.h
    #data/nodes/buildin_nodes.h
    data/nodes/data_node.h
    data/mtobject.h
    data/nodes/data_node_socket.h
    data/nodes/node_db.h
    data/nodes/nodetype.h
    data/nodes/socket_properties.h
    data/project.h
    data/properties.h
    data/python/console.h
    data/python/pycache_main.h
    data/python/pyexposable.h
    data/python/wrapper.h
    data/signal.h
    data/windowfactory.h
    graphics/prop_vis.h
    graphics/viewer.h
    graphics/viewer_dock_base.h
    graphics/windowlist.h
    graphics/pywindowlist.h
)

set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTSVG TRUE)

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(OpenGL REQUIRED)
QT4_WRAP_CPP(LIB_MOC_SRC ${LIB_HEAD} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
include(${QT_USE_FILE})
include_directories(
            ${PROJECT_SOURCE_DIR}
            ${Boost_INCLUDE_DIRS}
            ${PYTHON_INCLUDE_DIRS}
            ${OPENGL_ICNLUDE_DIRS}
)

message("core lib src directory: "${PROJECT_SOURCE_DIR})
message("core lib bin directory: "${MINDTREE_CORE_LIB_DIR})
link_directories(
            ${PROJECT_SOURCE_DIR}
            ${Boost_LIBRARY_DIRS}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARY_DIRS}
)

add_library(mindtree_core SHARED ${LIB_SRC} ${LIB_MOC_SRC})
message("mindtree core lib:" mindtree_core)

target_link_libraries(mindtree_core 
            ${QT_LIBRARIES}
            ${Boost_LIBRARIES}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARIES}
)

install(TARGETS mindtree_core 
        LIBRARY DESTINATION ${MINDTREE_CORE_LIB_DIR}
        ARCHIVE DESTINATION ${MINDTREE_CORE_LIB_DIR}
)

