#
# Copyright and description to be placed here
#

PROJECT(lhpCommon)

# Set your list of sources here.
SET(PROJECT_SRCS
mafPlotMath.cpp
mafPlotMath.h
forarray.h
VecProc.h
vectors.h
splines.h
lhpUtils.h
lhpUtils.cpp
lhpInteractorPicker.cpp
lhpInteractorPicker.h
lhpInteractorPERScalarInformation.cpp
lhpInteractorPERScalarInformation.h
lhpInteractorPERBrushFeedback.cpp
lhpInteractorPERBrushFeedback.h
)

IF(LHP_BUILD_DLL)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
  SET(BUILD_SHARED_LIBS 1)
  ADD_DEFINITIONS(-DLHP_COMMON_EXPORTS)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} mafDLL)
ELSE(LHP_BUILD_DLL)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
ENDIF (LHP_BUILD_DLL)

