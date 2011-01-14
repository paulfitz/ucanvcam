
PROJECT(UcanvcamModules)

FIND_PACKAGE(YARP REQUIRED)

INCLUDE(${YARP_MODULE_PATH}/YarpPlugin.cmake)

BEGIN_PLUGIN_LIBRARY(vcammod)
    #ADD_SUBDIRECTORY(wxsdl)
    ADD_SUBDIRECTORY(vdub)
    IF (UNIX)
      # make sure we have the ffmpeg device
      ADD_SUBDIRECTORY(${YARP_MODULE_PATH}/../src/modules/ffmpeg 
	${CMAKE_BINARY_DIR}/ffmpeg)
    ENDIF (UNIX)
END_PLUGIN_LIBRARY(vcammod)

