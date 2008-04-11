PROJECT(ucanvcam)

#########################################################################
# general compile options
#########################################################################

#### options do not do anything now; should come back eventually though.
SET (STANDALONE TRUE CACHE BOOL "standalone program or plugin?")
SET (USE_GUI TRUE CACHE BOOL "GUI or command-line?")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#########################################################################
# most of the work is done here 
#########################################################################

ADD_SUBDIRECTORY(src)

#########################################################################
# Information for packaging 
#########################################################################

SET(CPACK_PACKAGE_NAME "ucanvcam")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "2")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_SOURCE_IGNORE_FILES} "~$" ".svn$" ".svn/.*$")

INCLUDE(CPack)

ADD_CUSTOM_TARGET(dox COMMAND doxygen ${PROJECT_SOURCE_DIR}/doc/src/doxygen.conf)

