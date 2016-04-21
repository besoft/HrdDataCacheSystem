/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpCompareBonemat.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
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


#include "lhpOpCompareMesh.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafVMEMesh.h"
#include "vtkDoubleArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataArray.h"
#include "vtkCellData.h"
#include "vtkMAFSmartPointer.h"
#include "vtkPointData.h"
#include "vtkFieldData.h"
#include "mafGUI.h"


//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpCompareMesh);
//----------------------------------------------------------------------------

mafNode *staticCurrentNode;

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

enum COMPARE_MODE
{
	DIFF,
	ABS_DIFF,
	RELATIVE_DIFF,
};

//----------------------------------------------------------------------------
lhpOpCompareMesh::lhpOpCompareMesh(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
	m_Gui=NULL;
	m_OutputMesh = NULL;
	m_CompareMode = 0;
  m_Canundo = false;
}
//----------------------------------------------------------------------------
lhpOpCompareMesh::~lhpOpCompareMesh() 
//----------------------------------------------------------------------------
{
  
}

//----------------------------------------------------------------------------
mafOp* lhpOpCompareMesh::Copy()   
//----------------------------------------------------------------------------
{
	return new lhpOpCompareMesh(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpCompareMesh::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && mafVMEMesh::SafeDownCast(node));
}

//----------------------------------------------------------------------------
bool lhpOpCompareMesh::AcceptMesh(mafNode *node)
//----------------------------------------------------------------------------
{
	return (node && node != staticCurrentNode  && mafVMEMesh::SafeDownCast(node));
}
	
//----------------------------------------------------------------------------
void lhpOpCompareMesh::OpRun()   
//----------------------------------------------------------------------------
{
	m_MeshB=SelectMesh();

	if (!m_MeshB)
	{
		wxMessageBox("Target mesh not selected", _("Warning"), wxOK | wxICON_WARNING  ); 

		OpStop(OP_RUN_CANCEL);
		return;
	}

	m_MeshA=mafVMEMesh::SafeDownCast(m_Input);
	m_MeshA->GetUnstructuredGridOutput()->Update();
	vtkUnstructuredGrid *ugA=vtkUnstructuredGrid::SafeDownCast(m_MeshA->GetUnstructuredGridOutput()->GetVTKData());
	ugA->Update();
	
	m_MeshB->GetUnstructuredGridOutput()->Update();
	vtkUnstructuredGrid *ugB=vtkUnstructuredGrid::SafeDownCast(m_MeshB->GetUnstructuredGridOutput()->GetVTKData());
	ugB->Update();

	if(ugA->GetNumberOfCells() != ugB->GetNumberOfCells())
	{
		wxMessageBox("Target mesh does not have the same number of cells", _("Warning"), wxOK | wxICON_WARNING  ); 

		OpStop(OP_RUN_CANCEL);
		return;
	}

	if(ugA->GetNumberOfPoints() != ugB->GetNumberOfPoints())
	{
		wxMessageBox( "Target mesh does not have the same number of points", _("Warning"), wxOK | wxICON_WARNING  ); 
		OpStop(OP_RUN_CANCEL);
		return;
	}

	CreateGui();

	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpCompareMesh::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{
		case wxOK:
			{
				Execute();

				if(m_OutputMesh)
				{
					//mafEventMacro(mafEvent(this,VME_SELECT, m_OutputMesh, true));
					//mafEventMacro(mafEvent(this,VME_SHOW, m_OutputMesh, true));
					OpStop(OP_RUN_OK);
				}
				else
				{
					OpStop(OP_RUN_CANCEL);
				}
			}
			break;
		case wxCANCEL:
			OpStop(OP_RUN_CANCEL);
			break;
		}
	}
}

//----------------------------------------------------------------------------
void lhpOpCompareMesh::CompareArray(vtkDataArray *arrayA, vtkDataArray *arrayB, vtkDataArray *dest)
//----------------------------------------------------------------------------
{
	int nTuples=arrayA->GetNumberOfTuples();

	double minDiff=MAXDOUBLE;
	double maxDiff=MINDOUBLE;

	dest->SetNumberOfTuples(nTuples);
	dest->SetName(arrayA->GetName());
			
	for(int i=0;i<nTuples;i++)
	{
		double aValue=arrayA->GetTuple1(i);
		double bValue=arrayB->GetTuple1(i);

		double diff;
		double maxValue;
		switch (m_CompareMode)
		{
		case ABS_DIFF:
			diff=abs(aValue-bValue);
		break;
		case DIFF:
			diff=(aValue-bValue);
			break;
		case RELATIVE_DIFF:
			diff= abs(aValue-bValue)  /  ((aValue+bValue)/2.0);
		default:
			break;
		}

		minDiff=min(minDiff,diff);
		maxDiff=max(maxDiff,diff);
		
		dest->SetTuple1(i,diff);
	}

	mafLogMessage("Array %s  min diff:%lf  max diff: %lf",arrayA->GetName(), minDiff,maxDiff );

}

//---------------------------------------------------------------------------
void lhpOpCompareMesh::CreateMeshCopy()
//----------------------------------------------------------------------------
{
	mafString outputName = m_Input->GetName();
	outputName += " Compare";
	mafNEW(m_OutputMesh);
	m_OutputMesh->DeepCopy(mafVMEMesh::SafeDownCast(m_Input));
	m_OutputMesh->SetName(outputName);
	m_OutputMesh->ReparentTo(m_Input->GetParent());
	m_OutputMesh->Update();
}

