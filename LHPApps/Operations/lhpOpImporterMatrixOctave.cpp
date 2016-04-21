/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterMatrixOctave.cpp,v $
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
#include "lhpOpImporterMatrixOctave.h"
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

//----------------------------------------------------------------------------
 lhpOpImporterMatrixOctave:: lhpOpImporterMatrixOctave(const wxString &label) :
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
 lhpOpImporterMatrixOctave::~ lhpOpImporterMatrixOctave()
//----------------------------------------------------------------------------
{

}

//----------------------------------------------------------------------------
mafOp*  lhpOpImporterMatrixOctave::Copy()   
//----------------------------------------------------------------------------
{
   lhpOpImporterMatrixOctave *cp = new  lhpOpImporterMatrixOctave(m_Label);
  cp->m_File = m_File;
  cp->m_FileDir = m_FileDir;
  return cp;
}

bool lhpOpImporterMatrixOctave::Accept(mafNode* node)
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
void  lhpOpImporterMatrixOctave::CreateGui()
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
void  lhpOpImporterMatrixOctave::OnEvent(mafEventBase *maf_event) 
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

 void  lhpOpImporterMatrixOctave::OnChooseVme()
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

void lhpOpImporterMatrixOctave::CreateVMELandmarkCloud()
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
	std::string importedName = std::string("Imported ") + vmeName;
	m_Cloud->SetName(importedName.c_str());

	int nPoints = m_matPointList.GetNumOfRows();
	std::string ldNamePfx = "Landmark";
	double pos[3];
	for (int i = 0; i < nPoints; i++)
	{
		std::stringstream ldName;
		ldName << ldNamePfx << " " << i;
		
		for (int j = 0; j < 3; j++)
			pos[j] = m_matPointList.GetElement(i, j);
		
		mafSmartPointer<mafVMELandmark> pLandmark;
		
		pLandmark->SetName(ldName.str().c_str());
		pLandmark->ReparentTo(m_Cloud);

		//if(NULL != m_InputVME)
		//	pLandmark->SetTimeStamp(m_InputVME->GetTimeStamp());

		pLandmark->SetAbsPose(pos[0],pos[1],pos[2],0,0,0);
		pLandmark->Update();

	}

	SetPoseMatrix(m_Cloud);

}

