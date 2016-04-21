/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpOpExporterMeshFeBio.cpp,v $
Language:  C++
Date:      $Date: 2010-12-15 15:23:16 $
Version:   $Revision: 1.1.2.1 $
Authors:   Eleonora Mambrini
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

#include "lhpBuilderDecl.h"
#include "lhpUtils.h"
#include "lhpOpExporterMeshFeBio.h"
#include "lhpMeshFeBioWriter.h"

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

#include "vtkMAFSmartPointer.h"

#include <iostream>
#include <fstream>
#include "vtkGenericCell.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpExporterMeshFeBio);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpExporterMeshFeBio::lhpOpExporterMeshFeBio(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{

  m_OpType  = OPTYPE_EXPORTER;
  m_Canundo = true;
  //m_ImporterType = 0;
  m_ImportedVmeMesh = NULL;
 
  m_CacheDir = (lhpUtils::lhpGetApplicationDirectory() + "\\Data\\FeBioWriterCache").c_str();
  m_FebBioOutputFileNameFullPath		= "";
  m_FileDir = (lhpUtils::lhpGetApplicationDirectory() + "/Data/External/").c_str();

  m_Writer = new lhpMeshFeBioWriter();;
}

enum ANSYS_EXPORTER_ID
{
  ID_ABS_MATRIX_TO_STL = MINID,  
};

