cmake_minimum_required( VERSION 3.16.3 )
project( RaspberryMusicPlayer )

set( CMAKE_CXX_STANDARD 17 )

# jsoncpp
include_directories( lib/dist/json )
set( JSON_FILES lib/dist/json/json.h lib/dist/json/json-forwards.h lib/dist/jsoncpp.cpp )

# SFML
if( WIN32 )
    set( SFML_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/lib/SFML-2.5.1/include/SFML-2.5.1/include )
    set( SFML_LIBRARY_DIR ${PROJECT_SOURCE_DIR}/lib/SFML-2.5.1/lib )
    set( SFML_DIR ${PROJECT_SOURCE_DIR}/lib/SFML-2.5.1/lib/cmake/SFML )
else()
    set( SFML_INCLUDE_DIR "usr/include" )
    set( SFML_LIBRARY_DIR "usr/lib/x86_64-linux-gnu" )
    set( SFML_DIR "usr/lib/x86_64-linux-gnu/cmake/SFML" )
endif()

link_directories( ${SFML_LIBRARY_DIR} )
include_directories( ${SFML_LIBRARY_DIR} )

find_package( SFML 2.5.1 REQUIRED COMPONENTS audio )
set( SFML_LIBRARIES sfml-audio )

if( SFML_FOUND )
    message( STATUS "SFML_INCLUDE_DIR: ${SFML_INCLUDE_DIR}" )
    message( STATUS "SFML_LIBRARIES: ${SFML_LIBRARIES}" )
    message( STATUS "SFML_VERSION: ${SFML_VERSION}" )
endif()

add_executable( RaspberryMusicPlayer main.cpp MusicPlayer.h MusicPlayer.cpp ${JSON_FILES} )
target_link_libraries( RaspberryMusicPlayer sfml-audio )