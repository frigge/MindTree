project(utilities)

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

set(UTILITIES_SRC
    utilities.cpp
    )

add_library(utilities SHARED ${UTILITIES_SRC})
set_target_properties(utilities PROPERTIES PREFIX "")

target_link_libraries(utilities
                    mindtree_core
                    objectlib
                    ${MINDTREE_CORE_LIB}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
)


install(TARGETS utilities LIBRARY DESTINATION ${PROJECT_ROOT}/plugins/utilities/)
