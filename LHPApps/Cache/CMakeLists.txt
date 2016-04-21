PROJECT(Cache)

# Set your list of sources here.
SET(PROJECT_SRCS
	AbstractCachedFunction.h
	CacheConfiguration.cpp
	CacheConfiguration.h
	CacheData.cpp
	CacheData.h
	CacheDataStructure.cpp
	CacheDataStructure.h
	CachedFunction.h
	CachedFunctionDeclaration.h
	CachedFunctionManager.cpp
	CachedFunctionManager.h
	CachedFunctionParent.h
	CacheManagerConfiguration.h
	CachePolicy.cpp
	CachePolicy.h
	DefaultCachePolicy.cpp
	DefaultCachePolicy.h
	LFUCachePolicy.cpp
	LFUCachePolicy.h
	ParameterInfo.h
	ParameterType.h
	ReturnInfo.h
	ReturnType.h
	StandardDestroyFunctions.h
	StandardEqualFunctions.cpp
	StandardEqualFunctions.h
	StandardGetSizeFunctions.cpp
	StandardGetSizeFunctions.h
	StandardHashFunctions.cpp
	StandardHashFunctions.h
	StandardInitFunctions.cpp
	StandardInitFunctions.h
	StandardOutputFunctions.cpp
	StandardOutputFunctions.h
	StandardReturnFunctions.h
	TypedParameterInfo.h
	TypedReturnInfo.h
	TypedValue.h
	Value.h
	
	Cache.h
	Cache.cpp
)

#Add support for VS 2010 Edit & Continue
IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")

#Add OpenMP Support + Support for Parallel Compilation
IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")


IF(LHP_BUILD_DLL)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
  SET(BUILD_SHARED_LIBS 1)
  ADD_DEFINITIONS(-DLHP_VME_EXPORTS)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} mafDLL lhpCommon vtkLHP ${LHPExtLibs_TARGETS})

  IF (LHPExtLibs_BUILD_TARGETS)
    ADD_DEPENDENCIES(${PROJECT_NAME} ${LHPExtLibs_BUILD_TARGETS})
  ENDIF (LHPExtLibs_BUILD_TARGETS)
ELSE(LHP_BUILD_DLL)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
ENDIF (LHP_BUILD_DLL)

