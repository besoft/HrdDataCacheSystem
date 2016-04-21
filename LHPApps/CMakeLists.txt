# =========================================================================
#   Program:   lhpBuilder2
#   Module:    $RCSfile: CMakeLists.txt,v $
#   Language:  CMake
#   Date:      $Date: 2011-09-12 19:31:51 $
#   Version:   $Revision: 1.1.1.1.2.30 $
#   Authors:   Stefano Perticoni
# ==========================================================================

SET (CMAKE_BACKWARDS_COMPATIBILITY "2.4" CACHE INTERNAL "This project is for CMake 2.4")
CMAKE_MINIMUM_REQUIRED(VERSION 2.4) 

PROJECT (LHP)
  
#-----------------------------------------------------------------------------
# This enables support for Visual Studio: Edit & Continue, OpenMP, and parallel compilation
#-----------------------------------------------------------------------------
IF (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	#Add OpenMP Support
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")

	#Add support for Edit & Continue
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	IF (NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 17.0)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
		SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
	ENDIF (NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 17.0)
ENDIF (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")

#
#-----------------------------------------------------------------------------
# this define the LHP_SUBPROJECT macro
#-----------------------------------------------------------------------------
INCLUDE (${LHP_SOURCE_DIR}/CMakeModules/LHPConfigMacro.cmake)

#-----------------------------------------------------------------------------
# where to place output binaries
#-----------------------------------------------------------------------------
SET (LIBRARY_OUTPUT_PATH ${LHP_BINARY_DIR}/bin/ CACHE INTERNAL
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH ${LHP_BINARY_DIR}/bin/ CACHE INTERNAL 
     "Single output directory for building all executables.")

MARK_AS_ADVANCED (
  LIBRARY_OUTPUT_PATH 
)

#-----------------------------------------------------------------------------
# Options for lhpBuilder building
#-----------------------------------------------------------------------------
# OPTION(BUILD_SHARED_LIBS "Build with shared libraries." 0)
# MARK_AS_ADVANCED (
  # BUILD_SHARED_LIBS 
# )

OPTION(BUILD_TESTING "Build the testing tree" OFF)
OPTION(BUILD_PSLOADER "Build the PSLOADER application" OFF)
OPTION(BUILD_LHPBUILDER "Build the LHPBUILDER application" OFF)
OPTION(BUILD_PSLOADERPLUS "Build the PSLOADERPLUS application" OFF)
OPTION(BUILD_BUILDER "Build the BUILDER application" OFF)
OPTION(BUILD_BONEMAT "Build the BONEMAT application" OFF)
OPTION(BUILD_NMSBUILDER "Build the NMSBUILDER application" OFF)
OPTION(BUILD_IPOSE "Build the IPOSE application" OFF)
OPTION(BUILD_BUILDERM2O "Builder the BuilderM2O application" ON)
OPTION(USE_OPENSIM_API "Use OpenSim API inside LHP components" OFF)
OPTION(USE_MATIO_API "Use Matio API inside LHP components" OFF)

#-----------------------------------------------------------------------------
# find required libraries
#-----------------------------------------------------------------------------

INCLUDE("${PROJECT_SOURCE_DIR}/CMakeModules/FindMAF.cmake")

# find MAF library
IF(MAF_FOUND)
	
  INCLUDE("${MAF_USE_FILE}")

  # Find the Medical library 
  INCLUDE("${PROJECT_SOURCE_DIR}/CMakeModules/FindMED.cmake")

  IF(MED_FOUND)
    INCLUDE("${MED_USE_FILE}")
  ELSE (MED_FOUND)
  	MESSAGE (FATAL_ERROR "MAF-MEDICAL library not found, cannot build application!!")
  ENDIF(MED_FOUND)
  
	IF(MAF_BUILD_MAFDLL)
		OPTION(LHP_BUILD_DLL "Build apps with dlls" OFF)
	ENDIF(MAF_BUILD_MAFDLL)

	IF(LHP_BUILD_DLL)
	  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
	  SET(LHP_BUILD_DLL 1)
	ELSE(LHP_BUILD_DLL)
	  SET(LHP_BUILD_DLL 0)
	ENDIF (LHP_BUILD_DLL)
  
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/iPose/VME/iVMEDefines.h.in ${PROJECT_BINARY_DIR}/iVMEDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/iPose/Operations/iOperationsDefines.h.in ${PROJECT_BINARY_DIR}/iOperationsDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/iPose/GUI/iGuiDefines.h.in ${PROJECT_BINARY_DIR}/iGuiDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/iPose/Common/iCommonDefines.h.in ${PROJECT_BINARY_DIR}/iCommonDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/VME/lhpVMEDefines.h.in ${PROJECT_BINARY_DIR}/lhpVMEDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Common/lhpCommonDefines.h.in ${PROJECT_BINARY_DIR}/lhpCommonDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Operations/lhpOperationsDefines.h.in ${PROJECT_BINARY_DIR}/lhpOperationsDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Gui/lhpGuiDefines.h.in ${PROJECT_BINARY_DIR}/lhpGuiDefines.h @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Core/lhpCoreDefines.h.in ${PROJECT_BINARY_DIR}/lhpCoreDefines.h @ONLY)

  SET(MFL_SOURCE_DIR ${MFL_SOURCE_PATH})
  LHP_SUBPROJECT(LHPExternalLibraries ExternalLibraries)
	IF (LHPExternalLibraries_CONFIG_FOUND)
  	FIND_FILE (USE_LHPExternalLibraries_FILE UseLHPExternalLibraries.cmake "${LHP_BINARY_DIR}/ExternalLibraries") 
	  IF (USE_LHPExternalLibraries_FILE)
	    MESSAGE(STATUS "LHP: External Libraries Configured")
	    SET (LHP_CAN_BUILD 1)
	  ELSE (USE_LHPExternalLibraries_FILE)
	  	MESSAGE(ERROR "Error configuring External Libraries.")
		ENDIF (USE_LHPExternalLibraries_FILE)
	ELSE (LHPExternalLibraries_CONFIG_FOUND)
		MESSAGE(ERROR "Error finding External Libraries.")
	ENDIF (LHPExternalLibraries_CONFIG_FOUND)

	INCLUDE("${USE_LHPExternalLibraries_FILE}")
      

  LINK_DIRECTORIES(${LHP_BINARY_DIR}/bin/Debug)
  LINK_DIRECTORIES(${LHP_BINARY_DIR}/bin/Release)

  # Add single dirs to inclusion path
  INCLUDE_DIRECTORIES(BEFORE ".")
  INCLUDE_DIRECTORIES("${LHP_BINARY_DIR}" "${LHP_SOURCE_DIR}/Application" "${LHP_SOURCE_DIR}/Cache" "${LHP_SOURCE_DIR}/Core" "${LHP_SOURCE_DIR}/Common" "${LHP_SOURCE_DIR}/Gui"
                      "${LHP_SOURCE_DIR}/Operations" "${LHP_SOURCE_DIR}/VME" "${LHP_SOURCE_DIR}/Views" "${LHP_SOURCE_DIR}/vtkLHP" 
                      "${LHP_BINARY_DIR}/vtkLHP"
	"${LHP_SOURCE_DIR}/ExternalLibraries/lp_solve/include"
	"${LHP_SOURCE_DIR}/ExternalLibraries/ProgressiveHullCUDALib/Include"
	"${LHP_SOURCE_DIR}/ExternalLibraries/GPU_Solver/Include"
)  
#BES: two lines above is hot include of lpsolve and vtkCUDAProgressiveHull libraries - to be changed  
                     
  INCLUDE_DIRECTORIES(${MAF_BINARY_PATH}/Libraries/ITK/Sources/InsightToolkit-3.0.1/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src)

##################################################  
# MATIO with HDF5 library integration (USE_MATIO_API set to ON)
##################################################  
# Library should be built with CMake >=2.8.6 with default options from https://github.com/sperticoni/matio-openmeeg.git
# From CMake set MATIO_HEADERS_DIR to YourBuildPath\matio-openmeeg\src\
# and set MATIO_LIB to YourBuildPath\matio-openmeeg\Debug\matio.lib
IF (USE_MATIO_API)
   ADD_DEFINITIONS(-DUSE_MATIO_API)
   SET(MATIO_HEADERS_DIR "MATIO_HEADERS_DIR" CACHE PATH "Directory containing the matio.h file")
   INCLUDE_DIRECTORIES(${MATIO_HEADERS_DIR})
   SET(MATIO_LIB "MATIO_LIB" CACHE FILEPATH "matio.lib library file path")
   LINK_LIBRARIES(${MATIO_LIB})
ENDIF (USE_MATIO_API)
##################################################  

IF (USE_OPENSIM_API)

  ADD_DEFINITIONS(-DUSE_OPENSIM_API)
    
  SET(OPENSIM_INSTALL_DIR $ENV{OPENSIM_HOME} 
		CACHE PATH "Top-level directory of OpenSim install")

  # Location of headers
  SET(SIMTK_HEADERS_DIR ${OPENSIM_INSTALL_DIR}/sdk/include/SimTK/include)
  SET(OPENSIM_HEADERS_DIR ${OPENSIM_INSTALL_DIR}/sdk/include)
  INCLUDE_DIRECTORIES(${SIMTK_HEADERS_DIR} ${OPENSIM_HEADERS_DIR})
  # Libraries and dlls
  SET(OPENSIM_LIBS_DIR ${OPENSIM_INSTALL_DIR}/sdk/lib)
  SET(OPENSIM_DLLS_DIR ${OPENSIM_INSTALL_DIR}/bin)
  LINK_DIRECTORIES(${OPENSIM_LIBS_DIR} ${OPENSIM_DLLS_DIR})

SET(NameSpace "OpenSim_" CACHE STRING "Prefix for simtk lib names, includes trailing '_'. Leave empty to use stock SimTK libraries.")
MARK_AS_ADVANCED(NameSpace)

IF(WIN32)
  SET(PLATFORM_LIBS  pthreadVC2)
ELSE (WIN32)
  SET(NameSpace "")
ENDIF (WIN32)

LINK_LIBRARIES(
	debug osimCommon_d optimized osimCommon
	debug osimSimulation_d optimized osimSimulation
	debug osimAnalyses_d optimized osimAnalyses
	debug osimActuators_d optimized osimActuators
	debug osimTools_d optimized osimTools
	debug ${NameSpace}SimTKcommon_d optimized ${NameSpace}SimTKcommon
	debug ${NameSpace}SimTKmath_d optimized  ${NameSpace}SimTKmath
	debug ${NameSpace}SimTKsimbody_d optimized ${NameSpace}SimTKsimbody
	SimTKlapack
	${PLATFORM_LIBS})
	
ENDIF (USE_OPENSIM_API)
	
  SUBDIRS (Core)
  SUBDIRS (vtkLHP)
  SUBDIRS (Application)
  SUBDIRS (Common)
  SUBDIRS (Gui)
  SUBDIRS (Operations)
  SUBDIRS (Views)
  SUBDIRS (VME)
  SUBDIRS (Cache)
  IF (BUILD_IPOSE)
  SUBDIRS (iPose)
  ENDIF (BUILD_IPOSE)
    
  # Configuring Doxygen documentation  
  
  OPTION(BUILD_DOCUMENTATION "Build the doxygen documentation" OFF)
  
  IF(BUILD_DOCUMENTATION)
      SUBDIRS (Docs)
  ENDIF(BUILD_DOCUMENTATION)
  
  # ADD_SUBDIRECTORY(Aurion/C3DReader/share)
  # INCLUDE("${PROJECT_SOURCE_DIR}/CMakeModules/FindC3DReaderAPI.cmake")

  # IF(C3DReaderAPI_FOUND)
      # INCLUDE(${C3DReaderAPI_USE_FILE})
  # ELSE(C3DReaderAPI_FOUND)
	  # MESSAGE(FATAL_ERROR "Cannot find C3DReaderAPI_PATH")
  # ENDIF(C3DReaderAPI_FOUND)

  # Export Build settings and library dependencies
  INCLUDE(CMakeExportBuildSettings)
  
  CMAKE_EXPORT_BUILD_SETTINGS(
    ${LHP_BINARY_DIR}/LHPBuildSettings.cmake
  )
  
  EXPORT_LIBRARY_DEPENDENCIES(
    ${LHP_BINARY_DIR}/LHPLibraryDepends.cmake
  )

  CONFIGURE_FILE(
    ${LHP_SOURCE_DIR}/CMakeInput/LHPConfig.cmake.in
    ${LHP_BINARY_DIR}/LHPConfig.cmake
  @ONLY IMMEDIATE
  )
  
  CONFIGURE_FILE(
    ${LHP_SOURCE_DIR}/CMakeInput/LHPUse.cmake.in
    ${LHP_BINARY_DIR}/LHPUse.cmake
  @ONLY IMMEDIATE
  )

  
  
#BES: lpsolve library
	SET(extLib_lpsolve
		lpsolve55.dll
		lpsolve55.lib
	)

	FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
			SET(EXTLIB_TARGET_DIR ${EXECUTABLE_OUTPUT_PATH}/${config})
	  	FILE(MAKE_DIRECTORY ${EXTLIB_TARGET_DIR})
		  FOREACH(file ${extLib_lpsolve})
      	CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/lp_solve/lib/${file} 
                     ${EXTLIB_TARGET_DIR}/${file} COPYONLY IMMEDIATE)		
	  	ENDFOREACH(file)
	ENDFOREACH(config)


