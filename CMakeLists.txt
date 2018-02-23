cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

project(Huestacean)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Quick REQUIRED)

set(SOURCES
   ${SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/src/bridgediscovery.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/entertainment.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/huebridge.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/huestacean.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/objectmodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/utility.cpp
)
set(HEADERS
   ${HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/include/bridgediscovery.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/entertainment.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/huebridge.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/huestacean.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/objectmodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/utility.h
)

QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(screen_capture_lite)
add_subdirectory(mbedtls)

add_executable(huestacean WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${HEADERS_MOC} qml.qrc)

set(LIBRARIES
   ${LIBRARIES}
   Qt5::Core
   Qt5::Network
   Qt5::Quick 
   screen_capture_lite
   mbedtls
)

#screen_capture_lite additional requirements
if(WIN32)
   set(LIBRARIES
      ${LIBRARIES}
      Dwmapi
   )
elseif(APPLE)
   find_library(corefoundation_lib CoreFoundation)
   find_library(cocoa_lib Cocoa)
   set(LIBRARIES
      ${LIBRARIES}
      ${corefoundation_lib}
      ${cocoa_lib}
   )
endif()

target_link_libraries(huestacean ${LIBRARIES})