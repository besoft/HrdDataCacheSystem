PROJECT(CacheSystem)

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
	Hash.h
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
)

# Create the library.
ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})

# Inform CMAKE about the include directory to be used in the projects that depends on this
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})