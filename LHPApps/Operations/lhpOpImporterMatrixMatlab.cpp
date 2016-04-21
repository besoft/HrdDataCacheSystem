/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterMatrixMatlab.cpp,v $
  Language:  C++
  Date:      $Date: 2012-03-08 15:51:57 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Youbing Zhao
==========================================================================
  Copyright (c) 2002/2012
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpOpImporterMatrixMatlab.h"
#include "mafVME.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMELandmark.h"
#include "mafVMESurface.h"
#include "mafGUI.h"

#include <vtkPolyData.h>

#include <wx/busyinfo.h>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#include <vtkMAFSmartPointer.h>
#include <vtkCellArray.h>

extern "C" {
#include "matio.h"
}

//----------------------------------------------------------------------------
 lhpOpImporterMatrixMatlab:: lhpOpImporterMatrixMatlab(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_IMPORTER;
  //m_Canundo	= true;
  m_File	= "";
  //m_FileDir = (mafGetApplicationDirectory() + "/Data/External/").c_str();
  m_FileDir = "";

  m_pVme = NULL;
  m_NameVme = mafString("VME not selected");
  m_bUsePoseMatrix = 1;
  m_bUsePointList = 1;

}
//----------------------------------------------------------------------------
 lhpOpImporterMatrixMatlab::~ lhpOpImporterMatrixMatlab()
//----------------------------------------------------------------------------
{

}

//----------------------------------------------------------------------------
mafOp*  lhpOpImporterMatrixMatlab::Copy()   
//----------------------------------------------------------------------------
{
   lhpOpImporterMatrixMatlab *cp = new  lhpOpImporterMatrixMatlab(m_Label);
  cp->m_File = m_File;
  cp->m_FileDir = m_FileDir;
  return cp;
}

bool lhpOpImporterMatrixMatlab::Accept(mafNode* node)
{
	//return (node && node->IsA("mafVME"));
	return true;
}

//----------------------------------------------------------------------------
// Operation constants
//----------------------------------------------------------------------------
enum Octave_Exporter_ID
{
  ID_FIRST = MINID,
  ID_CHOOSE_VME,
  ID_USE_POSEMATRIX,
  ID_USE_POINTLIST,
  ID_CHOOSE_VMETYPE,

  ID_OK,
  ID_CANCEL,
};

//----------------------------------------------------------------------------
void  lhpOpImporterMatrixMatlab::CreateGui()
//----------------------------------------------------------------------------
{
  //mafString wildc = "Stereo Litography (*.stl)|*.stl";

  m_Gui = new mafGUI(this);
  /*m_Gui->Label("absolute matrix",true);
  m_Gui->Bool(ID_ABS_MATRIX_TO_STL,"apply",&m_ABSMatrixFlag,0);*/

  //m_Gui->Label("Operation Panel");
/*	
  m_Gui->Label(_("Target VME"),true);
  m_Gui->Label(&m_NameVme);
  m_Gui->Button(ID_CHOOSE_VME,_("Select target VME for importing"));

  m_Gui->Divider();
   */

  //m_Gui->Label("Importer Options", true);
  //m_Gui-> Bool(ID_USE_POSEMATRIX, "PoseMatrix", & m_bUsePoseMatrix, 0, "Import pose matrix to the octave matrix file"	);
  //m_Gui-> Bool(ID_USE_POINTLIST, "Point List", & m_bUsePointList, 0, "Import point list to the octave matrix file"	);

  m_Gui->Divider();
  
  int numChoices = 2;
  m_VmeType = 0;
  const wxString choices[] = {"Landmark Cloud", "Surface" };
  m_Gui->Label("VME Type", true);
  m_Gui->Radio(ID_CHOOSE_VMETYPE, "", & m_VmeType, numChoices, choices, 1, "Select VME type to be generated");


  m_Gui->OkCancel();  

  //m_Gui->Enable(wxOK, false);

  ShowGui();



}
//----------------------------------------------------------------------------
void  lhpOpImporterMatrixMatlab::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
	  case ID_CHOOSE_VME:
      {
        OnChooseVme();
      }
      break;
    case wxOK:
      {
        int ret = OnOK();
        this->OpStop(ret);
      }
      break;
    case wxCANCEL:
      {
        this->OpStop(OP_RUN_CANCEL);
      }
      break;
    default:
      mafEventMacro(*e);
      break;
    }	
  }
}

 void  lhpOpImporterMatrixMatlab::OnChooseVme()
 {
	mafEvent e(this,VME_CHOOSE);
	mafEventMacro(e);
	mafNode *vme = e.GetVme();

	if(!vme) return; // the user choosed cancel - keep previous target

	if (! vme->IsA("mafVMESurface") && ! vme->IsA("mafVMELandmarkCloud") ) // the user choosed ok     - check if it is a valid vme
	{
		wxString msg = _("Data must be an mafVMESurface or mafVMELandmarkCloud\n please choose another \n");
		wxMessageBox(msg,_("incorrect vme type"),wxOK|wxICON_ERROR);
		//m_Gui->Enable(wxOK,false);
		//m_Gui->Update();
		//mafEventMacro(mafEvent(this,result));
		return;
	}

	m_pVme = mafVME::SafeDownCast(vme);
	m_pVme->GetOutput()->Update();
	m_NameVme = mafString(m_pVme->GetName());

	 m_Gui->Enable(wxOK, true);
	 m_Gui->Update();

 }

