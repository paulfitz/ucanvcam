
PROJECT(UcanvcamModules)

FIND_PACKAGE(YARP REQUIRED)

BEGIN_DEVICE_LIBRARY(vcammod)
    #ADD_SUBDIRECTORY(wxsdl)
    ADD_SUBDIRECTORY(vdub)
    IF (UNIX)
      # make sure we have the ffmpeg device
      ADD_SUBDIRECTORY(${YARP_DIR}/src/modules/ffmpeg 
	${CMAKE_BINARY_DIR}/ffmpeg)
    ENDIF (UNIX)
END_DEVICE_LIBRARY(vcammod)

