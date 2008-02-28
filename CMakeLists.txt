PROJECT(ucanvcam)

# do we want a standalone program or a plugin of some kind?
SET (STANDALONE TRUE CACHE BOOL "standalone")

# new GUI not ready
SET (USE_NEW_GUI FALSE CACHE BOOL "work with new gui (not available yet!)")

IF (USE_NEW_GUI)
  FIND_PACKAGE(wxWidgets REQUIRED)
  INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
  ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})
  LINK_LIBRARIES(${wxWidgets_LIBRARIES})
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/wrapper/gui)
ENDIF (USE_NEW_GUI)


FIND_PACKAGE(YARP REQUIRED)

FIND_PACKAGE(SDL REQUIRED)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} ${SDL_INCLUDE_DIR}/..)
LINK_LIBRARIES(${SDL_LIBRARY})

# Search for native effects.
FILE(GLOB_RECURSE effects_source effects/*.cpp effects/*.c)
FILE(GLOB_RECURSE effects_header effects/*.h)

# Search for "effectv" effects.
# This source code should be left unchanged from the effectv source.
FILE(GLOB_RECURSE effectv_source effectv/*.cpp effectv/*.c)
FILE(GLOB_RECURSE effectv_header effectv/*.h)

# Pick up any local header files.
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} 
                    ${CMAKE_SOURCE_DIR}/effectv
                    ${CMAKE_SOURCE_DIR}/effects
                    ${CMAKE_SOURCE_DIR}/common)

FILE(GLOB_RECURSE common_source common/*.cpp common/*.c)
FILE(GLOB_RECURSE common_header common/*.h)

# sweetcam is the old name of ucanvcam
SET(SWEETCAM_RESOURCE_PATH ${CMAKE_SOURCE_DIR}/media)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.in 
               ${CMAKE_BINARY_DIR}/generated/config.h
               IMMEDIATE)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/generated)

SET(common_sources ${common_source} ${common_header} 
		   ${effectv_source} ${effectv_header}
		   ${effects_source} ${effects_header}
		   ${CMAKE_BINARY_DIR}/generated/config.h)

SET(TARGET_NAME ucanvcam)

IF (MINGW)
  ADD_DEFINITIONS(-D_OBJBASE_H_)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--add-stdcall-alias -Wl,--stack,0x200000")
  LINK_LIBRARIES(winmm)
ENDIF (MINGW)

IF (STANDALONE)
  IF (UNIX)
    IF (USE_NEW_GUI)
      ADD_EXECUTABLE(${TARGET_NAME} wrapper/gui/gui.cpp wrapper/linux/VcamLinuxStandalone.cpp ${common_sources})
    ELSE (USE_NEW_GUI)
      ADD_EXECUTABLE(${TARGET_NAME} wrapper/linux/standalone_linux.cpp ${common_sources})
    ENDIF (USE_NEW_GUI)
  ELSE (UNIX)
    ADD_EXECUTABLE(${TARGET_NAME} wrapper/win32/standalone_windows.cpp ${common_sources})
  ENDIF (UNIX)
ELSE (STANDALONE)
  IF (UNIX)
    ADD_EXECUTABLE(${TARGET_NAME} wrapper/linux/linux_loopback.cpp ${common_sources})
  ELSE (UNIX)
    FIND_PROGRAM(windres NAMES windres.exe windres mingw32-windres i586-mingw32msvc-windres PATHS /bin /usr/bin)
    MESSAGE(STATUS "windres at ${windres}")
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources.o
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/wrapper/win32/activex/prop.rc
		COMMAND ${windres} -I${CMAKE_CURRENT_SOURCE_DIR}
		-i ${CMAKE_CURRENT_SOURCE_DIR}/wrapper/win32/activex/prop.rc
		-o ${CMAKE_CURRENT_BINARY_DIR}/resources.o)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/wrapper/win32/activex)

    # fix this for your own path to directx
    SET(DIRECTX_BASECLASSES_DIR $ENV{HOME}/directx/BaseClasses)
    INCLUDE_DIRECTORIES(${DIRECTX_BASECLASSES_DIR})
    FILE(GLOB_RECURSE directx_source ${DIRECTX_BASECLASSES_DIR}/*.cpp)

    ADD_LIBRARY(${TARGET_NAME} SHARED wrapper/win32/activex/Filters.cpp wrapper/win32/activex/Dll.cpp ${CMAKE_CURRENT_BINARY_DIR}/resources.o ${directx_source} ${common_sources})
  ENDIF (UNIX)
ENDIF (STANDALONE)

TARGET_LINK_LIBRARIES(${TARGET_NAME} gd freetype)



ADD_SUBDIRECTORY(modules)


SET(vcammod_DIR ${CMAKE_CURRENT_BINARY_DIR})

TARGET_IMPORT_DEVICES(${TARGET_NAME}
		      ${CMAKE_BINARY_DIR}/generated/drivers.h 
		      vcammod)

SET(CPACK_PACKAGE_NAME "ucanvcam")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_SOURCE_IGNORE_FILES} "~$" ".svn$" ".svn/.*$")

INCLUDE(CPack)

ADD_CUSTOM_TARGET(dox COMMAND doxygen ${PROJECT_SOURCE_DIR}/doc/src/doxygen.conf)

