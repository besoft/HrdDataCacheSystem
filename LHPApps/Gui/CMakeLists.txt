#
# Copyright and description to be placed here
#

PROJECT(lhpGui)

#DBG_MESSAGE("Configuring project ${PROJECT_NAME}")

# Set your list of sources here.
SET(PROJECT_SRCS
  lhpGUINetworkConnectionSettings.cpp
  lhpGUINetworkConnectionSettings.h
  lhpGUIPythonSettings.cpp
  lhpGUIPythonSettings.h
  psLoaderGUIContextualMenu.cpp
  psLoaderGUIContextualMenu.h
  lhpGUISettingsOpUploadMultiVMERefactor.cpp
  lhpGUISettingsOpUploadMultiVMERefactor.h
  lhpGUIOpenSimSettings.h
  lhpGUIOpenSimSettings.cpp
  lhpGUIHistogramWidget.h
  lhpGUIHistogramWidget.cpp
)

# List libraries that are needed by this project.
SET(PROJECT_LIBS  mafGui medGui)

IF(LHP_BUILD_DLL)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
  SET(BUILD_SHARED_LIBS 1)
  ADD_DEFINITIONS(-DLHP_GUI_EXPORTS)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} mafDLL lhpCore ${LHPExtLibs_TARGETS})

  IF (LHPExtLibs_BUILD_TARGETS)
    ADD_DEPENDENCIES(${PROJECT_NAME} ${LHPExtLibs_BUILD_TARGETS})
  ENDIF (LHPExtLibs_BUILD_TARGETS)
ELSE(LHP_BUILD_DLL)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PROJECT_LIBS})
ENDIF (LHP_BUILD_DLL)
