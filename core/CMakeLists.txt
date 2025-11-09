set(INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(HEADERS
    ${INCLUDE_PATH}/cli.hpp
    ${INCLUDE_PATH}/command.hpp
    ${INCLUDE_PATH}/executor.hpp
    ${INCLUDE_PATH}/input.hpp
    ${INCLUDE_PATH}/dummy_input.hpp
    ${INCLUDE_PATH}/dummy_output.hpp
    ${INCLUDE_PATH}/output.hpp
    ${INCLUDE_PATH}/parser.hpp
    ${INCLUDE_PATH}/global_state.hpp
)

set(SOURCES
    ${SRC_PATH}/cli.cpp
    ${SRC_PATH}/parser.cpp
    ${SRC_PATH}/executor.cpp
)

add_library(
    ${PROJECT_NAME}_objs OBJECT ${SOURCES} ${HEADERS}
)
target_include_directories(
    ${PROJECT_NAME}_objs PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(
    ${PROJECT_NAME}_objs fmt
)
