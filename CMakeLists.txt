cmake_minimum_required(VERSION 3.9)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

project(Huestacean)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Quick REQUIRED)

set(SOURCES
   ${SOURCES}
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
set(HEADERS
   ${HEADERS}
)

QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mbedtls/include ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(thirdparty/screen_capture_lite)
add_subdirectory(thirdparty/mbedtls)

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
elseif(LINUX)
	set(LIBRARIES
      ${LIBRARIES}
      pthread
	  X11
	  Xext
	  Xinerama
	  Xfixes
   )
endif()

add_executable(huestacean WIN32 ${SOURCES} ${HEADERS} ${HEADERS_MOC} resources/qml.qrc)
target_link_libraries(huestacean ${LIBRARIES})