void lhpOpImporterMatrixMatlab::CreateVMELandmarkCloud()
{
	//mafNEW(m_Cloud);
	mafSmartPointer<mafVMELandmarkCloud> m_Cloud;
	m_Cloud->ReparentTo(m_Input);
	m_Cloud->Open();
	//m_Cloud->SetName(_("Imported Landmark Cloud"));
	//m_Cloud->SetRadius(m_InputVME->GetOutput()->GetVTKData()->GetLength()/60.0);
	//m_Cloud->SetRadius(10);
	std::string matName = m_matPointList.GetName();
	int idx = matName.find('_');
	std::string vmeName = matName.substr(0, idx);
	std::string importedName = std::string("Imported_") + vmeName;
	m_Cloud->SetName(importedName.c_str());

	int nPoints = m_matPointList.GetNumOfRows();
	std::string ldNamePfx = "Landmark";
	char ldName[20];
	double pos[3];
	for (int i = 0; i < nPoints; i++)
	{
		
		sprintf(ldName, "Landmark_%d", i);
		//ldName =  ldNamePfx  +  i;
		
		for (int j = 0; j < 3; j++)
			pos[j] = m_matPointList.GetElement(i, j);
		
		mafSmartPointer<mafVMELandmark> pLandmark;
		
		pLandmark->SetName(ldName);
		pLandmark->ReparentTo(m_Cloud);

		//if(NULL != m_InputVME)
		//	pLandmark->SetTimeStamp(m_InputVME->GetTimeStamp());

		pLandmark->SetAbsPose(pos[0],pos[1],pos[2],0,0,0);
		pLandmark->Update();

	}

	SetPoseMatrix(m_Cloud);

}

void lhpOpImporterMatrixMatlab::CreateVMESurface()
{
	mafSmartPointer<mafVMESurface> pVmeSurf;
	pVmeSurf->ReparentTo(m_Input);
	
	std::string matName = m_matPointList.GetName();
	int idx = matName.find('_');
	std::string vmeName = matName.substr(0, idx);
	std::string importedName = std::string("Imported_") + vmeName;
	pVmeSurf->SetName(importedName.c_str());

	int nPoints = m_matPointList.GetNumOfRows();

	vtkMAFSmartPointer<vtkPolyData> pPoly;
	vtkMAFSmartPointer<vtkPoints>  pAllPoints;
	pAllPoints->SetNumberOfPoints(nPoints);

	// add points
	double point[3];
	for (int i = 0; i < nPoints; i++)
	{
		for (int j = 0; j < 3; j++)
			point[j] = m_matPointList.GetElement(i, j);
		
		pAllPoints->SetPoint(vtkIdType(i), point);
	}

	pPoly->SetPoints(pAllPoints.GetPointer());

	// add Faces
	int nFaces = m_matFaceList.GetNumOfRows();
	vtkMAFSmartPointer<vtkCellArray>  pCellArray;
	int nIndex = m_matFaceList.GetNumOfColumns();
	int * pIdx = new int[nIndex];
	
	pCellArray->Initialize();
	for (int i = 0; i < nFaces; i++)
	{
		for (int j = 0; j < nIndex; j++)
			pIdx[j] = m_matFaceList.GetElement(i, j);

		pCellArray->InsertNextCell (nIndex, pIdx);
	}

	delete [] pIdx;
	pPoly->SetPolys(pCellArray);
	pPoly->Update();

	pVmeSurf->SetData(pPoly, 0);
	SetPoseMatrix(pVmeSurf);
}

