
IF (MINGW)
  ADD_DEFINITIONS(-D_OBJBASE_H_)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-stdcall-alias -Wl,--stack,0x200000")
  LINK_LIBRARIES(winmm)
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

# fix this for your own path to directx
FIND_PATH(DIRECTX_BASECLASSES_DIR NAMES amfilter.h PATHS
	  			$ENV{HOME}/directx 
				$ENV{HOME}/directx/BaseClasses
				/directx 
				/directx/BaseClasses
				c:/directx
				c:/directx/BaseClasses)
FIND_PATH(DIRECTX_INCLUDE_DIR NAMES dsound.h PATHS
				$ENV{HOME}/directx 
				$ENV{HOME}/directx/Include 
				/directx 
				/directx/Include 
				c:/directx)

INCLUDE_DIRECTORIES(${DIRECTX_BASECLASSES_DIR} ${DIRECTX_INCLUDE_DIR})
FILE(GLOB_RECURSE directx_source ${DIRECTX_BASECLASSES_DIR}/*.cpp)

ADD_LIBRARY(ucanvcamx SHARED Filters.cpp Dll.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.o ${directx_source} ${CMAKE_CURRENT_SOURCE_DIR}/../common/registry.cpp)

TARGET_LINK_LIBRARIES(ucanvcamx advapi32 gdi32 ole32 oleaut32 user32 winmm uuid strmiids comctl32)

INSTALL(TARGETS ucanvcamx DESTINATION bin)
