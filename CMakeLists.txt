cmake_minimum_required(VERSION 3.9)
project(Huestacean)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Quick REQUIRED)

set(MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

set(SOURCES
   ${SOURCES}
)
set(HEADERS
   ${HEADERS}
)

QT5_WRAP_CPP(HEADERS_MOC ${HEADERS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mbedtls/include ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(thirdparty/screen_capture_lite EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/mbedtls EXCLUDE_FROM_ALL)

set(LIBRARIES
   ${LIBRARIES}
   Qt5::Core
   Qt5::Network
   Qt5::Quick 
   screen_capture_lite
   mbedtls
   mbedx509
   mbedcrypto
)

#screen_capture_lite additional requirements
if(WIN32)
   set(LIBRARIES
      ${LIBRARIES}
      Dwmapi
	  wsock32
	  ws2_32
	  legacy_stdio_definitions
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

add_executable(huestacean WIN32 ${MAIN_SOURCE} ${SOURCES} ${HEADERS} ${HEADERS_MOC} resources/qml.qrc)
target_link_libraries(huestacean ${LIBRARIES})
