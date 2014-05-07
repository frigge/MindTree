project(mttimeline)

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

add_library(mttimeline SHARED timeline.cpp)
set_target_properties(mttimeline PROPERTIES PREFIX "")

target_link_libraries(mttimeline
                    mindtree_core
                    ${MINDTREE_CORE_LIB}
                    ${Boost_LIBRARIES}
                    ${PYTHON_LIBRARIES}
)


install(TARGETS mttimeline LIBRARY DESTINATION ${PROJECT_ROOT}/intern/timelineWidget/)
