#
# Program:   MULTIMOD APPLICATION FRAMEWORK (MAF)
# Module:    $RCSfile: FindMED.cmake,v $
# Language:  CMake 1.2
# Date:      $Date: 2009-05-19 14:29:52 $
# Version:   $Revision: 1.1 $
#
# Description:
# This module finds the location of MED include and library paths 
#

  MESSAGE (STATUS "Find: Searching for MED.")
  # If not built within MED project try standard places
  FIND_PATH(MED_BINARY_PATH MEDConfig.cmake
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]
      [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]
      )
    
  IF(MED_BINARY_PATH)
    SET (MED_FOUND 1)
    FIND_FILE(MED_USE_FILE MEDUse.cmake ${MED_BINARY_PATH})
    INCLUDE(${MED_BINARY_PATH}/MEDConfig.cmake)
  ENDIF(MED_BINARY_PATH)
    
#MARK_AS_ADVANCED (
#  MED_BINARY_PATH
#  MED_FOUND
#  )

#MESSAGE("FindMED: EOF")
