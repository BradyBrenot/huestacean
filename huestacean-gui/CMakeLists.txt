cmake_minimum_required(VERSION 3.9)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

project(huestacean-gui)

find_package(Qt5Core REQUIRED)
find_package(Qt5Quick REQUIRED)

set(SOURCES
   ${SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/objectmodel.cpp
)
set(HEADERS
   ${HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/include/objectmodel.h
)

QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/libhuestacean/include ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(libhuestacean)

set(LIBRARIES
   ${LIBRARIES}
   Qt5::Core
   Qt5::Quick 
)

add_executable(huestacean-gui WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${HEADERS_MOC} qml.qrc)
target_link_libraries(huestacean-gui ${LIBRARIES})
