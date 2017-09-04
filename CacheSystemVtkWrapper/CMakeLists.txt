PROJECT(CacheSystemVtkWrapper)

# Set your list of sources here.
SET(PROJECT_SRCS
  CacheManagerSource.cpp
  CacheManagerSource.h
  CacheUtils.cpp
  CacheUtils.h
  CachingFilter.h
)

# Create the library.
ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})

# this project needs 'CacheSystem', and 'VTK'
target_link_libraries(${PROJECT_NAME} CacheSystem ${VTK_LIBRARIES})

# Inform CMAKE about the include directory to be used in the projects that depends on this
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${VTK_INCLUDE_DIRS})