void lhpOpImporterMatrixMatlab::SetPoseMatrix(mafVME * pVme)
{
	if (!pVme) 
		return;
	
	// apply the matrix to the VME
	if  ((m_matPoseMatrix.GetNumOfRows() !=4) || (m_matPoseMatrix.GetNumOfColumns() != 4))
	{
		throw OctaveImportException("Matrix size is not 4x4");
	}
	
	mafLogMessage("Apply pose matrix to VME");
			
	mafMatrix mmat;
	for (int i = 0; i < 4; i ++)
		for (int j = 0; j < 4; j++)
			mmat.SetElement(i, j, m_matPoseMatrix.GetElement(i, j));

	pVme->SetAbsMatrix(mmat); // timestamp ??
}

int lhpOpImporterMatrixMatlab::OnOK()
{
	int result = OP_RUN_CANCEL;
	
		// read octave file, currently only read pose matri and point datax
		try {
			Read();
		
			if (m_VmeType == 0)
			{
				// landmark cloud
				CreateVMELandmarkCloud();

			}
			else if (m_VmeType == 1)
			{
				CreateVMESurface();
			}
	
			result = OP_RUN_OK;

			
		} catch (OctaveImportException & ex) {

			result = OP_RUN_CANCEL;
			std::string str = std::string("Error: ") + ex.what();
			wxMessageBox(str.c_str());
			mafLogMessage(str.c_str());
		}
		


	return result;
}

void lhpOpImporterMatrixMatlab::ImportPointList()
{
				// apply point data
			if  ( m_matPointList.IsTypeInfoReady() )
			{
				if  ( m_matPointList.GetNumOfColumns() != 3)
				{
					throw OctaveImportException("Points not 3D");
				}

				double point[3];
				if (m_pVme->IsA("mafVMELandmarkCloud"))
				{
					mafVMELandmarkCloud * pLdCloud = mafVMELandmarkCloud::SafeDownCast(m_pVme);
					if (m_matPointList.GetNumOfRows() != pLdCloud->GetNumberOfLandmarks())
					{
						throw OctaveImportException("Point number not equal to number of landmarks");
					}

					mafLogMessage("Apply point data to mafVMELandmarkCloud");

					for (int i = 0; i < m_matPointList.GetNumOfRows(); i++ )
					{
						for (int j = 0; j < 3; j++)
							point[j] =  m_matPointList.GetElement(i, j);

						pLdCloud->SetLandmark(i, point[0], point[1], point[2]);
					}
				}
				else if (m_pVme->IsA("mafVMESurface") )
				{
					mafVMESurface * pSurface = mafVMESurface::SafeDownCast(m_pVme);
				    pSurface->GetOutput()->Update();
					vtkPolyData * pPolyData = vtkPolyData::SafeDownCast(pSurface->GetOutput()->GetVTKData());
				
					if (m_matPointList.GetNumOfRows() != pPolyData->GetNumberOfPoints())
					{
						throw OctaveImportException("Point number in file not equal to number of points in VME");
					}

					mafLogMessage("Apply point data to mafVMESurface");

					vtkPoints * pPoints = pPolyData->GetPoints();

					for (int i = 0; i < m_matPointList.GetNumOfRows(); i++ )
					{
						for (int j = 0; j < 3; j++)
							point[j] =  m_matPointList.GetElement(i, j);

						pPoints->SetPoint(i, point);
					}

					pPolyData->SetPoints(pPoints);
					pSurface->SetData(pPolyData, 0);
					pSurface->Update();
				}
			} // if ( m_matPointList.IsTypeInfoReady() )
}