#BES: vtkCUDAProgressiveHull library
SET(extLib_HullLib
		ProgressiveHullCUDALib.dll
		ProgressiveHullCUDALib.lib
		ProgressiveHullCUDALib.pdb
		cudart32_40_17.dll
	)

	FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
			SET(EXTLIB_TARGET_DIR ${EXECUTABLE_OUTPUT_PATH}/${config})
	  	FILE(MAKE_DIRECTORY ${EXTLIB_TARGET_DIR})
		  FOREACH(file ${extLib_HullLib})
      	CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/ProgressiveHullCUDAlib/lib/${file} 
                     ${EXTLIB_TARGET_DIR}/${file} COPYONLY IMMEDIATE)		
	  	ENDFOREACH(file)
	ENDFOREACH(config)

	#Debug versions, if VS 2010 is used
	IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")	#Libraries were built using VS 2010
  	CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/ProgressiveHullCUDALib/Lib/ProgressiveHullCUDALibd.dll 
		${EXECUTABLE_OUTPUT_PATH}/Debug/ProgressiveHullCUDALibd.dll COPYONLY IMMEDIATE)
	  CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/ProgressiveHullCUDALib/Lib/ProgressiveHullCUDALibd.lib 
		${EXECUTABLE_OUTPUT_PATH}/Debug/ProgressiveHullCUDALibd.lib COPYONLY IMMEDIATE)
	  CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/ProgressiveHullCUDALib/Lib/ProgressiveHullCUDALibd.pdb 
		${EXECUTABLE_OUTPUT_PATH}/Debug/ProgressiveHullCUDALibd.pdb COPYONLY IMMEDIATE)
	ENDIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")

