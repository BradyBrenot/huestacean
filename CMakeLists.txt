cmake_minimum_required(VERSION 3.9)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

project(huestacean)

add_subdirectory(huestaceand)
add_subdirectory(huestacean-gui)

add_dependencies(huestacean-gui huestaceand)