CMAKE_MINIMUM_REQUIRED(VERSION 1.6)

# Set the project/library name here.  Classes should be declared as
# "class vtkLHP_EXPORT vtkFoo", where vtkLHP is the name of the
# project set here.
PROJECT(vtkLHP)

# Set your list of sources here.  Do not change the name of the
# vtkLHP_SRCS variable.
SET(vtkLHP_SRCS
# source files list here...	

)

SET(vtkLHP_SRCS	
	vtkMAFMuscleDecomposition.cpp	
	vtkMAFMuscleDecomposition.h	
	vtkMAFMuscleFibers.cpp	
	vtkMAFMuscleFibers.h

	########### VPHOP WP10 - BEGIN ########
	vtkMEDPolyDataDeformationPK.cpp
	vtkMEDPolyDataDeformationPK.h
	vtkProgressiveHull.cxx
	vtkProgressiveHull.h
	vtkProgressiveHullCPU.cxx
	vtkProgressiveHullCPU.h
	vtkProgressiveHullCUDA.cxx
	vtkProgressiveHullCUDA.h
	
	vtkMAFMuscleDecompositionMMSS.cpp	
	vtkMAFMuscleDecompositionMMSS.h	
	vtkMassSpringMuscle.h
	vtkMassSpringMuscle.cpp
	
	vtkMAFPolyDataCutOutFilterEx.cxx
	vtkMAFPolyDataCutOutFilterEx.h
	vtkMAFMeanValueCoordinatesInterpolation.h
	vtkMAFMeanValueCoordinatesInterpolation.cpp

	vtkMAFVisualDebugger.cpp
	vtkMAFVisualDebugger.h

	vtkMuscleDecomposer.cpp
	vtkMuscleDecomposer.h
	########### VPHOP WP10 - END ########	
)

SET(vtkLHP_SRCS_INSTANTIATOR
	${vtkLHP_SRCS}
)

SET(vtkLHP_SRCS
	${vtkLHP_SRCS}
	PriorityQueue.h
  MMSSVector3d.cpp

	MMSSVector3d.h
	MMSSUtil.cpp
	MMSSUtil.h
	MMSSMorphing.cpp
	MMSSMorphing.h
	MeshSurface.cpp
	MeshSurface.h
	PKUtils.cpp
	PKUtils.h
	PKMath.cpp
	PKMath.h
	PKSolutionState.cpp
	PKSolutionState.h
	IMassSpringSystem.h
	MassSpringSystemCPU.cpp
	MassSpringSystemCPU.h
	IMultiMeshDeformer.h
	OoCylinder.cpp
	OoCylinder.h
	MeshNavigator.cpp
	MeshNavigator.h
	MeshSkeleton.cpp
	MeshSkeleton.h
	DeformationCache.cpp
	DeformationCache.h
	vtkMEDPolyDataDeformationPK_DBG.cpp
	vtkMAFPolyDataCutOutFilterEx_DBG.cxx
	MutexLocker.h
	PKHashTable.cpp
	PKHashTable.h
	vtkMAFMuscleDecompositionAdv.cpp
  csparse.c

	csparse.h






        gl_init.h
        Library.h
        Library.cpp
        MassSpringSystemGPU.h
        MassSpringSystemGPU.cpp
        
	collisiondetection.cpp
	collisiondetection.h
	GPUOGL_CD.cpp
	GPUOGL_CD.h
	PKSolutionStateGPU.cpp
	PKSolutionStateGPU.h
)

SET(vtkLHP_SHADERS_CD 
	layers.vert
	layers.frag
	pass.vert
	pass.frag
	pass2.frag
	show.vert
	show.frag
)

SET(vtkLHP_SHADERS_MSS
	fs_mss_rf.f
	fs_mss_rp.f
	gs_mss_rf.f
	vs_mss_rf.f
	vs_mss_rp.f
)
 

# List the kits from VTK that are needed by this project.
SET(vtkLHP_LIBS
  vtkCommon
  vtkRendering
  vtkFiltering
  lpsolve55
)

#-----------------------------------------------------------------------------
# Most users should not need to change anything below this line.

#Add support for VS 2010 Edit & Continue
IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")

