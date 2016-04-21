/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpBonematBatch.cpp,v $
Language:  C++
Date:      $Date: 2012-02-24 15:18:49 $
Version:   $Revision: 1.1.2.7 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpBonematBatch.h"
#include "lhpProceduralElements.h"

#include "wx/busyinfo.h"
#include "wx/FileName.h"

#include "mafGUI.h"

#include "mafDecl.h"
#include "mafVMERoot.h"
#include "mafVMEMesh.h"
#include "mafString.h"
#include "mafAbsMatrixPipe.h"

#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "vtkMAFSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkImageData.h"
#include "vtkDoubleArray.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkIntArray.h"
#include "vtkFloatArray.h"
#include "vtkTransformFilter.h"
#include "vtkTransform.h"
#include "lhpOpBonemat.h"
#include "lhpOpExporterAnsysInputFile.h"
#include "lhpOpImporterAnsysCDBFile.h"
#include "medOpImporterVTK.h"
#include "lhpBuilderDecl.h"
#include "lhpOpImporterAnsysInputFile.h"
#include "lhpOpExporterAnsysCDBFile.h"
#include "mafOpExporterVTK.h"

//----------------------------------------------------------------------------
lhpOpBonematBatch::lhpOpBonematBatch(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = false;
  m_ConfigurationFileName = "";

  m_PythonExeFullPath = "python.exe_UNDEFINED";
  m_PythonwExeFullPath = "pythonw.exe_UNDEFINED";
}
//----------------------------------------------------------------------------
lhpOpBonematBatch::~lhpOpBonematBatch()
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
bool lhpOpBonematBatch::Accept(mafNode *node)
//----------------------------------------------------------------------------
{ 
  return true;
}
//----------------------------------------------------------------------------
mafOp* lhpOpBonematBatch::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpBonematBatch *cp = new lhpOpBonematBatch(m_Label);
  return cp;
}

//----------------------------------------------------------------------------
void lhpOpBonematBatch::OpRun()   
//----------------------------------------------------------------------------
{
  // get python interpreters
  mafEvent eventGetPythonExe;
  eventGetPythonExe.SetSender(this);
  eventGetPythonExe.SetId(ID_REQUEST_PYTHON_EXE_INTERPRETER);
  mafEventMacro(eventGetPythonExe);

  if(eventGetPythonExe.GetString())
  {
    m_PythonExeFullPath.Erase(0);
    m_PythonExeFullPath = eventGetPythonExe.GetString()->GetCStr();
    m_PythonExeFullPath.Append(" ");
  }

  mafEvent eventGetPythonwExe;
  eventGetPythonwExe.SetSender(this);
  eventGetPythonwExe.SetId(ID_REQUEST_PYTHONW_EXE_INTERPRETER);
  mafEventMacro(eventGetPythonwExe);

  if(eventGetPythonwExe.GetString())
  {
    m_PythonwExeFullPath.Erase(0);
    m_PythonwExeFullPath = eventGetPythonwExe.GetString()->GetCStr();
    m_PythonwExeFullPath.Append(" ");
  }

  if(OpenConfigurationFileFromGUI() == MAF_OK)
  {
    Execute();
    OpStop(OP_RUN_OK);
  }
  else
  {
    OpStop(OP_RUN_CANCEL);
  }
}
//----------------------------------------------------------------------------
void lhpOpBonematBatch::OpStop(int result)
//----------------------------------------------------------------------------
{
  mafEventMacro(mafEvent(this,result));        
}

//----------------------------------------------------------------------------
const char* lhpOpBonematBatch::GetConfigurationFileName()
//----------------------------------------------------------------------------
{
  return m_ConfigurationFileName.GetCStr();
}
//----------------------------------------------------------------------------
void lhpOpBonematBatch::SetConfigurationFileName(const char* name)
//----------------------------------------------------------------------------
{
  m_ConfigurationFileName = name;  
}

