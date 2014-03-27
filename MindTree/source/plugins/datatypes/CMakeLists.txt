add_subdirectory(Int)
add_subdirectory(Float)
#add_subdirectory(Color)
add_subdirectory(Vector)
add_subdirectory(Object)
add_subdirectory(Polygon)
add_subdirectory(String)
#add_subdirectory(Boolean)

project(generic_processor)

include_directories(
            ${PROJECT_SOURCE_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${Boost_INCLUDE_DIRS}
            ${PYTHON_INCLUDE_DIRS}
)

link_directories(
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/source/lib
            ${MINDTREE_CORE_LIB_DIR}
            ${MINDTREE_SRC_LIB_DIR}
            ${MAIN_INCLUDE_DIR}
            ${Boost_LIBRARY_DIRS}
            ${PYTHON_LIBRARIES}
)

add_library(generic_processor SHARED generic_processor.cpp)
set_target_properties(generic_processor PROPERTIES PREFIX "")

target_link_libraries(generic_processor
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
)


install(TARGETS generic_processor LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/datatypes/)
