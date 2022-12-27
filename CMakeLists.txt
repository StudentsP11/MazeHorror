cmake_minimum_required(VERSION 3.14.0)

set(PROJECT_NAME MazeHorror)
project(${PROJECT_NAME} VERSION 0.1.0)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

aux_source_directory(./src SRC_LIST)

# include(CTest)
# enable_testing()

# MazeHorror main project
add_executable(${PROJECT_NAME})
# src/Direction.cpp src/Maze.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Add sources
add_subdirectory(src)
# Add libraries
add_subdirectory(externals)

# Find dependencies.
find_package(OpenGL COMPONENTS OpenGL REQUIRED)

# Include and link against dependencies.
target_link_libraries(${PROJECT_NAME} PUBLIC GLAD)
target_include_directories(${PROJECT_NAME} PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/externals/GLAD"
                          )
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL SDL2 SDL2_mixer)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# include(CPack)
