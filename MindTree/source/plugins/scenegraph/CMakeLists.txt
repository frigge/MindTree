project(scenegraph)
set(SCE_SRC
    pluginentry.cpp
    graphics/viewport.cpp
    graphics/viewport_widget.cpp
)

set(SCE_HEADER
    pluginentry.h
    graphics/viewport.h
    graphics/viewport_widget.h
)

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(OpenGL REQUIRED)

QT4_WRAP_CPP(SCE_MOC_SRC ${SCE_HEADER} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
include(${QT_USE_FILE})
message("main include directory: " ${MAIN_INCLUDE_DIR})
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

add_library(scenegraph SHARED ${SCE_SRC} ${SCE_MOC_SRC})
set_target_properties(scenegraph PROPERTIES PREFIX "")
target_link_libraries(scenegraph 
            objectlib
            widgets3d
            render
            mindtree_core
            ${MINDTREE_CORE_LIB}
            ${QT_LIBRARIES}
            ${Boost_LIBRARIES}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARIES}
)

message("Root dir: " ${PROJECT_ROOT})
set(SCENEGRAPH_LIB_DIR ${PROJECT_ROOT}/plugins/scenegraph)
install(TARGETS scenegraph LIBRARY DESTINATION ${SCENEGRAPH_LIB_DIR})
