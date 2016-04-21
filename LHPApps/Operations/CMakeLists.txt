 PROJECT(lhpOperations)
 
 
# Set your list of sources here.
SET(PROJECT_SRCS
	#to be moved in core; need to solve some linking problems

	lhpOpModifyOpenSimModel.h
	lhpOpModifyOpenSimModel.cpp
	
	lhpOpModifyOpenSimModelCreateBodyFromSurface.h
	lhpOpModifyOpenSimModelCreateBodyFromSurface.cpp

	lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup.h
	lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup.cpp

	lhpOpModifyOpenSimModelCreateGeometry.h
	lhpOpModifyOpenSimModelCreateGeometry.cpp
	
	lhpOpModifyOpenSimModelCreateCustomJoint.h
	lhpOpModifyOpenSimModelCreateCustomJoint.cpp

	lhpOpModifyOpenSimModelCreateFreeJoint.h
	lhpOpModifyOpenSimModelCreateFreeJoint.cpp

	lhpOpModifyOpenSimModelCreatePinJoint.h
	lhpOpModifyOpenSimModelCreatePinJoint.cpp

	lhpOpModifyOpenSimModelCreateBallJoint.h
	lhpOpModifyOpenSimModelCreateBallJoint.cpp

	lhpOpModifyOpenSimModelCreateMuscle.h
	lhpOpModifyOpenSimModelCreateMuscle.cpp

	lhpOpModifyOpenSimModelCreateMuscleMultiPoint.h
	lhpOpModifyOpenSimModelCreateMuscleMultiPoint.cpp
	
	lhpOpModifyOpenSimModelCreateMuscleFromMeter.h
	lhpOpModifyOpenSimModelCreateMuscleFromMeter.cpp
	
	lhpOpModifyOpenSimModelCreateMarkerSet.h
	lhpOpModifyOpenSimModelCreateMarkerSet.cpp
	
	lhpOpCreateOpenSimModel.h
	lhpOpCreateOpenSimModel.cpp	

	lhpOpImporterOpenSimIKSimulationResults.h
	lhpOpImporterOpenSimIKSimulationResults.cpp
	
	lhpOpImporterOpenSimIDSimulationResults.h
	lhpOpImporterOpenSimIDSimulationResults.cpp
	
	lhpOpImporterOpenSimSOSimulationResults.h
	lhpOpImporterOpenSimSOSimulationResults.cpp

	lhpOpCreateOpenSimModelWorkflow.h
	lhpOpCreateOpenSimModelWorkflow.cpp
	
	lhpGuiDialogCreateOpenSimModelWorkflow.h
	lhpGuiDialogCreateOpenSimModelWorkflow.cpp
	
	lhpFactoryTagHandler.cpp
	lhpFactoryTagHandler.h
	
	lhpTagHandlerContainer.cpp
	lhpTagHandlerContainer.h
	
	lhpTagHandlerDICOMContainer.cpp
	lhpTagHandlerDICOMContainer.h
	
	lhpTagHandler.cpp
	lhpTagHandler.h
	
	#
	# VMEUploaderDownloader Refactor Target ("Refactor" word to be removed since legacy files have been deleted...)
	#
	
	lhpUploadDownloadVMEHelper.h
	lhpUploadDownloadVMEHelper.cpp
	
	lhpOpUploadVMERefactor.cpp
	lhpOpUploadVMERefactor.h
	
	lhpOpUploadMultiVMERefactor.cpp
	lhpOpUploadMultiVMERefactor.h
	
	lhpOpDownloadVMERefactor.cpp
	lhpOpDownloadVMERefactor.h
	
	lhpOpEditTagRefactor.cpp
	lhpOpEditTagRefactor.h
	
	lhpOpOpenRepository.cpp
	lhpOpOpenRepository.h
	
	#######################################
	
	#
	# Google KeyCZar Integration Test
	#

	lhpOpKeyczarIntegrationTest.cpp
	lhpOpKeyczarIntegrationTest.h

	#######################################
	
	#
	lhpOpBonematCommon.cpp
	lhpOpBonematCommon.h

	lhpOpBonemat.cpp
	lhpOpBonemat.h

	lhpOpBonematCaller.cpp
	lhpOpBonematCaller.h
	
	lhpOpBonematBatch.cpp
	lhpOpBonematBatch.h
	
	lhpOpCompareMesh.cpp
	lhpOpCompareMesh.h
		
	lhpOpImporterAnsysCommon.cpp
	lhpOpImporterAnsysCommon.h
	
	lhpOpImporterAnsysInputFile.cpp
	lhpOpImporterAnsysInputFile.h
	lhpOpImporterAnsysCDBFile.cpp
	lhpOpImporterAnsysCDBFile.h
	
	lhpOpExporterAnsysCommon.cpp
	lhpOpExporterAnsysCommon.h

	lhpOpExporterAnsysInputFile.cpp
	lhpOpExporterAnsysInputFile.h
	lhpOpExporterAnsysCDBFile.cpp
	lhpOpExporterAnsysCDBFile.h
	
	# multiscale files
	lhpOpMultiscaleExplore.cpp
	lhpOpMultiscaleExplore.h
	lhpMultiscaleActor.cpp
	lhpMultiscaleActor.h
	lhpMultiscaleActorCoordsUtility.cpp
	lhpMultiscaleActorCoordsUtility.h
	lhpMultiscaleCallbacks.cpp
	lhpMultiscaleCallbacks.h
	lhpMultiscaleCameraParams.cpp
	lhpMultiscaleCameraParams.h
	lhpMultiscaleCameraUtility.cpp
	lhpMultiscaleCameraUtility.h
	lhpMultiscaleUtility.cpp
	lhpMultiscaleUtility.h
	lhpMultiscaleVectorMath.cpp
	lhpMultiscaleVectorMath.h
	lhpMultiscaleVisualPipes.cpp
	lhpMultiscaleVisualPipes.h

	# texture orientation files
	lhpCooccurrenceMatrixFilter.cpp
	lhpCooccurrenceMatrixFilter.h
	lhpHistogram1D.cpp
	lhpHistogram1D.h
	lhpHistogramEqualizationFilter.cpp
	lhpHistogramEqualizationFilter.h
	lhpIsotropicSampleTable.cpp
	lhpIsotropicSampleTable.h
	lhpOpTextureOrientation.cpp
	lhpOpTextureOrientation.h
	lhpTextureOrientationCalculator.cpp
	lhpTextureOrientationCalculator.h
	lhpTextureOrientationCallbacks.cpp
	lhpTextureOrientationCallbacks.h
	lhpTextureOrientationFilter.cpp
	lhpTextureOrientationFilter.h
	lhpTextureOrientationVectorGlyphPipe.cpp
	lhpTextureOrientationVectorGlyphPipe.h
	lhpTextureOrientationTensorGlyphPipe.cpp
	lhpTextureOrientationTensorGlyphPipe.h

	lhpTextureOrientationSlicePipe.cpp
	lhpTextureOrientationSlicePipe.h
	lhpTextureOrientationUseful.cpp
	lhpTextureOrientationUseful.h
	
	lhpVolumeInterpolator.cpp
	lhpVolumeInterpolator.h
	lhpProceduralElements.cxx
	lhpProceduralElements.h
	mafDictionary.cpp
	mafDictionary.h
	mafInpReader.cpp
	mafInpReader.h
	mafINPWriter.cpp
	mafINPWriter.h
	mafMtrLMCReader.cpp
	mafMtrLMCReader.h
	mmoAFSys.cpp
	mmoAFSys.h
	mmoAverageLM.cpp
	mmoAverageLM.h
	mmoBuildHierarchy.cpp
	mmoBuildHierarchy.h
	mmoHelAxis.cpp
	mmoHelAxis.h
	mmoINPExporter.cpp
	mmoINPExporter.h
	mmoINPImporter.cpp
	mmoINPImporter.h
	mmoMTRExporter.cpp
	mmoMTRExporter.h
	mmoMTRImporter.cpp
	mmoMTRImporter.h
	mmoStickPalpation.cpp
	mmoStickPalpation.h
	mmoTimeReduce.cpp
	mmoTimeReduce.h
	mmoLnSurf.cpp
	mmoLnSurf.h
	
	# lhpOpImporterC3D.cpp
	# lhpOpImporterC3D.h

	lhpOpComputeTensor.cpp
	lhpOpComputeTensor.h
	lhpCOmputeTensor.cpp
	lhpComputeTensor.h
        
	medOpCreateMuscleWrapper.cpp	
	medOpCreateMuscleWrapper.h

	lhpOpMeshGeneratorExecutable.cpp
	lhpOpMeshGeneratorExecutable.h

	lhpOpImporterInriaMesh.cpp
	lhpOpImporterInriaMesh.h
  
	# VolumeBrush additions, Youbing, Oct. 2009
	lhpOp3DVolumeBrush.cpp
	lhpOp3DVolumeBrush.h
	lhpViewVolumeBrush.cpp
	lhpViewVolumeBrush.h
	lhpFrameVolumeBrush.cpp
	lhpFrameVolumeBrush.h
	lhpInteractorVolumeBrush.cpp
	lhpInteractorVolumeBrush.h
	lhpVolumeBrushContextualMenu.cpp
	lhpVolumeBrushContextualMenu.h
	lhpGizmoVolumeBrush.cpp
	lhpGizmoVolumeBrush.h
	vtkLHPSelectionVolumeGray.cpp
	vtkLHPSelectionVolumeGray.h
	vtkLHPImageStencilData.cpp
	vtkLHPImageStencilData.h
	vtkLHPVolumeTextureMapper3D.cpp
	vtkLHPVolumeTextureMapper3D.h
	vtkLHPOpenGLVolumeTextureMapper3D.cpp
	vtkLHPOpenGLVolumeTextureMapper3D.h
	# End of VolumeBrush additions
  
	lhpOpImporterAnalogASCII.cpp
	lhpOpImporterAnalogASCII.h
	
	lhpOpImporterMeshFeBio.cpp
	lhpOpImporterMeshFeBio.h
	lhpOpExporterMeshFeBio.cpp
	lhpOpExporterMeshFeBio.h
	lhpMeshFeBioReader.cpp
	lhpMeshFeBioReader.h

	lhpMeshFeBioWriter.cpp
	lhpMeshFeBioWriter.h	

	########### VPHOP WP10 - BEGIN ########
	mafOpCreateVMEShortcut.cpp
	mafOpCreateVMEShortcut.h

	#lhpOpSetLowerRes.cpp
	#lhpOpSetLowerRes.h

	lhpOpCopyMusculoskeletalModel.cpp
	lhpOpCopyMusculoskeletalModel.h
	lhpOpFuseMotionData.cpp
	lhpOpFuseMotionData.h

	lhpOpComputeProgressiveHull.cpp
	lhpOpComputeProgressiveHull.h

	lhpOpMergeSurfaces.cpp
	lhpOpMergeSurfaces.h

	lhpOpMoveVMEDown.cpp
	lhpOpMoveVMEDown.h
	lhpOpMoveVMEUp.cpp
	lhpOpMoveVMEUp.h
	lhpOpMultiReparentTo.cpp
	lhpOpMultiReparentTo.h
	lhpOpRemoveTracability.cpp
	lhpOpRemoveTracability.h
	lhpOpSmoothMuscle.cpp
	lhpOpSmoothMuscle.h

	lhpOpFilterPolyDataUsingExtApp.cpp
	lhpOpFilterPolyDataUsingExtApp.h
	lhpOpMSModelCheckTags.cpp
	lhpOpMSModelCheckTags.h

	lhpOpASFMotionRetargeting.cpp
	lhpOpASFMotionRetargeting.h
 	lhpOpMotionRetargeting.cpp
	lhpOpMotionRetargeting.h
	medOpCreateJoint.cpp
	medOpCreateJoint.h
    medOpCreateParticle.cpp
    medOpCreateParticle.h
	
	lhpOpTransform.cpp
	lhpOpTransform.h
	
	lhpOpScaleMMA.h
	lhpOpScaleMMA.cpp	
	lhpInteractorExtractLandmarkEOS.h
	lhpInteractorExtractLandmarkEOS.cpp
	lhpOpExtractLandmarkEOS.h
	lhpOpExtractLandmarkEOS.cpp

	lhpOpConvertMotionData.cpp
	lhpOpConvertMotionData.h

	lhpOpMuscleDecompose.cpp
	lhpOpMuscleDecompose.h

	########### VPHOP WP10 - END #########

	
	lhpOpComputeHausdorffDistance.cpp
	lhpOpComputeHausdorffDistance.h
	lhpComputeHausdorffDistance.cpp
	lhpComputeHausdorffDistance.h

	lhpOpComputeGradientStructureTensor.h
	lhpOpComputeGradientStructureTensor.cpp

	lhpOpShowHistogram.cpp
	lhpOpShowHistogram.h

	lhpOpComputeGradientStructureTensor.h
	lhpOpComputeGradientStructureTensor.cpp

	lhpOpExporterMatrixOctave.cpp
	lhpOpExporterMatrixOctave.h
	lhpOpImporterMatrixOctave.cpp
	lhpOpImporterMatrixOctave.h

)