#Add OpenMP Support + Support for Parallel Compilation
IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")

# Need to include class headers and the configuration header.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

# Find and load VTK settings.
IF (NOT VTK_FOUND)
  INCLUDE(${CMAKE_ROOT}/CMakeModules/FindVTK.cmake)
  IF(VTK_FOUND)
    INCLUDE(${VTK_USE_FILE})
  ENDIF(VTK_FOUND)
ENDIF (NOT VTK_FOUND)

# Give user option of building shared or static libraries.  Default to
# the choice made for VTK.
# OPTION(BUILD_SHARED_LIBS "Build with shared libraries." 
       # ${VTK_BUILD_SHARED_LIBS})

# Set VTKLOCAL_BUILD_SHARED_LIBS to 1 or 0 for use in creating the
# configuration header.
# IF(BUILD_SHARED_LIBS)
  # SET(VTKLOCAL_BUILD_SHARED_LIBS_CONFIG 1)
# ELSE(BUILD_SHARED_LIBS)
  # SET(VTKLOCAL_BUILD_SHARED_LIBS_CONFIG 0)
# ENDIF(BUILD_SHARED_LIBS)

IF(LHP_BUILD_DLL)
  # IF(WIN32)
  # SET(vtkLHP_LIBS
    # ${vtkLHP_LIBS}
    # GPUAPI
    # )
  # ENDIF(WIN32)
  SET(BUILD_SHARED_LIBS 1)
  SET(VTKLOCAL_BUILD_SHARED_LIBS_CONFIG 1)
  ADD_DEFINITIONS(-DvtkLHP_EXPORTS)
ELSE(LHP_BUILD_DLL)
  SET(BUILD_SHARED_LIBS 0)
  SET(VTKLOCAL_BUILD_SHARED_LIBS_CONFIG 0)
ENDIF(LHP_BUILD_DLL)

IF (NOT LIBRARY_OUTPUT_PATH)
  SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH
       "Single output directory for building all libraries.")
  MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH)
ENDIF (NOT LIBRARY_OUTPUT_PATH)

IF (EXECUTABLE_OUTPUT_PATH)
  SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH
       "Single output directory for building all executables.")
  MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH)
ENDIF (EXECUTABLE_OUTPUT_PATH)

