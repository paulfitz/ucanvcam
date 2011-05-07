
OPTION(USE_YARP "Use yarp" FALSE)

IF (USE_YARP)
  FIND_PACKAGE(YARP REQUIRED)
  INCLUDE_DIRECTORIES(${YARP_INCLUDE_DIRS})
  LINK_LIBRARIES(${YARP_LIBRARIES})
  ADD_DEFINITIONS(-DUSE_YARP)
ENDIF ()

IF (MINGW)
  ADD_DEFINITIONS(-D_OBJBASE_H_)
  ADD_DEFINITIONS(-fno-exceptions -mms-bitfields -O2  -march=pentium -mwindows -mthreads)
  #ADD_DEFINITIONS(-fno-rtti -fno-exceptions -fno-pcc-struct-return -mms-bitfields -O2  -march=pentium -mthreads)
  #  -fstrict-aliasing
  #SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-stdcall-alias -Wl,--stack,0xF00000")
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-stdcall-alias -Wl,--stack,0x200000 -Wl,--subsystem,windows -Wl,-export-all-symbols")
  #SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-stdcall-alias")
  LINK_LIBRARIES(winmm stdc++)
ENDIF (MINGW)

# for WIN32 only

FIND_PROGRAM(windres NAMES windres.exe windres mingw32-windres i586-mingw32msvc-windres PATHS /bin /usr/bin)
MESSAGE(STATUS "windres at ${windres}")
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources.o
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/prop.rc
		COMMAND ${windres} -I${CMAKE_CURRENT_SOURCE_DIR}
		-i ${CMAKE_CURRENT_SOURCE_DIR}/prop.rc
		-o ${CMAKE_CURRENT_BINARY_DIR}/resources.o)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../win32)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FIND_PACKAGE(DIRECTX REQUIRED)

INCLUDE_DIRECTORIES(${DIRECTX_BASECLASSES_DIR} ${DIRECTX_INCLUDE_DIR})
FILE(GLOB_RECURSE directx_source ${DIRECTX_BASECLASSES_DIR}/*.cpp)

#ADD_LIBRARY(ucanvcamx SHARED Filters.cpp Dll.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.o ${directx_source} ${CMAKE_CURRENT_SOURCE_DIR}/../common/registry.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/vivek)

ADD_LIBRARY(ucanvcamx SHARED vivek/Filters.cpp vivek/Dll.cpp memy.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.o ${directx_source})
#ADD_LIBRARY(ucanvcamx SHARED FBall.cpp Ball.cpp CBallProp.cpp memy.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.o ${directx_source})
#ADD_LIBRARY(ucanvcamx SHARED Filters.cpp Dll.cpp memy.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.o ${directx_source})

TARGET_LINK_LIBRARIES(ucanvcamx advapi32 gdi32 ole32 oleaut32 user32 winmm uuid strmiids comctl32)
TARGET_LINK_LIBRARIES(ucanvcamx ${YARP_LIBRARIES} ${SDL_LIBRARIES})


INSTALL(TARGETS ucanvcamx DESTINATION bin)
