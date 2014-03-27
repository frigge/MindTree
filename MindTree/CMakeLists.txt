cmake_minimum_required(VERSION 2.8)
project(mindTree)
SET(HEADER
            source/data/data_info_box.h
#            source/data/code_generator/shaderwriter.h
#            source/data/code_generator/rslwriter.h
#            source/data/code_generator/glslwriter.h
#            source/data/code_generator/inputs.h
#            source/data/code_generator/outputs.h
            source/data/base/init.h
            source/data/gl/init.h
            source/data/python/init.h
#            source/graphics/newnodeeditor.h
#            source/graphics/nodelib.h
#            source/graphics/scenewidgetcontainer.h
#            source/graphics/shaderpreview.h
#source/graphics/previewdock.h
#            source/graphics/sourcedock.h
#            source/graphics/properties_editor.h
            source/graphics/python/consolewindow.h
            source/data/base/raytracing/ray.h
            source/data/undo/frg_generic_undo.h
            #source/graphics/gui.h
#source/graphics/nodes/graphics_node.h
#            source/graphics/nodes/graphics_node_socket.h
            source/graphics/base/mindtree_mainwindow.h
            source/graphics/base/consoledialog.h
)

SET(SOURCE
            source/data/data_info_box.cpp
#            source/data/code_generator/shaderwriter.cpp
#            source/data/code_generator/rslwriter.cpp
#            source/data/code_generator/glslwriter.cpp
#            source/data/code_generator/inputs.cpp
#            source/data/code_generator/outputs.cpp
            source/data/base/init.cpp
            source/data/gl/init.cpp
            source/data/python/init.cpp
            #source/graphics/gui.cpp
#            source/graphics/newnodeeditor.cpp
#            source/graphics/nodelib.cpp
#            source/graphics/scenewidgetcontainer.cpp
#            source/graphics/shaderpreview.cpp
#            source/graphics/previewdock.cpp
#            source/graphics/sourcedock.cpp
#            source/graphics/properties_editor.cpp
            source/graphics/python/consolewindow.cpp
            source/data/base/main.cpp
            source/graphics/base/consoledialog.cpp
            source/data/base/raytracing/ray.cpp
            source/data/undo/frg_generic_undo.cpp
            source/graphics/base/mindtree_mainwindow.cpp
)

set(RESOURCES source/graphics/styles/styles.qrc)

#enable C++11 features
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
if(GCC_VERSION VERSION_LESS 4.7)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x")
else()
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")
endif()
    
set(QT_USE_QTOPENGL TRUE)
set(QT_USE_QTSVG TRUE)

find_package(Qt4 4.8 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

QT4_WRAP_CPP(MOC_SRC ${HEADER} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
QT4_ADD_RESOURCES(RES_RCC ${RESOURCES})

include(${QT_USE_FILE})
set(MINDTREE_CORE_LIB_DIR ${PROJECT_BINARY_DIR}/lib)
set(MINDTREE_SRC_LIB_DIR ${PROJECT_SOURCE_DIR}/source/lib)
include_directories(
            ${MINDTREE_SRC_LIB_DIR}
            ${PROJECT_SOURCE_DIR}
            ${Boost_INCLUDE_DIRS}
            ${PYTHON_INCLUDE_DIRS}
            ${OPENGL_ICNLUDE_DIRS}
            ${GLEW_ICNLUDE_DIRS}
)

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${MINDTREE_CORE_LIB_DIR}
            ${PROJECT_BINARY_DIR}/source/lib
            ${MINDTREE_SRC_LIB_DIR}
            ${Boost_LIBRARY_DIRS}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARY_DIRS}
            ${GLEW_LIBRARY_DIRS}
)

set(ALL_SRC ${SOURCE} ${MOC_SRC} ${RES_RCC})
set(MINDTREE_CORE_LIB ${PROJECT_BINARY_DIR}/source/lib/libmindtree_core.so)

add_subdirectory(source/lib)

add_executable(mindTree ${ALL_SRC})
target_link_libraries(mindTree 
            mindtree_core
            ${MINDTREE_CORE_LIB}
            ${QT_LIBRARIES}
            ${Boost_LIBRARIES}
            ${PYTHON_LIBRARIES}
            ${OPENGL_LIBRARIES}
            ${GLEW_LIBRARIES}
)

set(PROJECT_ROOT ${PROJECT_BINARY_DIR})
add_subdirectory(source/plugins)


message("Binary Dir:" ${PROJECT_BINARY_DIR})
message("mindtree core lib:" ${MINDTREE_CORE_LIB})
message("Python Libs: "${PYTHON_LIBRARIES})
install(TARGETS mindTree RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)
install(DIRECTORY source/plugins DESTINATION ${PROJECT_BINARY_DIR} PATTERN ".h .cpp" EXCLUDE)
install(DIRECTORY source/intern DESTINATION ${PROJECT_BINARY_DIR} PATTERN ".h .cpp" EXCLUDE)
install(DIRECTORY source/settings DESTINATION ${PROJECT_BINARY_DIR} PATTERN ".h .cpp" EXCLUDE)

