/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterMatrixOctave.cpp,v $
  Language:  C++
  Date:      $Date: 2012-03-08 15:51:56 $
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
#include "lhpBuilderDecl.h"
#include "lhpUtils.h"
#include"lhpOpExporterMatrixOctave.h"
#include "mafVME.h"
#include "mafGUI.h"

#include "mafVMELandmarkCloud.h"

#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkIdType.h>

#include <wx/datetime.h>
#include <iostream>
#include <fstream>


const std::string lhpOctaveMatrixType::matPoseNameSfx = "_PoseMatrix";
const std::string lhpOctaveMatrixType::pointListNameSfx = "_DataPoints";
const std::string lhpOctaveMatrixType::faceNameSfx = "_Faces";

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpExporterMatrixOctave);
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
lhpOpExporterMatrixOctave::lhpOpExporterMatrixOctave(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{

	m_OpType  = OPTYPE_EXPORTER;
	m_pVme = NULL;
	m_bUsePoseMatrix = 1;
	m_bUsePointList = 1;

	m_OutputDir = (lhpUtils::lhpGetApplicationDirectory() ).c_str();
	

}

lhpOpExporterMatrixOctave::~lhpOpExporterMatrixOctave() 
{

}

//----------------------------------------------------------------------------
bool  lhpOpExporterMatrixOctave::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node->IsA("mafVME"));
	//return true;
}
//----------------------------------------------------------------------------
mafOp*  lhpOpExporterMatrixOctave::Copy()   
//----------------------------------------------------------------------------
{
   lhpOpExporterMatrixOctave *cp = new  lhpOpExporterMatrixOctave(m_Label);
  return cp;
}
//----------------------------------------------------------------------------
void  lhpOpExporterMatrixOctave::OpRun()   
//----------------------------------------------------------------------------
{
	m_pVme = mafVME::SafeDownCast(m_Input);
	m_pVme->GetOutput()->Update();
	m_Name = m_pVme->GetName();
	
	CreateGui();

}


//----------------------------------------------------------------------------
// Operation constants
//----------------------------------------------------------------------------
enum Octave_Importer_ID
{
  ID_FIRST = MINID,
  //ID_Importer_Type,
  ID_USE_POSEMATRIX,
  ID_USE_POINTLIST,

