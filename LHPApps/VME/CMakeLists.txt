PROJECT(lhpVME)

# Set your list of sources here.
SET(PROJECT_SRCS
	mafPipeIntGraph.cpp
	mafPipeIntGraph.h
	mafVMEAFRefSys.cpp
	mafVMEAFRefSys.h
	mafVMEC3DData.cpp
	mafVMEC3DData.h
	mafVMEHelAxis.cpp
	mafVMEHelAxis.h
	
	lhpVisualPipeSurfaceScalar.cpp
	lhpVisualPipeSurfaceScalar.h
	
	medVMEMuscleWrapper.cpp	
	medVMEMuscleWrapper.h
	lhpPipeAnalogGraphXY.cpp
	lhpPipeAnalogGraphXY.h
	lhpVMEScalarMatrix.cpp
	lhpVMEScalarMatrix.h

	lhpPipeSurfacePlot.cpp
	lhpPipeSurfacePlot.h


	# for VolumeBrush
	lhpPipeVolumeBrushSlice.cpp
	lhpPipeVolumeBrushSlice.h
    
	lhpVMEPoseGroup.cpp
	lhpVMEPoseGroup.h

	########### VPHOP WP10 - BEGIN ########
	medVMEMuscleWrapper_Core.cpp
	medVMEMuscleWrapper_Helper.cpp
	medVMEMuscleWrapper_GUI.cpp
	medVMEMuscleWrapper_DBG.cpp
	vtkMEDPolyDataDeformationWrapperJH.cpp
	vtkMEDPolyDataDeformationWrapperJH.h

	mafVMEShortcut.cpp
	mafVMEShortcut.h

	medVMEMusculoSkeletalModel.cpp
	medVMEMusculoSkeletalModel.h
	medMSMGraph.cpp
	medMSMGraph.h

	medVMEJoint.cpp
	medVMEJoint.h

	medVMEOutputJoint.cpp
	medVMEOutputJoint.h

	medPipeJoint.cpp
	medPipeJoint.h
	
	lhpPipeMeshFake.cpp
	lhpPipeMeshFake.h

	########### VPHOP WP10 - END #########
	
	mafJointAnalysis.cpp
	mafJointAnalysis.h

  lhpOpSetLowerRes.cpp
	lhpOpSetLowerRes.h
	
	vtkMEDPolyDataDeformationPKCaching.h
	vtkMEDPolyDataDeformationPKCaching.cpp
)

IF (MAF_USE_ITK)
SET(PROJECT_SRCS  ${PROJECT_SRCS}
	lhpVMESurfaceScalarVarying.cpp
	lhpVMESurfaceScalarVarying.h
)
ENDIF (MAF_USE_ITK)

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

