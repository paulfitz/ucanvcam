cmake_minimum_required(VERSION 2.6)

PROJECT(ucanvcam)

#########################################################################
# general compile options
#########################################################################

#### options do not do anything now; should come back eventually though.
SET (STANDALONE TRUE CACHE BOOL "standalone program or plugin?")
SET (USE_GUI TRUE CACHE BOOL "GUI or command-line?")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/conf ${CMAKE_MODULE_PATH})

#########################################################################
# most of the work is done here 
#########################################################################

ADD_SUBDIRECTORY(src)

#########################################################################
# Information for packaging 
#########################################################################

INSTALL(DIRECTORY media/ DESTINATION media
        PATTERN ".svn" EXCLUDE)

INCLUDE(${CMAKE_SOURCE_DIR}/conf/PackageInfo.cmake)

#########################################################################
# Building documentation
#########################################################################

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/doc/doxygen.conf.in 
               ${CMAKE_BINARY_DIR}/src/doc/doxygen.conf
               IMMEDIATE)
ADD_CUSTOM_TARGET(dox COMMAND doxygen ${CMAKE_BINARY_DIR}/src/doc/doxygen.conf)