  ID_OK,
  ID_CANCEL,
};
//----------------------------------------------------------------------------
void  lhpOpExporterMatrixOctave::CreateGui()
//----------------------------------------------------------------------------
{

  m_Gui = new mafGUI(this);
  /*m_Gui->Label("absolute matrix",true);
  m_Gui->Bool(ID_ABS_MATRIX_TO_STL,"apply",&m_ABSMatrixFlag,0);*/
  
  //m_Gui->Label("Export Options", true);
  //m_Gui-> Bool(ID_USE_POSEMATRIX, "PoseMatrix", & m_bUsePoseMatrix, 0, "Export pose matrix to the octave matrix file"	);
  //m_Gui-> Bool(ID_USE_POINTLIST, "Point List", & m_bUsePointList, 0, "Export point list to the octave matrix file"	);

  m_Gui->OkCancel();  
  m_Gui->Divider();

  ShowGui();

}
//----------------------------------------------------------------------------
void  lhpOpExporterMatrixOctave::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
    case wxOK:
      {
        OnOK();
        this->OpStop(OP_RUN_OK);
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

void  lhpOpExporterMatrixOctave::OnOK()
{

	mafString wildcard = "octave matrix files (*.mat)|*.mat|text files (*.txt)|*.txt";

	//m_OutputFileNameFullPath = m_OutputDir + "/" +  m_Name;
	m_OutputFileNameFullPath = m_Name.c_str();

	wxString  f = mafGetSaveFile(m_OutputFileNameFullPath,wildcard).c_str(); 
	if(!f.IsEmpty())
	{
		m_OutputFileNameFullPath = f;
		Write();
	}
}

//----------------------------------------------------------------------------
int lhpOpExporterMatrixOctave::Write()
//----------------------------------------------------------------------------
{
	int ret = 0;
	// output to file
	// std::fstream
	const char * filename = m_OutputFileNameFullPath.c_str();
	std::ofstream outfile;
	
	outfile.open (filename, std::ios_base::trunc);

	if (!outfile)
	{
		 wxMessageBox(_("Error: Open file failed"));
		mafLogMessage("Open file failed");
		return -1;
	}

	outfile.exceptions( std::ios::failbit | std::ios_base::badbit);

	wxDateTime now = wxDateTime::Now();
	
	try {
		// output header
		//outfile << "# Created by NMSBuilder at " << now.Format("%c", wxDateTime::WET).c_str() << std::endl; //, Thu Feb 02 15:51:43 2012 W. Europe Standard Time <unknown@unknown>
		outfile << "# Created by NMSBuilder at " << now.Format("%c").c_str() << std::endl; 
	
		if (m_bUsePoseMatrix)
			ExportPoseMatrix(outfile);

		if (m_bUsePointList)
			ExportPointList(outfile);

		ExportFaces(outfile);

		outfile.close();
	}
	catch (const std::ofstream::failure ex )  {
		std::string msg = std::string("Error: writing file failed: ") + ex.what();
		wxMessageBox(_(msg.c_str()));
		mafLogMessage(msg.c_str());
		ret = -1;
	}

	return ret;
}

void lhpOpExporterMatrixOctave::ExportPoseMatrix(std::ofstream & outfile)
{
		// all mafMatrix are 4x4 double matrix
	int nRow = 4;
	int nCol = 4;
	int ret = 0;
	
	m_pVme->GetOutput()->Update();
	mafSmartPointer<mafMatrix> matrix;
	// ? is it right?
	m_pVme->GetOutput()->GetAbsMatrix(*matrix, m_pVme->GetTimeStamp());

	// output matrix
	nRow = 4;
	nCol = 4;
	outfile <<  std::endl;
	outfile << "# name : " << m_Name + lhpOctaveMatrixType::matPoseNameSfx  << std::endl; 
	outfile << "# type : matrix" << std::endl;
	outfile << "# rows : " << nRow << std::endl;
	outfile << "# columns : " << nCol << std::endl;

	for (int i = 0; i < nRow; i++)
	{
		for (int j = 0; j < nCol; j++)
		{
			outfile << matrix->GetElement(i, j);
			if (j < nCol - 1)
				outfile <<  " ";
			else
				outfile <<  std::endl;
		}
	}
}

void lhpOpExporterMatrixOctave::ExportPointList(std::ofstream & outfile)
{
		double point[3];
		int nRow, nCol;
		// output point data
		if (m_pVme->IsA("mafVMESurface") || m_pVme->IsA("mafVMEOutputSurface"))
		{
			vtkPointSet * pPointSet = vtkPointSet::SafeDownCast(m_pVme->GetOutput()->GetVTKData());

			if (pPointSet)
			{
				nRow = pPointSet->GetNumberOfPoints();
				nCol = 3;
				outfile <<  std::endl;
				outfile << "# name : " << m_Name +  lhpOctaveMatrixType::pointListNameSfx  << std::endl; 
				outfile << "# type : matrix" << std::endl;
				outfile << "# rows : " << nRow << std::endl;
				outfile << "# columns : " << nCol << std::endl;

				double point[3];
				for (int i = 0; i < nRow; i++)
				{
					pPointSet->GetPoint(i, point);
					for (int j = 0; j < nCol; j++)
					{
							outfile << point[j];

							if (j < nCol -1)
								outfile <<  " ";
							else
								outfile <<  std::endl;
					}
				}
			}
		}
		else 	if ( m_pVme->IsA("mafVMELandmarkCloud")  || m_pVme->IsA("mafVMEOutputLandmarkCloud") )
		{
				mafVMELandmarkCloud * pLdCloud = mafVMELandmarkCloud::SafeDownCast(m_pVme);
				nRow = pLdCloud->GetNumberOfLandmarks(); 
				nCol = 3;
			
				outfile << "# name : " << m_Name + lhpOctaveMatrixType::pointListNameSfx  << std::endl; 
				outfile << "# type : matrix" << std::endl;
				outfile << "# rows : " << nRow << std::endl;
				outfile << "# columns : " << nCol << std::endl;

				for (int i = 0; i < nRow; i++)
				{
					pLdCloud->GetLandmark(i, point); 

					for (int j = 0; j < nCol; j++)
					{
							outfile << point[j];

							if (j < nCol -1)
								outfile <<  " ";
							else
								outfile <<  std::endl;
					}
				}
		}
}

// WARNING: only support triangles
void lhpOpExporterMatrixOctave::ExportFaces(std::ofstream & outfile)
{
		//double point[3];
		int nRow, nCol;
		// output point data
		if (m_pVme->IsA("mafVMESurface") || m_pVme->IsA("mafVMEOutputSurface"))
		{
			vtkPolyData * pPoly = vtkPolyData::SafeDownCast(m_pVme->GetOutput()->GetVTKData());

			if (pPoly)
			{
				vtkCellArray * pCellArray = pPoly->GetPolys();
				nRow = pCellArray->GetNumberOfCells();
				// WARNING: only support triangles
				nCol = pCellArray->GetMaxCellSize();
				
				vtkIdType * pIdx = NULL;;
				vtkIdType   nPoints;
				int nTriangles = 0;
				pCellArray->InitTraversal();
				for (vtkIdType i = 0; i < nRow; i++)
				{
					pCellArray->GetNextCell(nPoints, pIdx);
					if (nPoints == 3)
						nTriangles++;
				}

				nRow = nTriangles;
				nCol = 3;

				outfile <<  std::endl;
				outfile << "# name : " << m_Name +  lhpOctaveMatrixType::faceNameSfx  << std::endl; 
				outfile << "# type : matrix" << std::endl;
				outfile << "# rows : " << nRow << std::endl;
				outfile << "# columns : " << nCol << std::endl;

				pCellArray->InitTraversal();
				for (vtkIdType i = 0; i < nRow; i++)
				{
					pCellArray->GetNextCell(nPoints, pIdx);

					if (nPoints != 3)
						continue;

					for (int j = 0; j < nPoints; j++)
					{
							outfile << pIdx[j];

							if (j < nCol -1)
								outfile <<  " ";
							else
								outfile <<  std::endl;
					}
				}
			}
		}

}
//----------------------------------------------------------------------------
void  lhpOpExporterMatrixOctave::OpStop(int result)
//----------------------------------------------------------------------------
{
	HideGui();
	mafEventMacro(mafEvent(this,result)); 
}