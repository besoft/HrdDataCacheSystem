cmake_minimum_required(VERSION 2.8.11)

project(DependencyObject)

set(PROJECT_SRCS
				DependencyObject.cpp
)

set(PROJECT_HDRS
				
)
	
add_executable(${PROJECT_NAME} ${PROJECT_SRCS} ${PROJECT_HDRS})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}) # apply DEBUG_POSTFIX

# this project needs 'CacheSystemVtkWrapper'
target_link_libraries(${PROJECT_NAME} CacheSystemVtkWrapper ${VTK_LIBRARIES})
