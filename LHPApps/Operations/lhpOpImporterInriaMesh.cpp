/*=========================================================================
Program:   vph2
Module:    $RCSfile: lhpOpImporterInriaMesh.cpp,v $
Language:  C++
Date:      $Date: 2009-06-03 13:26:56 $
Version:   $Revision: 1.1.2.1 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2009
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpImporterInriaMesh.h"
#include <wx/busyinfo.h>

#include "mafVMEMesh.h"
#include "mafTagArray.h"
#include "mafSmartPointer.h"

#include "vtkUnstructuredGrid.h"
#include "vtkUnsignedShortArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpImporterInriaMesh);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpImporterInriaMesh::lhpOpImporterInriaMesh(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_IMPORTER;
  m_Canundo = true;
  m_File    = "";
  m_FileDir = "";//mafGetApplicationDirectory().c_str();
  
  m_UnstructuredGrid = NULL;
}
//----------------------------------------------------------------------------
lhpOpImporterInriaMesh::~lhpOpImporterInriaMesh()
//----------------------------------------------------------------------------
{
  vtkDEL(m_UnstructuredGrid);
}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterInriaMesh::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpImporterInriaMesh *cp = new lhpOpImporterInriaMesh(m_Label);
  cp->m_File			= m_File;
  return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterInriaMesh::OpRun()   
//----------------------------------------------------------------------------
{
  mafString wildc = "vtk Data (*.mesh)|*.mesh";
  mafString f;
  if (m_File.IsEmpty())
  {
    f = mafGetOpenFile(m_FileDir, wildc, _("Choose Mesh file")).c_str();
    m_File = f;
  }

  int result = OP_RUN_CANCEL;
  if(!m_File.IsEmpty())
  {
    if (ImportMesh() == MAF_OK)
    {
      result = OP_RUN_OK;
    }
    else
    {
      if(!this->m_TestMode)
        mafMessage(_("Unsupported file format"), _("I/O Error"), wxICON_ERROR );
    }
  }
  mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
int lhpOpImporterInriaMesh::ImportMesh()
//----------------------------------------------------------------------------
{
  int result = MAF_ERROR;
  if(!this->m_TestMode)
    wxBusyInfo wait(_("Loading file: ..."));

  result = ImportInriaTypeTetrahedriMesh();
  
  if(result == MAF_OK)
  {
    mafSmartPointer<mafVMEMesh> mesh;
    mesh->ReparentTo(m_Input);
    mesh->SetData(m_UnstructuredGrid,mafVME::SafeDownCast(m_Input)->GetTimeStamp());
    mesh->SetName("Medit_Mesh");
    mesh->Update();

    mafTagItem tag_Nature;
    tag_Nature.SetName("VME_NATURE");
    tag_Nature.SetValue("SYNTHETIC");

    mesh->GetTagArray()->SetTag(tag_Nature);
  }

  return result;
}
//----------------------------------------------------------------------------
int lhpOpImporterInriaMesh::ImportInriaTypeTetrahedriMesh()
//----------------------------------------------------------------------------
{
  int result = MAF_ERROR;

  std::ifstream file (m_File.GetCStr());
  char str[256];

  if(file.fail())
  {
    mafLogMessage("Error opening file %s", m_File.GetCStr());
    return result;
  }

  vtkPoints* points = vtkPoints::New();
  vtkUnsignedShortArray* pointarray = vtkUnsignedShortArray::New();
  vtkUnsignedShortArray* cellarray  = vtkUnsignedShortArray::New();
  m_UnstructuredGrid = vtkUnstructuredGrid::New();

  unsigned short ref = 0;

  file >> str;
  
  while((strcmp (str, "Vertices") != 0) && (strcmp (str, "End") != 0) && (strcmp (str, "END") != 0) )
  {
    if (file.fail())
    {
      points->Delete();
      pointarray->Delete();
      cellarray->Delete();
      vtkDEL(m_UnstructuredGrid);
      mafLogMessage("Points not present in file");
      return result;
    }
    file >> str;
  }

  if((strcmp (str, "End") == 0) || (strcmp (str, "END") == 0))
  {
    mafLogMessage("Unexpected end of file");
    points->Delete();
    pointarray->Delete();
    cellarray->Delete();
    vtkDEL(m_UnstructuredGrid);
    return result;
  }

  unsigned int NVertices = 0;
  file >>  NVertices;
  points->SetNumberOfPoints (NVertices);

  pointarray->SetName ("PointArray");
  pointarray->Allocate(NVertices);

  // read vertex position 
  for(unsigned int i=0; i<NVertices; i++)
  {
    double pos[3];
    file >> pos[0] >> pos[1] >> pos[2] >> ref;
    points->SetPoint (i, pos[0], pos[1], pos[2]);
    pointarray->InsertNextValue(ref);
  }

  m_UnstructuredGrid->SetPoints (points);

  if (m_UnstructuredGrid->GetPointData())
  {
    m_UnstructuredGrid->GetPointData()->AddArray (pointarray);
  }

  file >> str;
  
  while(RemoveComment(str, file) && (strcmp (str, "Tetrahedra") != 0) && (strcmp (str, "End") != 0) && (strcmp (str, "END") != 0) )
  {
    if (file.fail())
    {
      points->Delete();
      pointarray->Delete();
      cellarray->Delete();
      vtkDEL(m_UnstructuredGrid);
      mafLogMessage("No tetrahedron in file");
      return result;
    }

    file >> str;
  }

  if((strcmp (str, "End") == 0) || (strcmp (str, "END") == 0) )
  { 
    points->Delete();
    pointarray->Delete();
    cellarray->Delete();
    vtkDEL(m_UnstructuredGrid);
    mafLogMessage("Unexpected end of file");
    return result;
  }

  unsigned int NTetrahedra;

  file >>  NTetrahedra;
  m_UnstructuredGrid->Allocate (NTetrahedra);
  cellarray->SetName("CellArray");
  cellarray->Allocate(NTetrahedra);
  for(unsigned int i=0; i<NTetrahedra; i++)
  {
    unsigned int ids[4];
    file >> ids[0] >> ids[1] >> ids[2] >> ids[3] >> ref;
    vtkIdList* idlist = vtkIdList::New();
    idlist->InsertNextId (ids[0]-1);
    idlist->InsertNextId (ids[1]-1);
    idlist->InsertNextId (ids[2]-1);
    idlist->InsertNextId (ids[3]-1);

    m_UnstructuredGrid->InsertNextCell (VTK_TETRA, idlist);
    idlist->Delete();
    cellarray->InsertNextValue(ref);
  }

  if (m_UnstructuredGrid->GetCellData())
  {
    m_UnstructuredGrid->GetCellData()->AddArray (cellarray);
  }

  points->Delete();
  pointarray->Delete();
  cellarray->Delete();
  
  result = MAF_OK;

  return result;
}
bool lhpOpImporterInriaMesh::RemoveComment(const char *str, std::ifstream &file)
{
  if(strcmp (str, "#") == 0)
  {
    char comment[256];
    file.getline(comment, 256); 
  }
  return true;
}