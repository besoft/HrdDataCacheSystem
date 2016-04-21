/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterMatrixMatlab.cpp,v $
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
#include "lhpOpExporterMatrixMatlab.h"
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



const std::string lhpMatlabMatrixType::matPoseNameSfx = "_PoseMatrix";
const std::string lhpMatlabMatrixType::pointListNameSfx = "_DataPoints";
const std::string lhpMatlabMatrixType::faceNameSfx = "_Faces";

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpExporterMatrixMatlab);
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
lhpOpExporterMatrixMatlab::lhpOpExporterMatrixMatlab(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{

	m_OpType  = OPTYPE_EXPORTER;
	m_pVme = NULL;
	m_bUsePoseMatrix = 1;
	m_bUsePointList = 1;

	m_OutputDir = (lhpUtils::lhpGetApplicationDirectory() ).c_str();
	

}

lhpOpExporterMatrixMatlab::~lhpOpExporterMatrixMatlab() 
{

}

//----------------------------------------------------------------------------
bool  lhpOpExporterMatrixMatlab::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	//return (node->IsA("mafVME"));

	return (node->IsA("mafVMESurface") || node->IsA("mafVMELandmarkCloud")
		 || node->IsA("mafVMEOutputSurface")  || node->IsA("mafVMEOutputLandmarkCloud"));
	
}
//----------------------------------------------------------------------------
mafOp*  lhpOpExporterMatrixMatlab::Copy()   
//----------------------------------------------------------------------------
{
   lhpOpExporterMatrixMatlab *cp = new  lhpOpExporterMatrixMatlab(m_Label);
   return cp;
}
//----------------------------------------------------------------------------
void  lhpOpExporterMatrixMatlab::OpRun()   
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
void  lhpOpExporterMatrixMatlab::CreateGui()
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
void  lhpOpExporterMatrixMatlab::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
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

int  lhpOpExporterMatrixMatlab::OnOK()
{
	int ret = OP_RUN_OK;

	mafString wildcard = "Matlab matrix files (*.mat)|*.mat";

	//m_OutputFileNameFullPath = m_OutputDir + "/" +  m_Name;
	m_OutputFileNameFullPath = m_Name.c_str();

	wxString  f = mafGetSaveFile(m_OutputFileNameFullPath,wildcard).c_str(); 

	try {
		if(!f.IsEmpty())
		{
			m_OutputFileNameFullPath = f;
			Write();
		}
		else
			ret = OP_RUN_CANCEL;

	} catch ( MatlabExportException & ex) {
		ret = OP_RUN_CANCEL;
		std::string str = std::string("Error: ") + ex.what();
		wxMessageBox(str.c_str());
		mafLogMessage(str.c_str());
	}

	return ret;
}

//----------------------------------------------------------------------------
void lhpOpExporterMatrixMatlab::Write()
//----------------------------------------------------------------------------
{
	// output to file
	const char * filename = m_OutputFileNameFullPath.c_str();

	/* setup the output */
	mat_t *mat;

	mat = Mat_Open(filename,MAT_ACC_RDWR);

	if(mat)
	{
		if (m_bUsePoseMatrix)
			ExportPoseMatrix(mat);

		if (m_bUsePointList)
			ExportPointList(mat);

		ExportFaces(mat);

		Mat_Close(mat);
	}
	else {
		throw MatlabExportException("File creation failed");
	}

}

void lhpOpExporterMatrixMatlab::ExportPoseMatrix(mat_t * mat)
{
	// all mafMatrix are 4x4 double matrix
	int nRow = 4;
	int nCol = 4;
	double * matElem;
	

	
	m_pVme->GetOutput()->Update();
	mafSmartPointer<mafMatrix> matrix;
	// ? is it right?
	m_pVme->GetOutput()->GetAbsMatrix(*matrix, m_pVme->GetTimeStamp());

	// output matrix
	matvar_t *matvar;
	size_t dims[2] = {nRow,nCol};
	matElem = new double[dims[0] * dims[1] ];

	std::string matName = m_Name + lhpMatlabMatrixType::matPoseNameSfx;

	for (int i = 0; i < nRow; i++)
		for (int j = 0; j < nCol; j++)
		{
			matElem[j * nRow + i] =  matrix->GetElement(i, j);

		}
	

	matvar = Mat_VarCreate(matName.c_str(), MAT_C_DOUBLE,MAT_T_DOUBLE,2,
			     dims,matElem,0);
    Mat_VarWrite( mat, matvar,  MAT_COMPRESSION_NONE);
    Mat_VarFree(matvar);

	delete [] matElem;
	
}