IF(VTK_FOUND)
  # If this is a build tree, provide an option for putting this
  # project's executables and libraries in with VTK's.
  IF (EXISTS ${VTK_DIR}/bin)
    OPTION(USE_VTK_OUTPUT_PATHS
     "Use VTK's output directory for this project's executables and libraries."
     OFF)
    MARK_AS_ADVANCED (USE_VTK_OUTPUT_PATHS)
    IF (USE_VTK_OUTPUT_PATHS)
      SET (LIBRARY_OUTPUT_PATH ${VTK_DIR}/bin)
      SET (EXECUTABLE_OUTPUT_PATH ${VTK_DIR}/bin)
    ENDIF (USE_VTK_OUTPUT_PATHS)
  ENDIF (EXISTS ${VTK_DIR}/bin)

  # Create the header to configure the classes.
  CONFIGURE_FILE(${LHP_SOURCE_DIR}/CMakeInput/vtkLHPConfigure.h.in
                 ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Configure.h
                 @ONLY IMMEDIATE)

  # Create the instantiator for these classes.
  SET(VTK_USE_INSTANTIATOR_NEW 1)
  VTK_MAKE_INSTANTIATOR2("${PROJECT_NAME}Instantiator" vtkLHPInstantiator_SRCS
                         ${vtkLHP_SRCS_INSTANTIATOR}
                         EXPORT_MACRO "VTK_${PROJECT_NAME}_EXPORT"
                         HEADER_LOCATION ${PROJECT_BINARY_DIR}
                         INCLUDES "${PROJECT_NAME}Configure.h")

  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${vtkLHP_SRCS} ${vtkLHPInstantiator_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${vtkLHP_LIBS} ${LHPExtLibs_TARGETS})
  # this is necessaty to force MFL compilation....
  IF (LHPExtLibs_BUILD_TARGETS)
    ADD_DEPENDENCIES(${PROJECT_NAME} ${LHPExtLibs_BUILD_TARGETS})
  ENDIF (LHPExtLibs_BUILD_TARGETS)
  
  # Copy shaders
  FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
    SET(OUT_SHADER_DIR ${EXECUTABLE_OUTPUT_PATH}/${config}/Shaders/GPUCollisionDetection)
    FILE(MAKE_DIRECTORY ${OUT_SHADER_DIR})		
    FOREACH(file ${vtkLHP_SHADERS_CD})
      CONFIGURE_FILE(${LHP_SOURCE_DIR}/vtkLHP/Shaders/GPUCollisionDetection/${file} 
                     ${OUT_SHADER_DIR}/${file} 
                     COPYONLY)
    ENDFOREACH(file)

    SET(OUT_SHADER_DIR ${EXECUTABLE_OUTPUT_PATH}/${config}/Shaders/MSS)
    FILE(MAKE_DIRECTORY ${OUT_SHADER_DIR})		
    FOREACH(file ${vtkLHP_SHADERS_MSS})
      CONFIGURE_FILE(${LHP_SOURCE_DIR}/vtkLHP/Shaders/MSS/${file} 
                     ${OUT_SHADER_DIR}/${file} 
                     COPYONLY)
    ENDFOREACH(file)
  ENDFOREACH(config)
  

  # Create Tcl wrappers if VTK is wrapped in Tcl.
  IF(VTK_WRAP_TCL)
    VTK_WRAP_TCL2(${PROJECT_NAME}TCL vtkLHPTCL_SRCS ${vtkLHP_SRCS})
    ADD_LIBRARY(${PROJECT_NAME}TCL ${vtkLHPTCL_SRCS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}TCL vtkCommonTCL ${PROJECT_NAME})

    # Create the list of kits that are required.
    SET(VTKLOCAL_KITS "")
    FOREACH(c ${vtkLHP_LIBS})
      SET(VTKLOCAL_KITS "${VTKLOCAL_KITS} ${c}")
    ENDFOREACH(c)
    FOREACH(c ${vtkLHP_SRCS})
      GET_FILENAME_COMPONENT(VTKLOCAL_LAST_CLASS ${c} NAME_WE)
    ENDFOREACH(c)

    IF(CMAKE_CONFIGURATION_TYPES)
      FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
        SET(VTKLOCAL_TCL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}/${config}")
        CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/pkgIndex.tcl.in
                       ${PROJECT_BINARY_DIR}/${config}/pkgIndex.tcl
                       @ONLY IMMEDIATE)
      ENDFOREACH(config)
    ELSE(CMAKE_CONFIGURATION_TYPES)
      SET(VTKLOCAL_TCL_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
      CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/pkgIndex.tcl.in
                     ${PROJECT_BINARY_DIR}/pkgIndex.tcl
                     @ONLY IMMEDIATE)
    ENDIF(CMAKE_CONFIGURATION_TYPES)

  ENDIF(VTK_WRAP_TCL)

  # Create Python wrappers if VTK is wrapped in Python.
  IF(VTK_WRAP_PYTHON)
    VTK_WRAP_PYTHON2(${PROJECT_NAME}Python vtkLHPPYTHON_SRCS ${vtkLHP_SRCS})
    ADD_LIBRARY(${PROJECT_NAME}Python MODULE ${vtkLHPPYTHON_SRCS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}Python vtkCommonPython ${PROJECT_NAME})
  ENDIF(VTK_WRAP_PYTHON)

  # Create Java wrappers if VTK is wrapped in Java.
  IF(VTK_WRAP_JAVA)
    SET(VTK_WRAP_HINTS "${PROJECT_SOURCE_DIR}/hints")
    SET(VTK_JAVA_HOME ${PROJECT_BINARY_DIR}/java/${PROJECT_NAME})
    MAKE_DIRECTORY(${VTK_JAVA_HOME})
    VTK_WRAP_JAVA2(${PROJECT_NAME}Java vtkLHPJAVA_SRCS ${vtkLHP_SRCS})
    ADD_LIBRARY(${PROJECT_NAME}Java ${vtkLHPJAVA_SRCS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}Java vtkCommonJava ${PROJECT_NAME})
  ENDIF(VTK_WRAP_JAVA)  
ENDIF(VTK_FOUND)