#BES: GPU_Solver library
SET(extLib_GPUSolverLib
		GPU_Solver.dll
		GPU_Solver.lib
		GPU_Solver.pdb
		cutil32.dll
		cublas32_42_9.dll
		cudart32_42_9.dll
	)

	FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
			SET(EXTLIB_TARGET_DIR ${EXECUTABLE_OUTPUT_PATH}/${config})
	  	FILE(MAKE_DIRECTORY ${EXTLIB_TARGET_DIR})
		  FOREACH(file ${extLib_GPUSolverLib})
      	CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/GPU_Solver/lib/${file} 
                     ${EXTLIB_TARGET_DIR}/${file} COPYONLY IMMEDIATE)		
	  	ENDFOREACH(file)
	ENDFOREACH(config)

	#Debug versions, if VS 2010 is used
	IF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")	#Libraries were built using VS 2010
	  CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/GPU_Solver/lib/GPU_Solverd.dll 
		${EXECUTABLE_OUTPUT_PATH}/Debug/GPU_Solverd.dll COPYONLY IMMEDIATE)
	  CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/GPU_Solver/lib/GPU_Solverd.lib
		${EXECUTABLE_OUTPUT_PATH}/Debug/GPU_Solverd.lib COPYONLY IMMEDIATE)
	  CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/GPU_Solver/lib/GPU_Solverd.pdb 
		${EXECUTABLE_OUTPUT_PATH}/Debug/GPU_Solverd.pdb COPYONLY IMMEDIATE)
	  CONFIGURE_FILE(${LHP_SOURCE_DIR}/ExternalLibraries/GPU_Solver/lib/cutil32D.dll 
		${EXECUTABLE_OUTPUT_PATH}/Debug/cutil32D.dll COPYONLY IMMEDIATE)
	ENDIF(${CMAKE_GENERATOR} STREQUAL "Visual Studio 10")


  #-----------------------------------------------------------------------------
  # Testing stuff
  #-----------------------------------------------------------------------------
  IF(BUILD_TESTING)
    
    MESSAGE(STATUS "variable LHP_SOURCE_DIR = " ${LHP_SOURCE_DIR})
    FIND_PATH(LHP_DATA_ROOT "TestingDataRoot.txt" "${LHP_SOURCE_DIR}/Testing/UnitTestsData" "${LHP_SOURCE_DIR}/../TestingData" "${LHP_SOURCE_DIR}/../mmData/TestingData" "${LHP_SOURCE_DIR}/../../TestingData"  DOC "Test data repository")
    
    ENABLE_TESTING()
      
    # where will executable tests be written ?
    IF (EXECUTABLE_OUTPUT_PATH)
        SET (CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
    ELSE (EXECUTABLE_OUTPUT_PATH)
        SET (CXX_TEST_PATH .)
    ENDIF (EXECUTABLE_OUTPUT_PATH)
      
    # Testing dirs:
    SUBDIRS (Testing)
    SUBDIRS (VMEUploaderDownloaderRefactor)
    # End Testing dirs:
    
  ENDIF(BUILD_TESTING)
  
ELSE (MAF_FOUND)
  MESSAGE (FATAL_ERROR "MAF library not found, cannot build application!!")
ENDIF (MAF_FOUND)

#-----------------------------------------------------------------------------
# Python and wxPython correct versions check
#-----------------------------------------------------------------------------
SET (PYTHON_REQUIRED_VERSION "2.5.4" CACHE INTERNAL "Required Python Version")
SET(PYTHON_EXE_PATH $INSTDIR/Python25/python.exe CACHE PATH "Python exe abs path")
SET (WXPYTHON_REQUIRED_VERSION "2.8.10.1" CACHE INTERNAL "Required wxPython Version")
# MESSAGE("variable PYTHON_EXE_PATH = " ${PYTHON_EXE_PATH})

EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/Installer" ARGS "-V" OUTPUT_VARIABLE O RETURN_VALUE R)

IF(${O} MATCHES ${PYTHON_REQUIRED_VERSION})

    MESSAGE(STATUS "Found Python ${PYTHON_REQUIRED_VERSION}")
    
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/Installer" ARGS "wxPythonExistenceCheck.py" OUTPUT_VARIABLE O RETURN_VALUE R)        
    
    IF(${O} MATCHES ${WXPYTHON_REQUIRED_VERSION})
       
       MESSAGE(STATUS "Found ${WXPYTHON_REQUIRED_VERSION}")
        
    ELSE(${O} MATCHES ${WXPYTHON_REQUIRED_VERSION})
       
        MESSAGE("Problems finding wxPython ${WXPYTHON_REQUIRED_VERSION}
Please go to: 

https://www.physiomespace.com/wikis/GetTheSoftware

and follow the Python and wxPython installation istructions for PSLoader slim installer
in order to have a working developer environment.
You can skip this step but application network facilities will not work. 
")
        
   ENDIF(${O} MATCHES ${WXPYTHON_REQUIRED_VERSION})
    
ELSE(${O} MATCHES ${PYTHON_REQUIRED_VERSION})
    
    MESSAGE("Problems finding Python ${PYTHON_REQUIRED_VERSION}
Please go to: 

https://www.physiomespace.com/wikis/GetTheSoftware

and follow the Python and wxPython installation istructions for PSLoader slim installer
in order to have a working developer environment.
You can skip this step but application network facilities will not work.
")
    
ENDIF(${O} MATCHES ${PYTHON_REQUIRED_VERSION})

#-----------------------------------------------------------------------------
# VMEUploaderDownloader Refactor tagsToBeRemovedFromUploadedVMETagArray download stuff
#-----------------------------------------------------------------------------
OPTION(DOWNLOAD_TAGS_TO_BE_REMOVED_FROM_UPLOADED_RESOURCES_LIST "tagsToBeRemovedFromUploadedVMETagArray.txt download in VMEUploaderDownloaderRefactor dir" ON)

IF(DOWNLOAD_TAGS_TO_BE_REMOVED_FROM_UPLOADED_RESOURCES_LIST)
   
    SET(ERROR 0)    

    SET(REMOTE_HOST "https://www.biomedtown.org" CACHE INTERNAL "LHPBuilder dictionary host")
    SET(TAGS_TO_BE_REMOVED_FILE_SELECTOR "/biomed_town/LHDL/users/swclient/tagsToBeRemovedFromUploadedVMETagArray/tagsToBeRemovedFromUploadedVMETagArray" CACHE INTERNAL "LHPBuilder TagsToBeRemoved remote file selector")
    SET(OUTPUT_FILE_NAME "tagsToBeRemovedFromUploadedVMETagArray.txt" CACHE INTERNAL "Tags to be removed file name on disk after download")
    
    SET(RET -1)
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloaderRefactor" ARGS "HttpsFileDownload.py ${REMOTE_HOST} ${TAGS_TO_BE_REMOVED_FILE_SELECTOR} ${OUTPUT_FILE_NAME}"  RETURN_VALUE RET)
    
    IF(${RET} MATCHES "0")
        MESSAGE(STATUS "tagsToBeRemovedFromUploadedVMETagArray.txt correctly downloaded!")
    ELSE(${RET} MATCHES "0")
        MESSAGE("PROBLEMS DOWNLOADING tagsToBeRemovedFromUploadedVMETagArray.txt! PLEASE CHECK IT!")
        SET(ERROR 1)
    ENDIF(${RET} MATCHES "0")

    IF(${ERROR} MATCHES "0")
        MESSAGE(STATUS "tagsToBeRemovedFromUploadedVMETagArray.txt downloaded correctly!")
    ELSE(${ERROR} MATCHES "0")
        MESSAGE("Problems downloading tagsToBeRemovedFromUploadedVMETagArray.txt!

        
In order for the download to work you need to:
1: Be connected to the Internet
2: Have python 2.5 installed in C:\Python25

If You don't want to this message to appear you can set DOWNLOAD_TAGS_TO_BE_REMOVED_FROM_UPLOADED_RESOURCES_LIST
to false. Please notice that some functions ol LHPBuilder will not be available
with this setting.
")
#         MESSAGE(${OUTPUT_VALUE})
    ENDIF(${ERROR} MATCHES "0")
    
ELSE(DOWNLOAD_TAGS_TO_BE_REMOVED_FROM_UPLOADED_RESOURCES_LIST)
         MESSAGE("You have choosed not to download tagsToBeRemovedFromUploadedVMETagArray.txt for the VMEUploaderDownloaderRefactor directory!
Please notice that some operations in the Operations->Devel submenu will not
work with this setting.")

ENDIF(DOWNLOAD_TAGS_TO_BE_REMOVED_FROM_UPLOADED_RESOURCES_LIST)


#-----------------------------------------------------------------------------
# VMEUploaderDownloader Refactor dictionary download stuff
#-----------------------------------------------------------------------------
OPTION(DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR "Generate XML lhpBuilder dictionary in VMEUploaderDownloaderRefactor dir" ON)

IF(DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR)
   
   # MASTER DICTIONARY
   
    SET(ERROR 0)    
    #  MESSAGE("variable DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR = " ${DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR})
    SET(LHPBUILDER_DICTIONARY_HOST "https://www.biomedtown.org" CACHE INTERNAL "LHPBuilder dictionary host")
    SET(LHPBUILDER_MASTER_DICTIONARY_SELECTOR "/biomed_town/LHDL/users/swclient/dictionaries/LHDL_dictionary" CACHE INTERNAL "LHPBuilder dictionary selector")
    SET(LHPBUILDER_XML_MASTER_DICTIONARY "lhpXMLDictionary" CACHE INTERNAL "LHPBuilder xml master dictionary")
    
#     MESSAGE("variable LHP_SOURCE_DIR = " ${LHP_SOURCE_DIR})

#     This line contains OUTPUT_VARIABLE OUTPUT_VALUE 
#     EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloader" ARGS "lhpCSVDictionaryDownloader.py ${LHPBUILDER_DICTIONARY_HOST} ${LHPBUILDER_DICTIONARY_SELECTOR} ${LHP_SOURCE_DIR}/${LHPBUILDER_XML_DICTIONARY}" OUTPUT_VARIABLE OUTPUT_VALUE RETURN_VALUE RETURN)
    
    SET(RET -1)
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloaderRefactor" ARGS "lhpCSVDictionaryDownloader.py ${LHPBUILDER_DICTIONARY_HOST} ${LHPBUILDER_MASTER_DICTIONARY_SELECTOR} ${LHPBUILDER_XML_MASTER_DICTIONARY}"  RETURN_VALUE RET)
    
    IF(${RET} MATCHES "0")
        MESSAGE(STATUS "XML master dictionary generated correctly!")
    ELSE(${RET} MATCHES "0")
        MESSAGE("PROBLEMS WITH MASTER DICTIONARY! PLEASE CHECK IT!")
        SET(ERROR 1)
    ENDIF(${RET} MATCHES "0")
#     MESSAGE("variable LHPBUILDER_XML_DICTIONARY = " ${LHPBUILDER_XML_DICTIONARY})
#     MESSAGE("variable  OUTPUT = " ${OUTPUT_VALUE})
#     MESSAGE("variable  RET = " ${RET})

    # DICOM DICTIONARY
       
    SET(LHPBUILDER_DICOM_DICTIONARY_SELECTOR "/biomed_town/LHDL/users/swclient/dictionaries/DicomSource" CACHE INTERNAL "LHPBuilder dictionary selector")
    SET(LHPBUILDER_XML_DICOM_DICTIONARY "lhpXMLDicomSourceSubdictionary" CACHE INTERNAL "LHPBuilder xml dicom dictionary")
        
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloaderRefactor" ARGS "lhpCSVDictionaryDownloader.py ${LHPBUILDER_DICTIONARY_HOST} ${LHPBUILDER_DICOM_DICTIONARY_SELECTOR} ${LHPBUILDER_XML_DICOM_DICTIONARY}"  RETURN_VALUE RET)
    
    IF(${RET} MATCHES "0")
        MESSAGE(STATUS "XML dicom subdictionary generated correctly!")
    ELSE(${RET} MATCHES "0")
        MESSAGE("PROBLEMS WITH DICOM DICTIONARY! PLEASE CHECK IT!")
        SET(ERROR 1)
    ENDIF(${RET} MATCHES "0")

    # MOTIONANALYSIS DICTIONARY
    
    SET(LHPBUILDER_MOTIONANALYSIS_DICTIONARY_SELECTOR "/biomed_town/LHDL/users/swclient/dictionaries/MASource" CACHE INTERNAL "LHPBuilder dictionary selector")
    SET(LHPBUILDER_XML_MOTIONANALYSIS_DICTIONARY "lhpXMLMotionAnalysisSourceSubdictionary" CACHE INTERNAL "LHPBuilder xml motion analysis dictionary")
    
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloaderRefactor" ARGS "lhpCSVDictionaryDownloader.py ${LHPBUILDER_DICTIONARY_HOST} ${LHPBUILDER_MOTIONANALYSIS_DICTIONARY_SELECTOR} ${LHPBUILDER_XML_MOTIONANALYSIS_DICTIONARY}"  RETURN_VALUE RET)
    
    IF(${RET} MATCHES "0")
        MESSAGE(STATUS "XML motion analysis dictionary generated correctly!")
    ELSE(${RET} MATCHES "0")
        MESSAGE("PROBLEMS WITH MOTION ANALYSIS DICTIONARY! PLEASE CHECK IT!")
        SET(ERROR 1)
    ENDIF(${RET} MATCHES "0")

    # FA DICTIONARY
    
    SET(LHPBUILDER_FA_DICTIONARY_SELECTOR "/biomed_town/LHDL/users/swclient/dictionaries/FA_onto" CACHE INTERNAL "LHPBuilder dictionary selector")
    SET(LHPBUILDER_XML_FA_DICTIONARY "lhpXMLFASourceSubdictionary" CACHE INTERNAL "LHPBuilder xml FA dictionary")
    
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloaderRefactor" ARGS "lhpFAOntoCSVDictionaryDownloader.py ${LHPBUILDER_DICTIONARY_HOST} ${LHPBUILDER_FA_DICTIONARY_SELECTOR} ${LHPBUILDER_XML_FA_DICTIONARY}"  RETURN_VALUE RET)

    
    IF(${RET} MATCHES "0")
        MESSAGE(STATUS "XML FA dictionary generated correctly!")
    ELSE(${RET} MATCHES "0")
        MESSAGE("PROBLEMS WITH FA DICTIONARY! PLEASE CHECK IT!")
        SET(ERROR 1)
    ENDIF(${RET} MATCHES "0")

    # MICROCT DICTIONARY
    
    SET(LHPBUILDER_MICROCT_DICTIONARY_SELECTOR "/biomed_town/LHDL/users/swclient/dictionaries/MicroCTSource" CACHE INTERNAL "LHPBuilder dictionary selector")
    SET(LHPBUILDER_XML_MICROCT_DICTIONARY "lhpXMLMicroCTSourceSubdictionary" CACHE INTERNAL "LHPBuilder xml MicroCT dictionary")
    
    EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/VMEUploaderDownloaderRefactor" ARGS "lhpCSVDictionaryDownloader.py ${LHPBUILDER_DICTIONARY_HOST} ${LHPBUILDER_MICROCT_DICTIONARY_SELECTOR} ${LHPBUILDER_XML_MICROCT_DICTIONARY}"  RETURN_VALUE RET)

    
    IF(${RET} MATCHES "0")
        MESSAGE(STATUS "XML MicroCT dictionary generated correctly!")
    ELSE(${RET} MATCHES "0")
        MESSAGE(STATUS "PROBLEMS WITH MICROCT DICTIONARY! PLEASE CHECK IT!")
        SET(ERROR 1)
    ENDIF(${RET} MATCHES "0")
        
    IF(${ERROR} MATCHES "0")
        MESSAGE(STATUS "All XML dictionaries generated correctly!")
    ELSE(${ERROR} MATCHES "0")
        MESSAGE("Problems generating XML dictionaries!

        
In order for the generation to work you need to:
1: Be connected to the Internet
2: Have python 2.5 installed in C:\Python25

If You don't want to this message to appear you can set DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR
to false. Please notice that some functions ol LHPBuilder will not be available
with this setting.
")
#         MESSAGE(${OUTPUT_VALUE})
    ENDIF(${ERROR} MATCHES "0")
    
ELSE(DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR)
         MESSAGE("You have choosed not to generate XML dictionaries for the VMEUploaderDownloaderRefactor directory!
Please notice that some operations in the Operations->Devel submenu will not
work with this setting.")
#   
ENDIF(DOWNLOAD_DICTIONARIES_FOR_VMEUPDOWN_REFACTOR)

#-----------------------------------------------------------------------------
# Security stuff.  
#-----------------------------------------------------------------------------
# 
# COMMENTED FOR THE MOMENT............
#
# EXEC_PROGRAM(${PYTHON_EXE_PATH} "${LHP_SOURCE_DIR}/Security/keyczar" ARGS "CheckKeyczarDependencies.py"  RETURN_VALUE RET)
# 
# # MESSAGE("variable RET = " ${RET})
# 
# IF(${RET} MATCHES "0")
#     MESSAGE(STATUS "Check Keyczar Dependencies successfull!")
#     SET(ERROR 0)
# ELSE(${RET} MATCHES "0")
#     SET(ERROR 1)
# ENDIF(${RET} MATCHES "0")
#     
# # MESSAGE("variable ERROR = " ${ERROR})
# 
# IF(${ERROR} MATCHES "0")
#     MESSAGE(STATUS "Check Keyczar Dependencies successfull!")
# ELSE(${ERROR} MATCHES "0")
#     MESSAGE("Problems checking Keyczar Dependencies...
#     
# In order to install the security components you have to:
# 1: Be connected to the Internet; this is required to download additional components
# 2: Have python 2.5 installed in C:\Python25
# 4: Your user account should be able to write in python2.5 directory
# 
# ")

# MESSAGE(${OUTPUT_VALUE})
# ENDIF(${ERROR} MATCHES "0")

#-----------------------------------------------------------------------------
# LHPBUILDER deploy stuff.  
#-----------------------------------------------------------------------------

# This is specific for NSIS Nullsoft Installer that runs currently under Windows only and Parabuild.
# An LHPBUILDER_VERSION variable is created to gather versioning information
IF (WIN32)
    
  # installer useful values
  SET(BUILDERM2O_EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/Release/" CACHE PATH "Source directory for BuilderM2O executable. This is used by the installer to find the executable.")
  SET(LHPBUILDER_EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/Release/" CACHE PATH "Source directory for lhpBuilder executable. This is used by the installer to find the executable.")
  SET(IPOSE_EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/Debug/" CACHE PATH "Source directory for iPose executable. This is used by the installer to find the executable.")
  
  STRING(REPLACE "/" "\\" LHPBUILDER_EXECUTABLE_OUTPUT_PATH_WIN ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH})
  SET(LHPBUILDER_EXECUTABLE_OUTPUT_PATH_WIN ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH_WIN} CACHE INTERNAL "get the executable from this directory")
  
  STRING(REPLACE "/" "\\" BUILDERM2O_EXECUTABLE_OUTPUT_PATH_WIN ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH})
  SET(BUILDERM2O_EXECUTABLE_OUTPUT_PATH ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH_WIN} CACHE INTERNAL "get the executable from this directory")
  
  # MESSAGE("variable value = " ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH})
  # MESSAGE("variable value = " ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH_WIN})
   
  # this values are set by hand when major or minor is released
  SET(LHPBUILDER_MAJOR_VERSION 1 CACHE INTERNAL "major version")
  SET(LHPBUILDER_MINOR_VERSION 2 CACHE INTERNAL "minor version")
  
  SET(IPOSE_MAJOR_VERSION 0 CACHE INTERNAL "major version")
  SET(IPOSE_MINOR_VERSION 1 CACHE INTERNAL "minor version")
  
  SET(BUILDERM2O_MAJOR_VERSION 0 CACHE INTERNAL "major version")
  SET(BUILDERM2O_MINOR_VERSION 3 CACHE INTERNAL "minor version")
  
  # test for environment variable reading
  # MESSAG# "$ENV{PATH}")
  # MESSAG# "$ENV{PIPPO}") 
  
  # automatic values from parabuild created temporary environment variable
  SET(BUILDERM2O_BUILD_TIMESTAMP $ENV{PARABUILD_BUILD_TIMESTAMP} CACHE INTERNAL "build timestamp")
  SET(BUILDERM2O_BUILD_NUMBER $ENV{BUILD_NUMBER} CACHE INTERNAL "build number")
  
  SET(LHPBUILDER_BUILD_TIMESTAMP $ENV{PARABUILD_BUILD_TIMESTAMP} CACHE INTERNAL "build timestamp")
  SET(LHPBUILDER_BUILD_NUMBER $ENV{PARABUILD_BUILD_NUMBER} CACHE INTERNAL "build number")
  
  SET(IPOSE_BUILD_TIMESTAMP $ENV{PARABUILD_BUILD_TIMESTAMP} CACHE INTERNAL "build timestamp")
  SET(IPOSE_BUILD_NUMBER $ENV{PARABUILD_BUILD_NUMBER} CACHE INTERNAL "build number")
  
   # create the main version variable:
  SET(BUILDERM2O_VERSION "${BUILDERM2O_MAJOR_VERSION}_${BUILDERM2O_MINOR_VERSION}_BUILD_${BUILDERM2O_BUILD_NUMBER}" CACHE INTERNAL "big version identifier concatenating other identifier")
  
  # create the main version variable:
  SET(LHPBUILDER_VERSION "${LHPBUILDER_MAJOR_VERSION}_${LHPBUILDER_MINOR_VERSION}_TS_${LHPBUILDER_BUILD_TIMESTAMP}_BUILD_${LHPBUILDER_BUILD_NUMBER}" CACHE INTERNAL "big version identifier concatenating other identifier")
  
  # create the bonemat common version variable:
  SET(BONEMAT_COMMON_VERSION 2)
	
  # create the main version variable:
  SET(IPOSE_VERSION "${IPOSE_MAJOR_VERSION}_${IPOSE_MINOR_VERSION}_TS_${IPOSE_BUILD_TIMESTAMP}_BUILD_${IPOSE_BUILD_NUMBER}" CACHE INTERNAL "big version identifier concatenating other identifier")
  
  # MESSAGE("variable value = " ${LHPBUILDER_VERSION})
    
  # config installer file
  # set the installer directory variable
  SET(BUILDERM2O_INSTALLER_DIR ${PROJECT_SOURCE_DIR}/Installer)
  # convert it to Windows format
  STRING (REPLACE "/" "\\" BUILDERM2O_INSTALLER_DIR_WIN ${BUILDERM2O_INSTALLER_DIR} )
  # MESSAGE("variable BUILDERM2O_INSTALLER_DIR_WIN = " ${BUILDERM2O_INSTALLER_DIR_WIN})	
	
  # config installer file
  # set the installer directory variable
  SET(LHPBUILDER_INSTALLER_DIR ${PROJECT_SOURCE_DIR}/Installer)
  # convert it to Windows format
  STRING (REPLACE "/" "\\" LHPBUILDER_INSTALLER_DIR_WIN ${LHPBUILDER_INSTALLER_DIR} )
  # MESSAGE("variable LHPBUILDER_INSTALLER_DIR_WIN = " ${LHPBUILDER_INSTALLER_DIR_WIN})
  
  # config installer file
  # set the installer directory variable
  SET(IPOSE_INSTALLER_DIR ${PROJECT_SOURCE_DIR}/Installer)
  # convert it to Windows format
  STRING (REPLACE "/" "\\" IPOSE_INSTALLER_DIR_WIN ${IPOSE_INSTALLER_DIR} )
  
  # installer executable will be written to this directory
  SET(BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH} CACHE PATH "the installer executable will be written to this directory ")
  # convert it to Windows format  
  STRING(REPLACE "/" "\\" BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN ${BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR})
  SET(BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN ${BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN} CACHE INTERNAL "get the executable  from this directory")
  # MESSAGE("variable BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN = " ${BUILDERM2O_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN})
  
  # installer executable will be written to this directory
  SET(LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH} CACHE PATH "the installer executable will be written to this directory ")
  # convert it to Windows format  
  STRING(REPLACE "/" "\\" LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN ${LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR})
  SET(LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN ${LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN} CACHE INTERNAL "get the executable  from this directory")
  # MESSAGE("variable LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN = " ${LHPBUILDER_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN})
  
  # installer executable will be written to this directory
  SET(IPOSE_INSTALLER_EXECUTABLE_OUTPUT_DIR ${IPOSE_EXECUTABLE_OUTPUT_PATH} CACHE PATH "the installer executable will be written to this directory ")
  # convert it to Windows format  
  STRING(REPLACE "/" "\\" IPOSE_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN ${IPOSE_INSTALLER_EXECUTABLE_OUTPUT_DIR})
  SET(IPOSE_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN ${IPOSE_INSTALLER_EXECUTABLE_OUTPUT_DIR_WIN} CACHE INTERNAL "get the executable  from this directory")
    
  # Build the Fat Installer ie the installer containing python and wxPython distributions 
  SET(BUILD_FAT_INSTALLER "ON" CACHE BOOL "Build the fat installer containing python and wxPython distributions")
  
  FIND_FILE( VISUAL_STUDIO_RUNTIME vcredist_x86.exe )
  STRING(REPLACE "/" "\\" VISUAL_STUDIO_RUNTIME ${VISUAL_STUDIO_RUNTIME})
  
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/PythonAndWxPythonInstallation.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/PythonAndWxPythonInstallation.nsi @ONLY)
  
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/ValidateStringWithCustomCriteria.nsi  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/ValidateStringWithCustomCriteria.nsi @ONLY)

  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/NotepadPlusPlusInstallation.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/NotepadPlusPlusInstallation.nsi @ONLY)
  
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/DeployShaders.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/DeployShaders.nsi @ONLY)
  
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/DeployGPUSolverLibs.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/DeployGPUSolverLibs.nsi @ONLY)
  
  
   CONFIGURE_FILE(${BUILDERM2O_INSTALLER_DIR}/PythonAndWxPythonInstallation.nsi.in  ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH}/PythonAndWxPythonInstallation.nsi @ONLY)
  
  CONFIGURE_FILE(${BUILDERM2O_INSTALLER_DIR}/ValidateStringWithCustomCriteria.nsi  ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH}/ValidateStringWithCustomCriteria.nsi @ONLY)

  CONFIGURE_FILE(${BUILDERM2O_INSTALLER_DIR}/NotepadPlusPlusInstallation.nsi.in  ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH}/NotepadPlusPlusInstallation.nsi @ONLY)
  
  CONFIGURE_FILE(${BUILDERM2O_INSTALLER_DIR}/DeployShaders.nsi.in  ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH}/DeployShaders.nsi @ONLY)
  
  CONFIGURE_FILE(${BUILDERM2O_INSTALLER_DIR}/DeployGPUSolverLibs.nsi.in  ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH}/DeployGPUSolverLibs.nsi @ONLY)
  
  # BuilderM2O
  IF (BUILD_BUILDERM2O)
  CONFIGURE_FILE(${BUILDERM2O_INSTALLER_DIR}/BuilderM2OInstallerScript.nsi.in  ${BUILDERM2O_EXECUTABLE_OUTPUT_PATH}/BuilderM2OInstallerScript.nsi @ONLY)
  ENDIF (BUILD_BUILDERM2O)
  
  # lhpBuilder
  IF (BUILD_LHPBUILDER)
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/LHPBuilderInstallerScript.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/LHPBuilderInstallerScript.nsi @ONLY)
  ENDIF (BUILD_LHPBUILDER)
  
  # psLoader
  IF (BUILD_PSLOADER)
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/psLoaderInstallerScript.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/psLoaderInstallerScript.nsi @ONLY)
  ENDIF (BUILD_PSLOADER)
	  
    
  # psLoaderPlus
  IF (BUILD_PSLOADERPLUS)
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/psLoaderPlusInstallerScript.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/psLoaderPlusInstallerScript.nsi @ONLY)
  ENDIF (BUILD_PSLOADERPLUS)

  # Builder
  IF (BUILD_BUILDER)
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/BuilderInstallerScript.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/BuilderInstallerScript.nsi @ONLY)
  ENDIF (BUILD_BUILDER)

  # Bonemat
  IF (BUILD_BONEMAT)
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/BonematInstallerScript.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/BonematInstallerScript.nsi @ONLY)
  ENDIF (BUILD_BONEMAT)
  
  # nmsBuilder
  IF (BUILD_NMSBUILDER)
  CONFIGURE_FILE(${LHPBUILDER_INSTALLER_DIR}/NMSBuilderInstallerScript.nsi.in  ${LHPBUILDER_EXECUTABLE_OUTPUT_PATH}/NMSBuilderInstallerScript.nsi @ONLY)
  ENDIF (BUILD_NMSBUILDER)
  
  # IPOSE
  IF (BUILD_IPOSE)
  CONFIGURE_FILE(${IPOSE_INSTALLER_DIR}/iposeInstallerScript.nsi.in  ${IPOSE_EXECUTABLE_OUTPUT_PATH}/iposeInstallerScript.nsi @ONLY)
  ENDIF (BUILD_IPOSE)
  
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE " )
  
ENDIF (WIN32)     

# Create the header to configure the classes.
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/lhpDefines.h.in
             ${PROJECT_BINARY_DIR}/lhpDefines.h
             @ONLY)

add_subdirectory(qa)          
