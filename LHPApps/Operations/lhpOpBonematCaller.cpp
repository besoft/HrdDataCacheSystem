/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpBonematCaller.cpp,v $
  Language:  C++
  Date:      $Date: 2014-10-07 12:33:41 $
  Version:   $Revision: 1.1.1.1.2.4 $
  Authors:   Gianluigi Crimi
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

#include "lhpOpBonematCaller.h"
#include <wx/config.h>
#include "lhpDefines.h"
#include "lhpProceduralElements.h"
#include "mafOpExporterVTK.h"
#include "medOpImporterVTK.h"
#include "mafVMEMesh.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkUnstructuredGrid.h"

#define BonematURL "https://www.bonemat.org"
#define LhpBuilderURL "https://www.biomedtown.org/"

//----------------------------------------------------------------------------
lhpOpBonematCaller::lhpOpBonematCaller(wxString label) :
lhpOpBonematCommon(label)
//----------------------------------------------------------------------------
{
  m_bonematCalleePath="";
}
//----------------------------------------------------------------------------
lhpOpBonematCaller::~lhpOpBonematCaller()
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
mafOp* lhpOpBonematCaller::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpBonematCaller *cp = new lhpOpBonematCaller(m_Label);
  return cp;
}
//----------------------------------------------------------------------------
void lhpOpBonematCaller::OpRun()   
//----------------------------------------------------------------------------
{ 

  if(CheckBonematCompatibility() == MAF_OK)
  {
    lhpOpBonematCommon::OpRun();
  }
  else
  {
    OpStop(OP_RUN_CANCEL);
  }
}
//----------------------------------------------------------------------------
int lhpOpBonematCaller::Execute()
{  
	CreateMeshCopy();

	return ExecuteBonematCallee();
}
//---------------------------------------------------------------------------
int lhpOpBonematCaller::CheckBonematCompatibility()
{
  wxRegKey RegKey(wxString("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Bonemat"));

  if(RegKey.Exists())
  {
    // Check Version Compatibility
    RegKey.Create();
    wxString str_bonematCommonVer;
    RegKey.QueryValue(wxString("BonematCommonVersion"), str_bonematCommonVer);

    int bonematCommonVer = atoi(str_bonematCommonVer);

    if(bonematCommonVer == BONEMAT_COMMON_VERSION)
    {
      // Get BonematCallee.exe Path
      RegKey.QueryValue(wxString("DisplayIcon"), m_bonematCalleePath);
      m_bonematCalleePath.Replace("Bonemat.exe", "BonematCallee.exe");

      return MAF_OK;
    }
    else
    {
      // Suggest Bonemat Update or Builder Update
      if(bonematCommonVer > BONEMAT_COMMON_VERSION)
      {
        if (GetTestMode() == false)
        {
          if(wxMessageBox("Update Builder", _("Warning"),wxYES_NO) == wxYES)
          {
            OpenUrl( wxString(LhpBuilderURL));
          }
        }
      }
      else
      {
        if (GetTestMode() == false)
        {
          if(wxMessageBox("Update Bonemat", _("Warning"), wxYES_NO) == wxYES)
          {
            OpenUrl( wxString(BonematURL));
          }
        }
      }
    }
  }
  else
  {
    // Suggest Bonemat Installation
    if (GetTestMode() == false)
    {
      if(wxMessageBox("Install Bonemat", _("Warning"), wxYES_NO) == wxYES)
      {
        OpenUrl( wxString(BonematURL));
      }
    }
  }

  return MAF_ERROR;
}
//---------------------------------------------------------------------------
void lhpOpBonematCaller::OpenUrl(wxString &url)
{
  //url = url + m_File.GetCStr();
  url.Replace("\\","/");
  mafLogMessage("Opening %s",url.c_str());
  wxString command = "rundll32.exe url.dll,FileProtocolHandler ";
  command = command + url;
  wxExecute( command );
}
//---------------------------------------------------------------------------
int lhpOpBonematCaller::ExecuteBonematCallee()
{
  // Create tmp path
  mafString tmpPath = mafGetApplicationDirectory().c_str();
  tmpPath.Append("\\tmp");
  mkdir(tmpPath);

  // Create VTK Exporter
  mafOpExporterVTK *exporterVTK=new mafOpExporterVTK();
  exporterVTK->TestModeOn();

  // Export Volume
  wxString inputVolume = tmpPath + "\\volume.vtk";

  if(m_InputVolume == NULL)
  {
    rmdir(tmpPath);

    mafDEL(exporterVTK);
    return MAF_ERROR;
  }

  exporterVTK->SetInput(m_InputVolume);
  exporterVTK->SetFileName(inputVolume);
  exporterVTK->ExportVTK();

  // Export Mesh
  wxString inputMesh = tmpPath + "\\mesh.vtk";

  if(m_Input == NULL)
  {
    // Remove tmp files
    remove(inputVolume);
    rmdir(tmpPath);

    mafDEL(exporterVTK);
    return MAF_ERROR;
  }

	CreateAbsMesh();

  exporterVTK->SetInput(m_AbsMesh);
  exporterVTK->SetFileName(inputMesh);
  exporterVTK->ExportVTK();

	DeleteAbsMesh();

  mafDEL(exporterVTK);

  // Save Configuration File
  wxString inputConfiguration = tmpPath + "\\config.xml";

  SaveConfigurationFile(inputConfiguration);

  // Set Frequency File
  wxString inputFrequency = m_FrequencyFileName;

  // Set Output Mesh File
  wxString outputMesh = tmpPath + "\\result.vtk";

  // CALL BONEMAT
  char temp[512];
  sprintf(temp, "%s \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"", m_bonematCalleePath, inputVolume, inputMesh, inputConfiguration, inputFrequency, outputMesh);

  int	result = system((char *)temp);

  if(result == MAF_OK)
  {
    // Import Mesh
    medOpImporterVTK *importerVTK=new medOpImporterVTK();
    importerVTK->TestModeOn();
    importerVTK->SetFileName(outputMesh);
    result = importerVTK->ImportVTK();

    mafVMEOutputMesh *outputImporter=mafVMEMesh::SafeDownCast(importerVTK->GetOutput())->GetUnstructuredGridOutput();
    mafVMEMesh::SafeDownCast(m_OutputMesh)->SetData(outputImporter->GetUnstructuredGridData(),0);

    mafDEL(importerVTK);
  }

	TransformOutput();

  // Remove tmp files
  remove(inputVolume);
  remove(inputMesh);
  remove(inputConfiguration);
  remove(inputConfiguration + ".Load.cache");
  remove(outputMesh);
	
  rmdir(tmpPath);

  return result;
}

void lhpOpBonematCaller::CreateAbsMesh()
{
	mafMatrix identityMatrix;
	mafMatrix meshAbsMatrix = mafVMEMesh::SafeDownCast(m_OutputMesh)->GetAbsMatrixPipe()->GetMatrix();
	mafMatrix volumeAbsMatrix = mafVMEVolumeGray::SafeDownCast(m_InputVolume)->GetAbsMatrixPipe()->GetMatrix();

	bool isMeshMatrixIdentity = meshAbsMatrix.Equals(&identityMatrix);
	bool isVolumeMatrixIdentity = volumeAbsMatrix.Equals(&identityMatrix);

	if (!(isMeshMatrixIdentity && isVolumeMatrixIdentity))
	{		
		m_NewAbsMeshGenerated = true;

		// if VME matrix is not identity apply it to dataset
		vtkTransform *transform = NULL;
		vtkTransformFilter *transformFilter = NULL;
		

		//Calculate align matrix 
		volumeAbsMatrix.Invert();
		mafMatrix::Multiply4x4(volumeAbsMatrix,meshAbsMatrix,m_MeshVolumeAlignMatrix);

		mafNEW(m_AbsMesh);
		m_AbsMesh->DeepCopy(m_Input);
		
		// apply abs matrix to geometry
		transform = vtkTransform::New();
		transform->SetMatrix(m_MeshVolumeAlignMatrix.GetVTKMatrix());

		// to delete
		transformFilter = vtkTransformFilter::New();

		transformFilter->SetInput(m_AbsMesh->GetUnstructuredGridOutput()->GetUnstructuredGridData());
		transformFilter->SetTransform(transform);
		transformFilter->Update();

		m_AbsMesh->SetData(transformFilter->GetOutput(),0);
	}
	else
	{
		m_NewAbsMeshGenerated=false;
		m_AbsMesh=mafVMEMesh::SafeDownCast(m_Input);
	}

}

void lhpOpBonematCaller::DeleteAbsMesh()
{
	if(m_NewAbsMeshGenerated)
	{
		mafDEL(m_AbsMesh);
	}
}

void lhpOpBonematCaller::TransformOutput()
{

	mafMatrix identityMatrix;
	
	bool isAlignMatrixIdentity = m_MeshVolumeAlignMatrix.Equals(&identityMatrix);

	if (!isAlignMatrixIdentity)
	{		
		// if VME matrix is not identity apply it to dataset
		vtkTransform *transform = NULL;
		vtkTransformFilter *transformFilter = NULL;


		m_MeshVolumeAlignMatrix.Invert();

		// apply abs matrix to geometry
		transform = vtkTransform::New();
		transform->SetMatrix(m_MeshVolumeAlignMatrix.GetVTKMatrix());

		// to delete
		transformFilter = vtkTransformFilter::New();
		transformFilter->SetInput(m_OutputMesh->GetUnstructuredGridOutput()->GetUnstructuredGridData());
		transformFilter->SetTransform(transform);
		transformFilter->Update();

		m_OutputMesh->SetData(transformFilter->GetOutput(),0);
	}
}

