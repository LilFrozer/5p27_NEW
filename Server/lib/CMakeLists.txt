cmake_minimum_required( VERSION 3.0 )
project( dspapi )
set( APP_NAME dspapi )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s" )

file( GLOB_RECURSE SOURCES *.h *.cpp )
set( SOURCES ${SOURCES} )

add_library( ${APP_NAME} SHARED ${SOURCES} )
target_compile_options( ${APP_NAME} PRIVATE -Wall -Wextra )
target_link_libraries( ${APP_NAME} -lsocket )