//----------------------------------------------------------------------------
lhpOpExporterMeshFeBio::~lhpOpExporterMeshFeBio()
//----------------------------------------------------------------------------
{
  mafDEL(m_ImportedVmeMesh);

  delete m_Writer;
}
//----------------------------------------------------------------------------
bool lhpOpExporterMeshFeBio::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
  return (node->IsA("mafVMEMesh"));
}
//----------------------------------------------------------------------------
mafOp* lhpOpExporterMeshFeBio::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpExporterMeshFeBio *cp = new lhpOpExporterMeshFeBio(m_Label);
  return cp;
}
//----------------------------------------------------------------------------
void lhpOpExporterMeshFeBio::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
}
//----------------------------------------------------------------------------
int lhpOpExporterMeshFeBio::Write()
//----------------------------------------------------------------------------
{
  if (!m_TestMode)
  {
    wxBusyInfo wait(_("Writing file: ..."));
  }

  bool cacheDirExist = wxDirExists(m_CacheDir.GetCStr());

  if (cacheDirExist == false)
  {
    std::ostringstream stringStream;
    stringStream << "creating cache dir: " << m_CacheDir.GetCStr() << std::endl;
    mafLogMessage(stringStream.str().c_str());          
    wxMkdir(m_CacheDir.GetCStr());
  }
  else
  {
    std::ostringstream stringStream;
    stringStream << "found cache dir: " << m_CacheDir.GetCStr() << std::endl;
    mafLogMessage(stringStream.str().c_str());

  }

  // check for cache dir existence
  assert(wxDirExists(m_CacheDir.GetCStr()));

  mafVMEMesh *inMesh = mafVMEMesh::SafeDownCast(m_Input);
  assert(inMesh);

  //////////////////////////////////////////////////////////////////////////
  // Retrieve data from input mesh
  //////////////////////////////////////////////////////////////////////////


  //POINTS
  int numberOfPoints;
  double point[3];

  vtkUnstructuredGrid *data = (vtkUnstructuredGrid*)inMesh->GetUnstructuredGridOutput()->GetVTKData();

  vtkPoints *points = data->GetPoints();
  numberOfPoints = points->GetNumberOfPoints();
  m_Writer->SetNumberOfPoints(numberOfPoints);

  for(int i = 0;i<numberOfPoints;i++)
  {
    points->GetPoint(i, point);
    m_Writer->SetPointCoords(i+1, point);
  }

  //CELLS
  
  int numberOfCells;
  int hex8Counter, penta6Counter, tet4Counter, quad4Counter, tri3Counter;

  hex8Counter = 0;
  penta6Counter = 0;
  tet4Counter = 0;
  quad4Counter = 0;
  tri3Counter = 0;


  numberOfCells = data->GetNumberOfCells();
  vtkGenericCell *cell;
  vtkIdList *list;

  vtkNEW(cell);

  for(int i=0;i<numberOfCells;i++)
  {
    data->GetCell(i, cell);

    if( (vtkHexahedron*)cell )
    {
      int hex8Points[8];

      list = cell->GetPointIds();
      for(int j =0;j<8;j++)
        hex8Points[j] = list->GetId(j);
      m_Writer->SetHex8PointsId(hex8Counter+1, hex8Points);
      hex8Counter++;

    }
    else if( (vtkWedge*)cell )
    {
      int penta6Points[6];

      list = cell->GetPointIds();
      for(int j =0;j<6;j++)
        penta6Points[j] = list->GetId(j);

      m_Writer->SetPenta6PointsId(penta6Counter+1, penta6Points);
      penta6Counter++;

    }
    else if( (vtkTetra*)cell )
    {
      int tet4Points[4];

      list = cell->GetPointIds();
      for(int j =0;j<4;j++)
        tet4Points[j] = list->GetId(j);

      m_Writer->SetTet4PointsId(tet4Counter+1, tet4Points);
      tet4Counter++;

    }
    else if( (vtkQuad*)cell )
    {
      int quad4Points[4];

      list = cell->GetPointIds();
      for(int j =0;j<4;j++)
        quad4Points[j] = list->GetId(j);
      m_Writer->SetQuad4PointsId(quad4Counter+1, quad4Points);
      quad4Counter++;

    }
    else if( (vtkTriangle*)cell )
    {
      int tri3Points[3];

      list = cell->GetPointIds();
      for(int j =0;j<3;j++)
        tri3Points[j] = list->GetId(j);
      m_Writer->SetTri3PointsId(tri3Counter+1, tri3Points);
      tri3Counter++;

    }
  }
  vtkDEL(cell);

  //////////////////////////////////////////////////////////////////////////


  int result = MAF_OK;

  m_Writer->Write(m_FebBioOutputFileNameFullPath);

  if (result == MAF_OK)
  {
    // continue
  } 
  else
  {
    wxMessageBox("Problems generating intermediate output files! \n\
                 See the log area for more details. ");
    return MAF_ERROR;
  }

  wxArrayString output;
  wxArrayString errors;

  // wRITE

  for (int i = 0; i < output.GetCount(); i++)
  {
    mafLogMessage(output[i].c_str());
  }


  return MAF_OK;

}
//----------------------------------------------------------------------------
// Operation constants
//----------------------------------------------------------------------------
enum Mesh_Importer_ID
{
  ID_FIRST = MINID,
  ID_Importer_Type,
  ID_NodesFileName,
  ID_ElementsFileName,
  ID_MaterialsFileName,
  ID_OK,
  ID_CANCEL,
};
//----------------------------------------------------------------------------
void lhpOpExporterMeshFeBio::CreateGui()
//----------------------------------------------------------------------------
{
  //mafString wildc = "Stereo Litography (*.stl)|*.stl";

  m_Gui = new mafGUI(this);
  /*m_Gui->Label("absolute matrix",true);
  m_Gui->Bool(ID_ABS_MATRIX_TO_STL,"apply",&m_ABSMatrixFlag,0);*/
  m_Gui->OkCancel();  
  m_Gui->Divider();

  ShowGui();

}
//----------------------------------------------------------------------------
void lhpOpExporterMeshFeBio::OnEvent(mafEventBase *maf_event) 
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

void lhpOpExporterMeshFeBio::OnOK()
{
  mafString wildcard = "feb files (*.feb)|*.feb|All Files (*.*)|*.*";

  m_FebBioOutputFileNameFullPath = "";

  wxString f;
  f = mafGetSaveFile(m_FileDir,wildcard).c_str(); 
  if(!f.IsEmpty())
  {
    m_FebBioOutputFileNameFullPath = f;
    Write();
  }
}

//----------------------------------------------------------------------------
void lhpOpExporterMeshFeBio::OpStop(int result)
//----------------------------------------------------------------------------
{
  HideGui();
  mafEventMacro(mafEvent(this,result));        
}