IF (USE_MATIO_API)
	SET(PROJECT_SRCS 
		${PROJECT_SRCS}
		lhpOpImporterMatrixMatlab.h
		lhpOpImporterMatrixMatlab.cpp
		lhpOpExporterMatrixMatlab.h
		lhpOpExporterMatrixMatlab.cpp
		)
ENDIF(USE_MATIO_API)
	
IF (USE_OPENSIM_API)
		# Solver featuring API integration
		SET(PROJECT_SRCS 
			${PROJECT_SRCS}
			lhpOpModifyOpenSimModelRunIKSimulationAPI.h
			lhpOpModifyOpenSimModelRunIKSimulationAPI.cpp	
			lhpOpModifyOpenSimModelRunIDSimulationAPI.h
			lhpOpModifyOpenSimModelRunIDSimulationAPI.cpp
			lhpOpModifyOpenSimModelRunStaticOptimizationAPI.h
			lhpOpModifyOpenSimModelRunStaticOptimizationAPI.cpp
			lhpOpModifyOpenSimModelCreateCustomJointAPI.h
			lhpOpModifyOpenSimModelCreateCustomJointAPI.cpp
		)
	ELSE (USE_OPENSIM_API)
		# Solver running external executables
		SET(PROJECT_SRCS 
			${PROJECT_SRCS}
			lhpOpModifyOpenSimModelRunIKSimulation.h
			lhpOpModifyOpenSimModelRunIKSimulation.cpp
			lhpOpModifyOpenSimModelRunIDSimulation.h
			lhpOpModifyOpenSimModelRunIDSimulation.cpp
		)
