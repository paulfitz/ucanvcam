
PROJECT(UcanvcamModules)

FIND_PACKAGE(YARP REQUIRED)

BEGIN_DEVICE_LIBRARY(vcammod)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/modules/wxsdl
                     ${CMAKE_CURRENT_BINARY_DIR})
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/modules/vdub
                     ${CMAKE_CURRENT_BINARY_DIR})
    ADD_SUBDIRECTORY(${YARP_DIR}/src/libYARP_dev/src/ffmpeg
                     ${CMAKE_CURRENT_BINARY_DIR})
END_DEVICE_LIBRARY(vcammod)