void lhpOpImporterMatrixMatlab::ImportPoseMatrix()
{
	// apply the matrix to the VME
	if  ((m_matPoseMatrix.GetNumOfRows() !=4) || (m_matPoseMatrix.GetNumOfColumns() != 4))
	{
		throw OctaveImportException("Matrix size is not 4x4");
	}
	
	mafLogMessage("Apply pose matrix to VME");
			
	mafMatrix mmat;
	for (int i = 0; i < 4; i ++)
		for (int j = 0; j < 4; j++)
			mmat.SetElement(i, j, m_matPoseMatrix.GetElement(i, j));

	m_pVme->SetAbsMatrix(mmat); // timestamp ??
}
//----------------------------------------------------------------------------
void  lhpOpImporterMatrixMatlab::OpRun()   
//----------------------------------------------------------------------------
{
	m_File = "";
	wxString pgd_wildc	= "Octave Matrix File (*.mat)|*.mat";
	wxString f;
	f = mafGetOpenFile(m_FileDir,pgd_wildc).c_str(); 
	if(!f.IsEmpty() && wxFileExists(f))
	{
		m_File = f;
		CreateGui();
	}
	else
	{
		int result = OP_RUN_CANCEL;
		mafEventMacro(mafEvent(this,result));
		//this->OpStop(OP_RUN_CANCEL);
	}


}

//----------------------------------------------------------------------------
void  lhpOpImporterMatrixMatlab::Read()   
//----------------------------------------------------------------------------
{
	wxString path, fname, ext;
	wxSplitPath(m_File.c_str(),&path,&fname,&ext);

	mafLogMessage(wxString::Format("Starting import %s", fname).c_str());

	mat_t *mat = NULL;
    matvar_t *matvar = NULL;

	// matrix file
    mat = Mat_Open(m_File.c_str(),MAT_ACC_RDONLY);

	if (NULL == mat) 
	{
		throw OctaveImportException("File corrupted or not matlab file");
	}


	char * matName = NULL;
	int nRow = 0; 
	int nCol = 0;
	double elem = 0.0;
	int stride = 0;

	while (1) {
		matvar = Mat_VarReadNext(mat);

		if (NULL == matvar)
			break;

		MatrixOctave matTmp;
		matTmp.SetName(matvar->name);
		nRow = matvar->dims[0];
		nCol = matvar->dims[1];
		matTmp.SetNumOfRows(nRow);
		matTmp.SetNumOfColumns(nCol);

		if (matvar->data_type == MAT_T_DOUBLE)
			stride = 8;
		else if (matvar->data_type == MAT_T_INT32)
			stride = 4;
		else  if (matvar->data_type == MAT_T_UINT32)
			stride = 4;
		else
			throw OctaveImportException("Data type is not double, int32, uint32");


		for (int i = 0; i < nRow; i++)
			for (int j = 0; j < nCol; j++)
			{
			
				if (matvar->data_type == MAT_T_DOUBLE)
					elem = * ( (double *)matvar->data + (j * nRow + i) );
				else if (matvar->data_type == MAT_T_INT32) 
					elem = * ( (int *)matvar->data + (j * nRow + i) );
				else  if (matvar->data_type == MAT_T_UINT32)
					elem = * ( (unsigned int *)matvar->data + (j * nRow + i) );


				matTmp.SetElement(i, j, elem);
			}
		
		matTmp.SetElemValid(true);
		if (matTmp.IsPoseMatrix())
			m_matPoseMatrix = matTmp;
		else if (matTmp.IsPointList())
			m_matPointList = matTmp;
		else if (matTmp.IsFaceList())
			m_matFaceList = matTmp;

		Mat_VarFree(matvar);
	}
	
	Mat_Close(mat);

}

