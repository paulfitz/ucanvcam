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

INSTALL(DIRECTORY media/ DESTINATION media
        PATTERN ".svn" EXCLUDE)

#########################################################################
# Information for packaging 
#########################################################################

SET(CPACK_PACKAGE_NAME "ucanvcam")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "4")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_VENDOR "MakeSweet")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/GPL.TXT")
IF (WIN32)
  SET(CPACK_GENERATOR "NSIS")
ELSEIF (WIN32)
  SET(CPACK_GENERATOR "TGZ")
ENDIF (WIN32)
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_SOURCE_IGNORE_FILES} "~$" ".svn$" ".svn/.*$")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "ucanvcam")
SET(CPACK_STRIP_FILES TRUE)
IF (WIN32)
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "RegDLL \\\$INSTDIR\\\\bin\\\\libucanvcamx.dll")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "UnRegDLL \\\$INSTDIR\\\\bin\\\\libucanvcamx.dll")
ENDIF (WIN32)
SET(CPACK_PACKAGE_EXECUTABLES "ucanvcam" "UCanVCam")
SET(CPACK_CREATE_DESKTOP_LINKS "ucanvcam")
SET(CPACK_NSIS_DISPLAY_NAME "UCanVCam - Your Free Virtual Camera Toolkit")
SET(CPACK_NSIS_URL_INFO_ABOUT "http://code.google.com/p/ucanvcam/")
SET(CPACK_NSIS_MUI_ICON "@PROJECT_SOURCE_DIR@/media/ucanvcam.ico")
#SET(CPACK_NSIS_MUI_UNIICON "@PROJECT_SOURCE_DIR@/media/ucanvcam.ico")
SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin/ucanvcam.exe")
INCLUDE(CPack)

#########################################################################
# Building documentation
#########################################################################

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/doc/doxygen.conf.in 
               ${CMAKE_BINARY_DIR}/src/doc/doxygen.conf
               IMMEDIATE)
ADD_CUSTOM_TARGET(dox COMMAND doxygen ${CMAKE_BINARY_DIR}/src/doc/doxygen.conf)

