/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonematBatch.h,v $
  Language:  C++
  Date:      $Date: 2012-02-07 15:33:17 $
  Version:   $Revision: 1.1.2.4 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpOpBonematBatch_H__
#define __lhpOpBonematBatch_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "mafNode.h"
#include "mafVMEVolumeGray.h"
#include "lhpOperationsDefines.h"
#include <iostream>
//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;


//----------------------------------------------------------------------------
// lhpOpBonematBatch :
//----------------------------------------------------------------------------
/** Batch operation to map multiple CT Volumes properties on corresponding input finite element meshes

The test suite lhpOpBonematBatchTest is available in Testing/Operations dir: at the present time it's not running in 
regression since (TODO) it needs some fix to be executed in release mode. It can be executed in debug mode to validate changes 
to this class code.

Sample configuration file: # is the comment token

# <Configuraton file start>
#--------------------
# Bonemat 1
#--------------------
# Input CDB
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/BV3_test.cdb
# Input VTK
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/volume.vtk
# Input Configuration File
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/Calib_120-180_CT_nuova_correction_Morgan_FATS.conf
# Output Ansys File
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/ansysOutputBatch1.inp
#--------------------
# Bonemat 2
#--------------------
# Input CDB
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/BV3_test.cdb
# Input VTK
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/volume.vtk
# Input Configuration File
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/Calib_120-180_CT_nuova_correction_Morgan_FATS.conf
# Output Ansys File
D:/vapps_merge_target/Branch22/LHPBuilder_Parabuild/Testing/UnitTestsData/lhpOpBonematBatchTest/ansysOutputBatch2.inp
# ...
# <Configuraton file end>
 
*/

class LHP_OPERATIONS_EXPORT lhpOpBonematBatch: public mafOp
{
public:

  lhpOpBonematBatch(wxString label);
  ~lhpOpBonematBatch(); 

  /** Set the batch run configuration file name */
  void SetConfigurationFileName(const char* name);
  const char* GetConfigurationFileName();

  /** Execute the procedure that maps TAC values on the finite element mesh */
  int Execute();

  /** Overridden to always return true */
  bool Accept(mafNode *node);

  /** Builds operation's interface. */
  void OpRun();

  mafOp* Copy();
 
  /** Set the python interpreter that will be used to execute embedded python scripts; 
  to be used if the op is instantiated without gui */
  void SetPythonExeFullPath(mafString pythonExeFullPath) {m_PythonExeFullPath = pythonExeFullPath;};

protected:

  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  void OpStop(int result);

  /** Open bonemat batch run configuration file from gui*/ 
  int OpenConfigurationFileFromGUI();
 
  /** Load bonemat batch runs configuration file parameters */
  int LoadBonematBatchConfigurationFile( const char * configurationFileName, std::vector<std::string> &inCDBVector, std::vector<std::string> &inVTKVector, std::vector<std::string> &inBnematConfVector, std::vector<std::string> &outAnsysInp );

  bool FileExists(std::string filePath);

  std::string GetFilePath(std::string filePath);

  /** Run bonemat single instance: inVTKVolumeFileName is a structured points or rectilinear grid */
  void RunBonemat( mafString &pythonExeFullPath, wxString inCDBFileName, wxString inVTKVolumeFileName, wxString inputBonematConfigurationFileName, wxString outputAnsysFileName );
  int GetLine(FILE *fp, char *buffer);

  /** Bonemat configuration file abs path */
  mafString m_ConfigurationFileName;
  wxString m_ConfigurationFilePath;

  std::vector<std::string> m_inCDBVector;
  std::vector<std::string> m_inVTKVector;
  std::vector<std::string> m_inBonematConfVector;
  std::vector<std::string> m_outAnsysInp;

  mafString m_PythonExeFullPath; //>python  executable
  mafString m_PythonwExeFullPath; //>pythonw  executable

};
#endif
