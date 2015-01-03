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

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)

QT4_WRAP_CPP(SCE_MOC_SRC ${SCE_HEADER} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
include(${QT_USE_FILE})
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
            ${QT_LIBRARIES}
)

message("Root dir: " ${PROJECT_ROOT})
set(SCENEGRAPH_LIB_DIR ${PROJECT_ROOT}/plugins/scenegraph)
install(TARGETS scenegraph LIBRARY DESTINATION ${SCENEGRAPH_LIB_DIR})