void lhpOpExporterMatrixMatlab::ExportPointList(mat_t * mat)
{
		double point[3];
		int nRow, nCol;
		double * matElem = NULL;
		std::string matName;
		size_t dims[2];

		matName =  m_Name +  lhpMatlabMatrixType::pointListNameSfx;

		// output point data
		if (m_pVme->IsA("mafVMESurface") || m_pVme->IsA("mafVMEOutputSurface"))
		{
			vtkPointSet * pPointSet = vtkPointSet::SafeDownCast(m_pVme->GetOutput()->GetVTKData());

			if (pPointSet)
			{
				nRow = pPointSet->GetNumberOfPoints();
				nCol = 3;

				//matName =  m_Name +  lhpMatlabMatrixType::pointListNameSfx;
				dims[0] = nRow;
				dims[1] = nCol;
				matElem = new double[dims[0] * dims[1] ];

				double point[3];
				for (int i = 0; i < nRow; i++)
				{
					pPointSet->GetPoint(i, point);
					for (int j = 0; j < nCol; j++)
					{
						matElem[j * nRow + i] = point[j];

					}
				}

			}
		}
		else 	if ( m_pVme->IsA("mafVMELandmarkCloud")  || m_pVme->IsA("mafVMEOutputLandmarkCloud") )
		{
				mafVMELandmarkCloud * pLdCloud = mafVMELandmarkCloud::SafeDownCast(m_pVme);
				nRow = pLdCloud->GetNumberOfLandmarks(); 
				nCol = 3;

				dims[0] = nRow;
				dims[1] = nCol;
				matElem = new double[dims[0] * dims[1] ];

				for (int i = 0; i < nRow; i++)
				{
					pLdCloud->GetLandmark(i, point); 

					for (int j = 0; j < nCol; j++)
					{
						matElem[j * nRow + i] = point[j];
					}
				}
				
		}

		if (NULL != matElem) {
			matvar_t *matvar;
			matvar = Mat_VarCreate(matName.c_str(), MAT_C_DOUBLE,MAT_T_DOUBLE,2,
						 dims,matElem,0);
			Mat_VarWrite( mat, matvar,  MAT_COMPRESSION_NONE);
			Mat_VarFree(matvar);
			delete [] matElem;
			matElem = NULL;
		}
}

// WARNING: only support triangles
void lhpOpExporterMatrixMatlab::ExportFaces(mat_t * mat)
{
		//double point[3];
		int nRow, nCol;
		int * matElem = NULL;
		std::string matName;
		size_t dims[2];
		
		
		matName = m_Name +  lhpMatlabMatrixType::faceNameSfx;

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

				dims[0] = nRow;
				dims[1] = nCol;
				matElem = new int[dims[0] * dims[1] ];
			
				pCellArray->InitTraversal();
				for (vtkIdType i = 0; i < nRow; i++)
				{
					pCellArray->GetNextCell(nPoints, pIdx);

					if (nPoints != 3)
						continue;

					for (int j = 0; j < nPoints; j++)
					{
						matElem[j*nRow + i] = pIdx[j];

					}
				}
			}

			if (NULL != matElem) {
				matvar_t *matvar;
				matvar = Mat_VarCreate(matName.c_str(), MAT_C_INT32,MAT_T_INT32,2,
						 dims,matElem,0);
				Mat_VarWrite( mat, matvar,  MAT_COMPRESSION_NONE);
				Mat_VarFree(matvar);

				delete [] matElem;

				matElem = NULL;
			}
		}

}
//----------------------------------------------------------------------------
void  lhpOpExporterMatrixMatlab::OpStop(int result)
//----------------------------------------------------------------------------
{
	HideGui();
	mafEventMacro(mafEvent(this,result)); 
}