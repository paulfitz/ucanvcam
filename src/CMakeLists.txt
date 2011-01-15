
SET (STATLIB "${CMAKE_BINARY_DIR}/static_libs")
IF (EXISTS ${STATLIB})
    MESSAGE(STATUS "static_libs directory present: ${STATLIB}")
    LINK_DIRECTORIES(${STATLIB})
    ADD_DEFINITIONS(-static-libgcc)
    SET(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -static-libgcc")
    FILE(GLOB statlibs ${STATLIB}/*.a)
    LINK_LIBRARIES(${statlibs})
ENDIF (EXISTS ${STATLIB})

IF (MINGW)
  ADD_DEFINITIONS(-DMINGW)
  LINK_LIBRARIES(winmm)
ENDIF (MINGW)

IF (WIN32)
  ADD_SUBDIRECTORY(activex)
ENDIF (WIN32)


IF (STANDALONE)

  FIND_PACKAGE(YARP REQUIRED)
  INCLUDE_DIRECTORIES(${YARP_INCLUDE_DIRS})
  SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${YARP_MODULE_PATH})

  FIND_PACKAGE(SDL REQUIRED)
  INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} ${SDL_INCLUDE_DIR}/..)

  # Search for native effects.
  FILE(GLOB_RECURSE effects_source native_effects/*.cpp native_effects/*.c)
  FILE(GLOB_RECURSE effects_header native_effects/*.h)

  # Search for "effectv" effects.
  # This source code should be left unchanged from the effectv source.
  FILE(GLOB_RECURSE effectv_source effectv_effects/*.cpp effectv_effects/*.c)
  FILE(GLOB_RECURSE effectv_header effectv_effects/*.h)

  FILE(GLOB_RECURSE common_source common/*.cpp common/*.c)
  FILE(GLOB_RECURSE common_header common/*.h)

  SET(common_sources ${common_source} ${common_header} 
  		   ${effectv_source} ${effectv_header}
		   ${effects_source} ${effects_header}
		   ${CMAKE_BINARY_DIR}/src/generated/config.h)
ENDIF ()

# Pick up any local header files.
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} 
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src/effectv_effects
                    ${CMAKE_SOURCE_DIR}/src/native_effects
                    ${CMAKE_SOURCE_DIR}/src/common)

SET(UCANVCAM_RESOURCE_PATH ${CMAKE_SOURCE_DIR}/media)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/config.h.in 
               ${CMAKE_BINARY_DIR}/src/generated/config.h
               IMMEDIATE)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/generated)

SET(TARGET_NAME ucanvcam)

# gui directory should really be called ui
SET(INTERFACE_SOURCE ${PROJECT_SOURCE_DIR}/src/gui/cmdline.cpp)

IF (USE_GUI)
  FIND_PACKAGE(wxWidgets REQUIRED)
  INCLUDE_DIRECTORIES(${wxWidgets_INCLUDE_DIRS})
  SET_PROPERTY(DIRECTORY PROPERTY COMPILE_DEFINITIONS ${wxWidgets_DEFINITIONS})
  ADD_DEFINITIONS(${wxWidgets_CXX_FLAGS})
  SET(INTERFACE_SOURCE ${PROJECT_SOURCE_DIR}/src/gui/gui.cpp)
ENDIF (USE_GUI)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gui)

IF (STANDALONE)

  IF (UNIX)
    
    ADD_EXECUTABLE(${TARGET_NAME} ${INTERFACE_SOURCE} linux/VcamLinux.cpp ${common_sources})
    
  ELSE (UNIX)
    
    # assume Windows
    
    ADD_DEFINITIONS(-DSHMEM_SERVICE)
    ADD_EXECUTABLE(${TARGET_NAME} ${INTERFACE_SOURCE} win32/VcamWin.cpp  ${common_sources})
  ENDIF (UNIX)
  
  TARGET_LINK_LIBRARIES(${TARGET_NAME} gd freetype)
  TARGET_LINK_LIBRARIES(${TARGET_NAME} ${YARP_LIBRARIES})
  IF (USE_GUI)
    # SDL does not seem to be really needed anymore, is this true?
    # check effectv effects
    TARGET_LINK_LIBRARIES(${TARGET_NAME} ${SDL_LIBRARY})
    TARGET_LINK_LIBRARIES(${TARGET_NAME} ${wxWidgets_LIBRARIES})
  ENDIF (USE_GUI)


  IF (WIN32)
    TARGET_LINK_LIBRARIES(${TARGET_NAME} jpeg png z)
  ENDIF ()

  #SET (DISABLE_SWSCALE TRUE)
  ADD_SUBDIRECTORY(modules)

  #SET(vcammod_DIR ${CMAKE_CURRENT_BINARY_DIR})
  SET(vcammod_DIR ${CMAKE_BINARY_DIR})

  #TARGET_IMPORT_DEVICES(${TARGET_NAME}
  #		      ${CMAKE_BINARY_DIR}/src/generated/drivers.h 
  #		      vcammod)
  target_link_libraries(${TARGET_NAME} vcammod)
  
  INSTALL(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
endif ()