//----------------------------------------------------------------------------
void lhpOpCompareMesh::Execute()
//----------------------------------------------------------------------------
{
	vtkUnstructuredGrid *ugA=vtkUnstructuredGrid::SafeDownCast(m_MeshA->GetUnstructuredGridOutput()->GetVTKData());
	vtkUnstructuredGrid *ugB=vtkUnstructuredGrid::SafeDownCast(m_MeshB->GetUnstructuredGridOutput()->GetVTKData());

	CreateMeshCopy();

	vtkMAFSmartPointer<vtkUnstructuredGrid> outputUG;
	outputUG->SetPoints(ugA->GetPoints());
	outputUG->SetCells(ugA->GetCellTypesArray(),ugA->GetCellLocationsArray(),ugA->GetCells());
	outputUG->Update();

	mafLogMessage("Comparing Cell data arrays...");
	CompareFieldData(ugA->GetCellData(), ugB->GetCellData(),outputUG->GetCellData());

	mafLogMessage("Comparing Point data arrays...");
	outputUG->GetPointData()->SetScalars(ugA->GetPointData()->GetScalars());
	CompareFieldData(ugA->GetPointData(), ugB->GetPointData(),outputUG->GetPointData());
		
	outputUG->Modified();
	outputUG->Update();

	mafVMEMesh::SafeDownCast(m_OutputMesh)->SetData(outputUG, 0);

}

//----------------------------------------------------------------------------
mafVMEMesh* lhpOpCompareMesh::SelectMesh()
//----------------------------------------------------------------------------
{
	staticCurrentNode = m_Input;

	mafString title = _("Choose Mesh to compare");
	mafEvent *e; 
	e = new mafEvent();
	e->SetId(VME_CHOOSE);
	e->SetArg((long)&AcceptMesh);
	e->SetString(&title);

	mafEventMacro(*e);

	mafNode *n = e->GetVme();
	delete e;

	return mafVMEMesh::SafeDownCast(n);
}

//----------------------------------------------------------------------------
int lhpOpCompareMesh::IsArrayToCopy(mafString arrayName)
//----------------------------------------------------------------------------
{
	char *arrayNamesToCopy[4] = {"id","ANSYS_ELEMENT_ID","ANSYS_ELEMENT_TYPE","ANSYS_ELEMENT_REAL"};
	int nArray=4;

	return IsNameInArrayList(nArray, arrayName, arrayNamesToCopy);
}

//----------------------------------------------------------------------------
int lhpOpCompareMesh::IsArrayToExclude(mafString arrayName)
//----------------------------------------------------------------------------
{
	char *arrayNamesToCopy[2] = {"material_id","material"};
	int nArray=2;

	return IsNameInArrayList(nArray, arrayName, arrayNamesToCopy);
}

//----------------------------------------------------------------------------
void lhpOpCompareMesh::CompareFieldData(vtkFieldData *fieldDataA, vtkFieldData *fieldDataB, vtkFieldData *fieldDataC)
//----------------------------------------------------------------------------
{
	for(int i=0;i<fieldDataA->GetNumberOfArrays();i++)
	{
		vtkDataArray *arrayA, *arrayB, *arrayC;
		arrayA=fieldDataA->GetArray(i);
		mafString arrayName=arrayA->GetName();

		if(IsArrayToExclude(arrayName))
		{
			continue;
		}

		if(IsArrayToCopy(arrayName))
		{
			fieldDataC->AddArray(arrayA);
			continue;
		}

		arrayB=fieldDataB->GetArray(arrayName.GetCStr());
		if(arrayB == NULL)
		{
			mafLogMessage("Target mesh does not have %s data array, skypping compare",arrayName.GetCStr());
			continue;
		}

		if(arrayA->GetNumberOfTuples() != arrayB->GetNumberOfTuples())
		{
			mafLogMessage("Target mesh array %s does not have the same number of elements of source mesh", arrayName.GetCStr());
			continue;
		}
		
		arrayC=arrayA->NewInstance();
		CompareArray(arrayA,arrayB,arrayC);
		fieldDataC->AddArray(arrayC);
		vtkDEL(arrayC);
	}
}

//----------------------------------------------------------------------------
void lhpOpCompareMesh::CreateGui()
//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->Label("", true);

	const wxString compareModeStrings[] = {"Difference","Absolute difference","Relative: abs(a-b) / mean(a,b)"};
	m_Gui->Label("Comparison Mode:",true);
	m_Gui->Combo(-1, "", &m_CompareMode, 3, compareModeStrings);
	m_Gui->Label("");
	m_Gui->Label("");
	m_Gui->Label("");
	m_Gui->Label("");
	m_Gui->Label("");
	m_Gui->Label("");
	m_Gui->Divider(true);
	m_Gui->OkCancel();
}

//----------------------------------------------------------------------------
int lhpOpCompareMesh::IsNameInArrayList(int nArray, mafString arrayName, char ** arrayNamesToCopy)
//----------------------------------------------------------------------------
{
	for (int i=0;i<nArray;i++)
		if (arrayName==arrayNamesToCopy[i])
			return true;

	return false;
}

//----------------------------------------------------------------------------
void lhpOpCompareMesh::OpStop(int result)
//----------------------------------------------------------------------------
{
	if (result==OP_RUN_CANCEL && m_OutputMesh)
		m_OutputMesh->ReparentTo(NULL);

	mafDEL(m_OutputMesh);

	if(m_Gui)
		HideGui();

	mafEventMacro(mafEvent(this,result));        
}