//----------------------------------------------------------------------------
int lhpOpBonematBatch::Execute()
//----------------------------------------------------------------------------
{
  int res = MAF_ERROR;

  // DEBUG
  std::ostringstream stringStream;
  stringStream << "Loading configuration file: " << m_ConfigurationFileName << std::endl;
  mafLogMessage(stringStream.str().c_str());

  int result = MAF_ERROR;
  result = LoadBonematBatchConfigurationFile(m_ConfigurationFileName, m_inCDBVector, m_inVTKVector, m_inBonematConfVector, m_outAnsysInp );

  if (result == MAF_ERROR)
  {
	 return result;
  }

  mafLogMessage("done!");
  mafLogMessage("");

  int numBonematRuns = m_inCDBVector.size(); 

  long progress = 0;
  //mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));

  wxBusyInfo *waitProgress;

  for (int i = 0; i < numBonematRuns; i++) 
  {
    long progress = (int)(i * 100 / numBonematRuns);
    //mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE, progress));

    // DEBUG
    std::ostringstream stringStream;
    stringStream << std::endl;
    stringStream << "-----------------" << std::endl;
    stringStream << "Bonemat Run " << i << std::endl;
    stringStream << "-----------------" << std::endl;
    stringStream << "Input CDB: " << m_inCDBVector[i] << std::endl;
    stringStream << "Input VTK: " << m_inVTKVector[i] << std::endl;
    stringStream << "Input Bonemat Configuration File: " << m_inBonematConfVector[i] << std::endl;
	  stringStream << "-----------------" << std::endl;
	  stringStream << "Running....." << std::endl;
    mafLogMessage(stringStream.str().c_str());  
    
    if(!m_TestMode)
    {
      char mess[254];
      sprintf_s(mess, "          BONEMAT BATCH  \n\n\n\n\n\n\n\nPlease wait Bonemat Run %d of %d", (i+1), numBonematRuns);
   
      waitProgress = new wxBusyInfo(mess);
    }

    this->RunBonemat(m_PythonExeFullPath, m_inCDBVector[i].c_str() , m_inVTKVector[i].c_str() , m_inBonematConfVector[i].c_str() , m_outAnsysInp[i].c_str());
    
    std::ostringstream stringStream2;
	  stringStream2 << std::endl;
	  stringStream2 << "-----------------" << std::endl;
    stringStream2 << "Finished run: " << i << std::endl;
	  assert(wxFileExists(m_outAnsysInp[i].c_str()));
	  stringStream2 << "Output Ansys Inp file generated: " << m_outAnsysInp[i] << std::endl;
	  stringStream2 << "-----------------" << std::endl;
	  stringStream2 << std::endl;
    mafLogMessage(stringStream2.str().c_str());  
    
    //mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));

    if(!m_TestMode)
    {
      delete waitProgress;
    }
  }

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpBonematBatch::LoadBonematBatchConfigurationFile( const char * configurationFileName, std::vector<std::string> &inCDBVector, std::vector<std::string> &inVTKVector, std::vector<std::string> &inBonematConfVector, std::vector<std::string> &outAnsysInp )
//----------------------------------------------------------------------------
{
  inCDBVector.clear();
  inVTKVector.clear();
  inBonematConfVector.clear();
  outAnsysInp.clear();
  
  FILE *m_FilePointer = fopen(configurationFileName, "r");

  if (m_FilePointer == NULL)
  {
    mafMessage(_("Error parsing input files! File not found."),_("Error"));
    return MAF_ERROR;
  }

  // Get configurationFile path
  m_ConfigurationFilePath = wxFileName(configurationFileName).GetPath();

  char line[512];
  std::vector<std::string> cacheVector;

  while (GetLine(m_FilePointer, line) != 0)
  {
    if(strncmp (line, "#", 1) != 0 && strncmp (line, "\n", 2) != 0)
    {
      cacheVector.push_back(line);  
    }
  }
  
  assert(cacheVector.size() % 4 == 0);
  int numberOfBonematRuns = cacheVector.size() / 4;
  
  for (int i = 0; i < numberOfBonematRuns; i++) 
  {
    std::string fileCDBPath = GetFilePath(cacheVector[4*i]);

    if(!FileExists(fileCDBPath))
    {
      continue;
    }

    std::string fileVTKPath = GetFilePath(cacheVector[4*i+1]);

    if(!FileExists(fileVTKPath))
    {
      continue;
    }

    std::string fileConfPath = GetFilePath(cacheVector[4*i+2]);

    if(!FileExists(fileConfPath))
    {
      continue;
    }

    std::string fileInpPath = GetFilePath(cacheVector[4*i+3]);

    inCDBVector.push_back(fileCDBPath);
    inVTKVector.push_back(fileVTKPath);
    inBonematConfVector.push_back(fileConfPath);
    outAnsysInp.push_back(fileInpPath);   
  }  

  return MAF_OK;
}
//----------------------------------------------------------------------------
int lhpOpBonematBatch::GetLine(FILE *fp, char *buffer)
//--------------------------------------------------------------------------
{
  int readValue;
  int readedChar=0;

  do 
  {
    readValue = fgetc (fp);
    if (readValue>0)
    {
      if(!(readValue == '\n' && readedChar > 0))
      {
        buffer[readedChar]=readValue;
        readedChar++;
      }
    }
  } while (readValue != EOF && readValue != '\n');

  buffer[readedChar]=0;

  return readedChar;
}
//----------------------------------------------------------------------------
std::string lhpOpBonematBatch::GetFilePath(std::string filePath)
//----------------------------------------------------------------------------
{
  wxFileName fileName = wxFileName(filePath.c_str());

  std::string path ="";

  if(fileName.IsRelative())
  {
    path = m_ConfigurationFilePath + "\\" + filePath.c_str();
  }
  else
  {
    path = fileName.GetFullPath();
  }

  return path;
}
//--------------------------------------------------------------------------
bool lhpOpBonematBatch::FileExists(std::string filePath)
//--------------------------------------------------------------------------
{
  if(!wxFileExists(wxString(filePath.c_str())))
  {
    if(!m_TestMode)
    {
      std::string mess = "WARNING! "+ filePath + " not found!";
      wxMessageBox(mess.c_str(), "Warning", wxOK|wxICON_WARNING , NULL);
    }

    return false;
  }

  return true;
}
//----------------------------------------------------------------------------
int lhpOpBonematBatch::OpenConfigurationFileFromGUI()
//----------------------------------------------------------------------------
{
  mafString wildcconf = "txt (*.txt)|*.txt";

  std::string initial = mafGetApplicationDirectory().c_str();

  std::string returnString = mafGetOpenFile("", wildcconf);

  if (returnString == "")
  {
    return MAF_ERROR;
  }

  m_ConfigurationFileName = returnString.c_str();  

  return MAF_OK;
}
//----------------------------------------------------------------------------
void lhpOpBonematBatch::RunBonemat( mafString &pythonExeFullPath, wxString inMeshFileName, wxString inVolumeFileName, wxString inputBonematConfigurationFileName, wxString outputFileName )
//----------------------------------------------------------------------------
{
	//------------------
	// import Ansys (CDB or INP) as VTK Unstructured grid
	//------------------
  
  wxString name, path, ext;
  wxFileName::SplitPath(inMeshFileName, &path, &name, &ext);

	std::ostringstream waitImportAnsysLog;
	waitImportAnsysLog << std::endl;
	waitImportAnsysLog << "//------------------------------------------------------" << std::endl;
	waitImportAnsysLog << "// Importing " << inMeshFileName.c_str() <<  " as VTK Unstructured Grid .... "  << std::endl;
	waitImportAnsysLog << "//------------------------------------------------------"  << std::endl;
	mafLogMessage(waitImportAnsysLog.str().c_str());

  medOpImporterVTK *vtkImporter=new medOpImporterVTK();
	lhpOpImporterAnsysCDBFile *ansysCDBImporter=new lhpOpImporterAnsysCDBFile();
  lhpOpImporterAnsysInputFile *ansysInpImporter=new lhpOpImporterAnsysInputFile();

	if(m_TestMode)
  {
    ansysCDBImporter->TestModeOn();
    ansysInpImporter->TestModeOn();
    vtkImporter->TestModeOn();
  }

  mafVMEMesh *inputVMEMesh = NULL;

  if(ext == "vtk")
  {
    vtkImporter->SetListener(m_Listener);

    assert(wxFileExists(inMeshFileName)); 

    vtkImporter->SetFileName(inMeshFileName);
    vtkImporter->ImportVTK();

    inputVMEMesh=mafVMEMesh::SafeDownCast(vtkImporter->GetOutput());
  }

  if(ext == "cdb")
  {
    ansysCDBImporter->SetListener(m_Listener);

    assert(wxFileExists(inMeshFileName) == true); 

    ansysCDBImporter->SetFileName(inMeshFileName);
    ansysCDBImporter->Import();

    inputVMEMesh=mafVMEMesh::SafeDownCast(ansysCDBImporter->GetOutput());
  }

  if(ext == "inp")
  {
    ansysInpImporter->SetListener(m_Listener);

    assert(wxFileExists(inMeshFileName) == true); 

    ansysInpImporter->SetFileName(inMeshFileName);
    ansysInpImporter->Import();

    inputVMEMesh=mafVMEMesh::SafeDownCast(ansysInpImporter->GetOutput());
  }
    
	assert(inputVMEMesh!=NULL);
	inputVMEMesh->Modified();
	inputVMEMesh->Update();

	vtkUnstructuredGrid *inputVTKUnstructuredGrid=vtkUnstructuredGrid::SafeDownCast(inputVMEMesh->GetOutput()->GetVTKData());

	assert(inputVTKUnstructuredGrid!=NULL);

	inputVTKUnstructuredGrid->Modified();
	inputVTKUnstructuredGrid->Update();

	//------------------
	// import VTK volume (Rectilinear grid or structured points)
	//------------------

	std::ostringstream waitImportVolumeLog;
	waitImportVolumeLog << std::endl;
	waitImportVolumeLog << "//------------------------------------------------------"  << std::endl;;
	waitImportVolumeLog << "// Importing VTK Volume " << inVolumeFileName.c_str() << " .... " << std::endl;
	waitImportVolumeLog << "//------------------------------------------------------"  << std::endl;;
	mafLogMessage(waitImportVolumeLog.str().c_str());

	wxBusyInfo *waitImportVolume;
	if(!m_TestMode)
	{
		waitImportVolume = new wxBusyInfo("Please wait importing VTK Volume...");
	}

  medOpImporterVTK *vtkVolumeImporter=new medOpImporterVTK();
  
  if(m_TestMode)
    vtkVolumeImporter->TestModeOn();

  vtkVolumeImporter->SetListener(m_Listener);

	assert(wxFileExists(inVolumeFileName)); 

	vtkVolumeImporter->SetFileName(inVolumeFileName);

	vtkVolumeImporter->ImportVTK();
	mafVMEVolumeGray *inputVolume=mafVMEVolumeGray::SafeDownCast(vtkVolumeImporter->GetOutput());

	assert(inputVolume!=NULL);
	inputVolume->Modified();
	inputVolume->Update();
	inputVolume->GetOutput()->Update();
	inputVolume->GetOutput()->GetVTKData()->Update();

	std::ostringstream importVolumeOkLog;
	importVolumeOkLog << std::endl;;
	importVolumeOkLog << " VTK volume imported correctly " << std::endl;
	importVolumeOkLog << std::endl;;
	mafLogMessage(importVolumeOkLog.str().c_str());

	assert(inputVolume);
	assert(inputVMEMesh);

	if(!m_TestMode)
	{
		delete waitImportVolume;
	}

	//------------------
	// Run Bonemat
	//------------------
  
	std::ostringstream waitExecuteBonematLog;
	waitExecuteBonematLog << std::endl;
	waitExecuteBonematLog << "//------------------------------------------------------"  << std::endl;;
	waitExecuteBonematLog << "// Executing Bonemat ... " << std::endl;
	waitExecuteBonematLog << "//------------------------------------------------------"  << std::endl;;
	mafLogMessage(waitExecuteBonematLog.str().c_str());

	// Create the bonemat operation
	lhpOpBonemat *opBonemat = new lhpOpBonemat("op bonemat");
  if(m_TestMode)
    opBonemat->TestModeOn();

  opBonemat->SetListener(m_Listener);

	opBonemat->SetInput(inputVMEMesh);
	opBonemat->SetSourceVolume(inputVolume);
	opBonemat->LoadConfigurationFile(inputBonematConfigurationFileName); 
	opBonemat->SetFrequencyFileName("frequencyFileToBeDiscarded.txt");

	// execute bonemat
	int result = opBonemat->Execute();

	assert(result == MAF_OK);

	//------------------
	// export the bonemat mesh as vtk, ansys inp or cdb file
	//------------------

  wxFileName::SplitPath(outputFileName, &path, &name, &ext);

	std::ostringstream waitExportBonematMeshLog;
	waitExportBonematMeshLog << std::endl;
	waitExportBonematMeshLog << "//------------------------------------------------------"  << std::endl;;
	waitExportBonematMeshLog << "// Exporting Bonematted Mesh as " << outputFileName.c_str() << " File .... " << std::endl;
	waitExportBonematMeshLog << "//------------------------------------------------------"  << std::endl;;
	mafLogMessage(waitExportBonematMeshLog.str().c_str());

	wxBusyInfo *waitExportBonematMesh;
	if(!m_TestMode)
	{
		waitExportBonematMesh = new wxBusyInfo("Please wait for Bonemat Mesh export to " + ext.Upper() + " file");
	}

  if(ext == "vtk")
  {
    mafOpExporterVTK *opExportVTK = new mafOpExporterVTK("op export VTK");    

    waitExportBonematMeshLog << std::endl;
    opExportVTK->SetInput(opBonemat->GetOutput());
    opExportVTK->SetFileName(outputFileName);	

    if(m_TestMode)
      opExportVTK->TestModeOn();

    opExportVTK->SetListener(m_Listener);

    opExportVTK->ExportVTK();

    result = wxFileExists(outputFileName) ? MAF_OK : MAF_ERROR;

    mafDEL(opExportVTK);
  }

  if(ext == "inp")
  {
    lhpOpExporterAnsysInputFile *opExportAnsysInp = new lhpOpExporterAnsysInputFile("op export ansys inp");

    waitExportBonematMeshLog << std::endl;
    opExportAnsysInp->SetInput(opBonemat->GetOutput());
    opExportAnsysInp->SetOutputFileName(outputFileName);	

    if(m_TestMode)
      opExportAnsysInp->TestModeOn();

    opExportAnsysInp->SetListener(m_Listener);

    result = opExportAnsysInp->Write();

    mafDEL(opExportAnsysInp);
  }

  if(ext == "cdb")
  {
    lhpOpExporterAnsysCDBFile *opExportAnsysCDB = new lhpOpExporterAnsysCDBFile("op export ansys CDB");

    waitExportBonematMeshLog << std::endl;
    opExportAnsysCDB->SetInput(opBonemat->GetOutput());
    opExportAnsysCDB->SetOutputFileName(outputFileName);	

    if(m_TestMode)
      opExportAnsysCDB->TestModeOn();

    opExportAnsysCDB->SetListener(m_Listener);

    result = opExportAnsysCDB->Write();

    mafDEL(opExportAnsysCDB);
  }

	assert(result == MAF_OK);

	assert(wxFileExists(outputFileName));

	// clean up
	mafDEL(vtkImporter);
  mafDEL(vtkVolumeImporter);
	mafDEL(ansysCDBImporter);
  mafDEL(ansysInpImporter);
	mafDEL(opBonemat);

	if(!m_TestMode)
	{
		delete waitExportBonematMesh;
	}
}
