IF (COMPILE_DEVICE_LIBRARY)
  PREPARE_DEVICE(vdub TYPE WinCap INCLUDE WinCap.h WRAPPER grabber)
ENDIF (COMPILE_DEVICE_LIBRARY)

IF (NOT SKIP_vdub)

  # no real standard for where directx might be installed
  FIND_PATH(DIRECTX_INCLUDE_DIR NAMES dsound.h PATHS
				$ENV{HOME}/directx 
				$ENV{HOME}/directx/Include 
				/directx 
				/directx/Include 
				c:/directx)

  FIND_PACKAGE(YARP REQUIRED)

  IF (NOT WIN32)
    MESSAGE(FATAL_ERROR "vdub is for windows")
  ENDIF (NOT WIN32)
 
  IF (NOT DIRECTX_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "could not find directx header files")
  ENDIF (NOT DIRECTX_INCLUDE_DIR)
 
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${DIRECTX_INCLUDE_DIR})

  LINK_LIBRARIES(advapi32 gdi32 ole32 oleaut32 user32 winmm uuid strmiids comctl32)
  ADD_LIBRARY(vdub WinCap.h WinCap.cpp bitmap.cpp     Error.cpp     memory.cpp       registry.cpp cap_dshow.cpp  Fraction.cpp  misc.cpp         text.cpp cap_vfw.cpp       pixmaputils.cpp  VDString.cpp)

  IF (NOT COMPILE_DEVICE_LIBRARY)
    LINK_LIBRARY(vdub)
    ADD_EXECUTABLE(vdub_test  main.cpp)
  ENDIF (NOT COMPILE_DEVICE_LIBRARY)

ENDIF(NOT SKIP_vdub)