void lhpOpImporterMatrixOctave::CreateVMESurface()
{
	mafSmartPointer<mafVMESurface> pVmeSurf;
	pVmeSurf->ReparentTo(m_Input);
	
	std::string matName = m_matPointList.GetName();
	int idx = matName.find('_');
	std::string vmeName = matName.substr(0, idx);
	std::string importedName = std::string("Imported ") + vmeName;
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

void lhpOpImporterMatrixOctave::SetPoseMatrix(mafVME * pVme)
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

int lhpOpImporterMatrixOctave::OnOK()
{
	int result = OP_RUN_CANCEL;
	
		// read octave file, currently only read pose matri and point datax
		try {
			Read();
		
			//if (m_bUsePointList)
			//	ImportPointList();

			//if (m_bUsePoseMatrix)
			//	ImportPoseMatrix();
	
			result = OP_RUN_OK;

		} catch (OctaveImportException & ex) {
			std::string str = std::string("Error: ") + ex.what();
			wxMessageBox(str.c_str());
			mafLogMessage(str.c_str());
		}

		if (m_VmeType == 0)
		{
			// landmark cloud
			CreateVMELandmarkCloud();

		}
		else if (m_VmeType == 1)
		{
			CreateVMESurface();
		}

	//}

	return result;
}

void lhpOpImporterMatrixOctave::ImportPointList()
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

void lhpOpImporterMatrixOctave::ImportPoseMatrix()
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
void  lhpOpImporterMatrixOctave::OpRun()   
//----------------------------------------------------------------------------
{
	m_File = "";
	wxString pgd_wildc	= "Octave Matrix File (*.mat)|*.mat";
	wxString f;
	f = mafGetOpenFile(m_FileDir,pgd_wildc).c_str(); 
	if(!f.IsEmpty() && wxFileExists(f))
	{
		m_File = f;
	}

	CreateGui();

	//int result = OP_RUN_CANCEL;
	//mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void  lhpOpImporterMatrixOctave::Read()   
//----------------------------------------------------------------------------
{
	/*
	if (!m_TestMode)
	{
	wxBusyInfo wait("Please wait, working...");
	mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
	}
	*/
	wxString path, fname, ext;
	wxSplitPath(m_File.c_str(),&path,&fname,&ext);

	mafLogMessage(wxString::Format("start_import %s", fname).c_str());


	// read all the lines in the file
	std::ifstream inputfile;
	inputfile.open(m_File.c_str(), std::ifstream::in);
	if (!inputfile)
	{
		throw OctaveImportException("Open file failed");
	}

	inputfile.exceptions (std::ios_base::badbit);

	std::string line;
	try {
		  while (!inputfile.eof())
		  {
			  std::getline(inputfile, line);
			  if (! line.empty())
				m_Lines.push_back(line);
		  }
	}
	catch (const std::ifstream::failure &  ex )  {
		inputfile.close();
		throw OctaveImportException("reading lines in the file failed");
	}
	
	inputfile.close();

	HeaderOctave hdr;
	MatrixOctave mat;

	m_nCurrLine = 0;
	while (m_nCurrLine < m_Lines.size() )
	{
		HeaderOctave hdr = ReadHeader();
		if ( !hdr.valid())
		{
			throw OctaveImportException("Not an valid octave file : no object header");
		}

		// only read one matrix now
		if (hdr.type.compare("matrix") == 0)
		{
			// return a good matrix otherwise throw an exception
			mat = ReadMatrix(hdr);
			//if (mat.GetElemValid() )
			if (hdr.IsPoseMatrix())
				m_matPoseMatrix = mat;
			else if (hdr.IsPointList())
				m_matPointList = mat;
			else if (hdr.IsFaceList())
				m_matFaceList = mat;
			//break;
		}
	}
}

lhpOpImporterMatrixOctave::MatrixOctave lhpOpImporterMatrixOctave::ReadMatrix(HeaderOctave & hdr)
{

	MatrixOctave mat;
	mat.SetName(hdr.name);
	std::string line;

	std::string  key;
	std::string value;
	
	while (m_nCurrLine < m_Lines.size())
	{
		line = m_Lines[m_nCurrLine];

		m_nCurrLine++;
		
		// reaader header
		if (line[0] == '#')
		{
			int pos = line.find_first_of(':');

			if (pos >=2)
			{
				key = line.substr(1, pos - 1);
				key = RemoveWhiteSpacesAtEnds(key);

				value = line.substr(pos+1, line.size()  - pos - 1);
				value = RemoveWhiteSpacesAtEnds(value);

				if (key.compare("rows") == 0)
				{
					mat.SetNumOfRows(atoi(value.c_str()) );
				}
				else if (key.compare("columns") == 0)
				{
					mat.SetNumOfColumns( atoi(value.c_str()) );
				}
			}

			if (mat. IsTypeInfoReady())
				break;
		}
	}


	if (!mat.IsTypeInfoReady())
	{
		throw OctaveImportException("no matrix row or column number");
	}

	// read matrix elements
	int currRow = 0;
	int currCol = 0;
	double elem;

	while (m_nCurrLine < m_Lines.size())
	{
		line = m_Lines[m_nCurrLine];
		m_nCurrLine++;
		
		if (line[0] != '#')
		{
			std::istringstream ss(line);
			ss.exceptions (std::ios_base::badbit | std::ios_base::failbit);
			try {
			//ss  <<  lines[i];
				for (currCol = 0; currCol < mat.GetNumOfColumns(); currCol++) 
				{
					ss >> elem;
					mat.SetElement(currRow, currCol, elem);
				
				}
			} catch (const std::ios_base::failure &  ex )  {
				
				throw OctaveImportException("invalid matrix element");
			}
			currRow++;
		}

		// the matrix is ready, so break
		if (( currRow == mat.GetNumOfRows()  ) && ( currCol  == mat.GetNumOfColumns())  )
		{
			mat.SetElemValid(true);
			break;
		}
	} // while

	if (!mat.GetElemValid())
	{
		throw OctaveImportException("matrix element incomplete");
	}

	// a good mat 
	return mat;

}

lhpOpImporterMatrixOctave::HeaderOctave lhpOpImporterMatrixOctave::ReadHeader()
{
	//lhpOctaveMatrixType::m_pointListNameSfx
	// lhpOctaveMatrixType::m_matPoseNameSfx

	std::string line;
	std::string  key;
	std::string value;
	std::string name;
	std::string type;

	lhpOpImporterMatrixOctave::HeaderOctave header;

	while (m_nCurrLine < m_Lines.size())
	{
		line = m_Lines[m_nCurrLine];

		m_nCurrLine++;
		
		// reaader header
		if (line[0] == '#')
		{
			int pos = line.find_first_of(':');

			if (pos >=2)
			{
				key = line.substr(1, pos - 1);
				key = RemoveWhiteSpacesAtEnds(key);

				value = line.substr(pos+1, line.size()  - pos - 1);
				value = RemoveWhiteSpacesAtEnds(value);

				if (key.compare("name") == 0)
				{
					header.name = value;
				}
				else if (key.compare("type") == 0)
				{
					header.type = value;
				}

				if (header.valid())
					break;
			}
		} // if
	}// while

	return header;
}

std::string lhpOpImporterMatrixOctave::RemoveWhiteSpacesAtEnds(const std::string & str) const
{
		// remove white spaces at both ends
	std::string retStr;
	int wsBegin =str.find_first_not_of(' ');
	int wsEnd = str.find_last_not_of(' ');
	if (wsBegin && wsEnd) {
		retStr =str.substr(wsBegin, wsEnd - wsBegin+1);
	}

	return retStr;
}