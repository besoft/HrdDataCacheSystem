/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpExporterAnsysCommon.cpp,v $
  Language:  C++
  Date:      $Date: 2010-12-03 14:58:16 $
  Version:   $Revision: 1.1.1.1.2.3 $
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

#include "lhpUtils.h"
#include "lhpBuilderDecl.h"

#include "lhpOpExporterAnsysCommon.h"

#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mafGUI.h"

#include "mafSmartPointer.h"
#include "mafTagItem.h"
#include "mafTagArray.h"
#include "mafVME.h"
#include "mafVMEMesh.h"
#include "mafVMEMeshAnsysTextExporter.h"
#include "mafAbsMatrixPipe.h"

#include <iostream>
#include <fstream>

// vtk includes
#include "vtkMAFSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkFieldData.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"

// vcl includes
#include <vcl_string.h>
#include <vcl_fstream.h>
#include <vcl_sstream.h>
#include <vcl_map.h>
#include <vcl_vector.h>

//----------------------------------------------------------------------------
lhpOpExporterAnsysCommon::lhpOpExporterAnsysCommon(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_EXPORTER;
  m_Canundo = true;
  m_ImporterType = 0;
  m_ImportedVmeMesh = NULL;

  m_AnsysOutputFileNameFullPath		= "";
  m_FileDir = (lhpUtils::lhpGetApplicationDirectory() + "/Data/External/").c_str();

  m_Pid = -1;
  m_ABSMatrixFlag = 1;

  m_BusyInfo = NULL;
}
//----------------------------------------------------------------------------
lhpOpExporterAnsysCommon::~lhpOpExporterAnsysCommon()
//----------------------------------------------------------------------------
{
  mafDEL(m_ImportedVmeMesh);
}

//----------------------------------------------------------------------------
bool lhpOpExporterAnsysCommon::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node->IsA("mafVMEMesh"));
}
//----------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
}
//----------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::OnEvent(mafEventBase *maf_event) 
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
//----------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::OnOK()
//----------------------------------------------------------------------------
{
  mafString wildcard = GetWildcard();

  m_AnsysOutputFileNameFullPath = "";

  wxString f;
  f = mafGetSaveFile(m_FileDir,wildcard).c_str(); 
  if(!f.IsEmpty())
  {
    m_AnsysOutputFileNameFullPath = f;
    Write();
  }
}
//----------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::OpStop(int result)
//----------------------------------------------------------------------------
{
  HideGui();
  mafEventMacro(mafEvent(this,result));        
}

//----------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->Label("Absolute matrix",true);
  m_Gui->Bool(ID_ABS_MATRIX_TO_STL,"Apply",&m_ABSMatrixFlag,0);
  m_Gui->OkCancel();  
  m_Gui->Divider();

  ShowGui();
}

//---------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::InitProgressBar(wxString label = "")
//----------------------------------------------------------------------------
{
  m_OperationProgress = 0;

  if (GetTestMode() == false)
  {
    m_BusyInfo = new wxBusyInfo(label);
    mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
  }
  else
  {
    printf("\n" + label + "\n");
    printf("%c", 179);
  }

  mafVMEMesh *input = mafVMEMesh::SafeDownCast(m_Input);
  assert(input);

  vtkUnstructuredGrid *inputUGrid = input->GetUnstructuredGridOutput()->GetUnstructuredGridData();

  if(inputUGrid != NULL)
  {
    // Calculate Num of Elements
    m_TotalElements = inputUGrid->GetNumberOfPoints(); // Points

    m_TotalElements += inputUGrid->GetNumberOfCells(); // Elements

    vtkDataArray *materialsIDArray = NULL;
    materialsIDArray = inputUGrid->GetFieldData()->GetArray("material_id");

    if(materialsIDArray == NULL)
      materialsIDArray = inputUGrid->GetCellData()->GetScalars("material_id");  

    if(materialsIDArray != NULL)
      m_TotalElements += materialsIDArray->GetNumberOfTuples(); // Materials
  }
}
//---------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::UpdateProgressBar(long progress)
//----------------------------------------------------------------------------
{
  if (GetTestMode() == false)
  {
    if(progress != m_OperationProgress)
    {
      m_OperationProgress = progress;
      mafEventMacro(mafEvent(this,PROGRESSBAR_SET_VALUE,progress));
    }
  }
  else
  {
    while(progress - m_OperationProgress > 2)
    {
      m_OperationProgress += 2;
      printf("%c", 177);
    }
  }
}
//---------------------------------------------------------------------------
void lhpOpExporterAnsysCommon::CloseProgressBar()
//----------------------------------------------------------------------------
{
  if (GetTestMode() == false)
  {
    cppDEL(m_BusyInfo);
    mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
  }
  else
  {
    printf("%c\n", 179);
  }

  m_OperationProgress = 0;
}

//----------------------------------------------------------------------------
int lhpOpExporterAnsysCommon::compareElem(const void *p1, const void *p2) 
//----------------------------------------------------------------------------
{
  ExportElement *a, *b;
  a = (ExportElement *)p1;
  b = (ExportElement *)p2;

  double result;

  result = a->elementType - b->elementType;  
  if (result < 0)
    return -1;
  else if (result > 0)
    return 1;
  else
  {
    result = a->matID - b->matID;  
    if (result < 0)
      return -1;
    else if (result > 0)
      return 1;
    else
      return 0;
  }
}

//----------------------------------------------------------------------------
long lhpOpExporterAnsysCommon::GetPid()   
//----------------------------------------------------------------------------
{
  return m_Pid;
}

