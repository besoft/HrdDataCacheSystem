#
# Program:   @neufuse
# Module:    $RCSfile: FindC3DReaderAPI.cmake,v $
# Language:  CMake 1.2
# Date:      $Date: 2009-05-19 14:29:52 $
# Version:   $Revision: 1.1 $
#
# Description:
# This module finds the location of Geo include and library paths 
#

  MESSAGE (STATUS "Find: Searching for Aurion API.")
 
  FIND_PATH(C3DReaderAPI_PATH C3DReaderAPIConfig.cmake
	"${PROJECT_SOURCE_DIR}/Aurion/C3DReader"
      )
    
  IF(C3DReaderAPI_PATH)
    SET (C3DReaderAPI_FOUND 1)
    INCLUDE(${C3DReaderAPI_PATH}/C3DReaderAPIConfig.cmake)
  ENDIF(C3DReaderAPI_PATH)