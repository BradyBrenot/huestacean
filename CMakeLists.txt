cmake_minimum_required(VERSION 3.10)
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
find_package(Qt5RemoteObjects REQUIRED)

set(MAIN_SOURCE 
   ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

SET(THIRD_PARTY_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hsluv-c/src/hsluv.c
)

SET(THIRD_PARTY_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/hsluv-c/src/hsluv.h

	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ChromaSDK/inc/RzChromaLinkAPI.h
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ChromaSDK/inc/RzChromaSDK.h
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ChromaSDK/inc/RzChromaSDKDefines.h
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ChromaSDK/inc/RzChromaSDKTypes.h
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ChromaSDK/inc/RzErrors.h
)

set(SOURCES
   ${SOURCES}
   ${THIRD_PARTY_SOURCE}

   ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/backend.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/src/common/math.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/common/scene.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/common/device.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/common/effect.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/src/effects/effects.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/src/hue/hue.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/hue/bridge.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/hue/bridgediscovery.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/hue/streamer.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/src/razer/chroma.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/razer/razer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/razer/razerdevices.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/gui/gui.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/frontend.cpp
)
set(HEADERS
   ${HEADERS}
   ${THIRD_PARTY_HEADERS}

   ${CMAKE_CURRENT_SOURCE_DIR}/include/common/device.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/common/effect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/common/lightupdate.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/common/providertype.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/common/scene.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/common/math.h

   ${CMAKE_CURRENT_SOURCE_DIR}/include/effects/effects.h

   ${CMAKE_CURRENT_SOURCE_DIR}/include/hue/hue.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/hue/bridge.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/hue/bridgediscovery.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/hue/streamer.h

   ${CMAKE_CURRENT_SOURCE_DIR}/include/razer/chroma.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/razer/razer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/razer/razerdevices.h

   ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/gui/gui.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/rep_frontend.h
   ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/frontend.h
)

#headers with Qt-relevant types (for moc)
set(QT_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/hue/bridgediscovery.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/hue/bridge.h

	${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/gui/gui.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/frontend.h
)

QT5_WRAP_CPP(HEADERS_MOC ${QT_HEADERS})

set(INCLUDE_PATHS
	${INCLUDE_PATHS}

	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mbedtls/include
	${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ChromaSDK/inc
)

include_directories(${INCLUDE_PATHS})

add_subdirectory(thirdparty/screen_capture_lite EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/mbedtls EXCLUDE_FROM_ALL)

set(LIBRARIES
   ${LIBRARIES}
   Qt5::Core
   Qt5::Network
   Qt5::Quick 
   Qt5::RemoteObjects
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

add_custom_command(
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/rep_frontend.rep
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend/rep_frontend.h
  COMMAND repc -o merged rep_frontend.rep rep_frontend.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/frontend
  COMMENT "compile with repc"
)

### Tests

enable_testing()

find_package(Qt5Test REQUIRED)
include_directories(
	${Qt5Test_INCLUDES}
)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

set(TEST_SOURCES
	${SOURCES}

	${CMAKE_CURRENT_SOURCE_DIR}/tests/testsmain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/tests/backendtests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/tests/mathtests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/tests/huetests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/tests/razertests.cpp
)

set(TEST_HEADERS
	${HEADERS}
	${HEADERS_MOC}
)

set(TEST_LIBRARIES
   ${LIBRARIES}
   Catch
   Qt5::Test
)

# Make test executable
add_executable(huestacean_tests ${TEST_SOURCES} ${TEST_HEADERS})
target_link_libraries(huestacean_tests ${TEST_LIBRARIES})

include(${CMAKE_CURRENT_SOURCE_DIR}/tests/catch/cmake/Catch.cmake)
catch_discover_tests(huestacean_tests)