ENDIF (USE_OPENSIM_API)

IF (MAF_USE_ITK)
SET(PROJECT_SRCS  ${PROJECT_SRCS}
	lhpOpCreateSurfaceScalar.cpp
	lhpOpCreateSurfaceScalar.h
  lhpOpExtractGeometry.cpp
  lhpOpExtractGeometry.h
  lhpStructureTensorRecursiveGaussianImageFilter.txx
  lhpStructureTensorRecursiveGaussianImageFilter.h
  lhpSymmetricEigenVectorAnalysisImageFilter.h
)
ENDIF (MAF_USE_ITK)


IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
#IF (MED_USE_BTK)
SET(PROJECT_SRCS  ${PROJECT_SRCS}
	lhpOpImporterC3DBTK.cpp
	lhpOpImporterC3DBTK.h
)
#ENDIF (MED_USE_BTK)
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")

#Add support for VS 2010 Edit & Continue
IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")

#Add OpenMP Support + Support for Parallel Compilation
IF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
ENDIF (${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 11")

# List libraries that are needed by this project.
# SET(PROJECT_LIBS "mafVME" "mafInteraction")

# SET(PROJECT_LIBS ${PROJECT_LIBS} vtkMED medCommon)
IF(LHP_BUILD_DLL)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
  SET(BUILD_SHARED_LIBS 1)
  ADD_DEFINITIONS(-DLHP_OPERATIONS_EXPORTS)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} mafDLL lhpGui lhpVME vtkLHP ${LHPExtLibs_TARGETS})

  IF (LHPExtLibs_BUILD_TARGETS)
    ADD_DEPENDENCIES(${PROJECT_NAME} ${LHPExtLibs_BUILD_TARGETS})
  ENDIF (LHPExtLibs_BUILD_TARGETS)
ELSE(LHP_BUILD_DLL)
  # Create the library.
  ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
ENDIF (LHP_BUILD_DLL)
