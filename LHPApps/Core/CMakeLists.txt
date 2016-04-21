#
# Copyright and description to be placed here
#
PROJECT(lhpCore)

# Set your list of sources here.
 SET(PROJECT_SRCS
	# need to solve some linking problems: for the moment moved save to operations
  # lhpFactoryTagHandler.cpp
  # lhpFactoryTagHandler.h
  lhpUser.cpp
  lhpUser.h
 )

IF(LHP_BUILD_DLL)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
  SET(BUILD_SHARED_LIBS 1)
  ADD_DEFINITIONS(-DLHP_CORE_EXPORTS)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} mafDLL lhpCommon)
ELSE(LHP_BUILD_DLL)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
ENDIF (LHP_BUILD_DLL)

