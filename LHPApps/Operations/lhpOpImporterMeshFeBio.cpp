/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpOpImporterMeshFeBio.cpp,v $
Language:  C++
Date:      $Date: 2010-12-15 15:28:11 $
Version:   $Revision: 1.1.2.4 $
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
#include "lhpOpImporterMeshFeBio.h"

#include <wx/busyinfo.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include "mafGUI.h"

#include "lhpMeshFeBioReader.h"

#include "mafTagArray.h"
#include "mafVMEMesh.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkHexahedron.h"
#include "vtkPointData.h"
#include "vtkQuad.h"
#include "vtkTetra.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWedge.h"

#include <fstream>
#include <iostream>
#include <string>

#include <vnl\vnl_matrix.h>
#include <vnl\vnl_vector.h>
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkCell3D.h"

using namespace std;

#define TAG_FORMAT "LHP_FEB"

//----------------------------------------------------------------------------
lhpOpImporterMeshFeBio::lhpOpImporterMeshFeBio(const wxString &label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_IMPORTER;
  m_Canundo	= true;
  m_File		= "";
  m_FileDir = (mafGetApplicationDirectory() + "/Data/External/").c_str();

  m_Mesh = NULL;
  m_MeshData = NULL;
  m_Cells = NULL;
  m_Points = NULL;

  m_Reader = NULL;
}
//----------------------------------------------------------------------------
lhpOpImporterMeshFeBio::~lhpOpImporterMeshFeBio()
//----------------------------------------------------------------------------
{
  mafDEL(m_Mesh);
  vtkDEL(m_MeshData);
  vtkDEL(m_Cells);
  vtkDEL(m_Points);

  cppDEL(m_Reader);
}
//----------------------------------------------------------------------------
mafOp* lhpOpImporterMeshFeBio::Copy()   
//----------------------------------------------------------------------------
{
  lhpOpImporterMeshFeBio *cp = new lhpOpImporterMeshFeBio(m_Label);
  cp->m_File = m_File;
  cp->m_FileDir = m_FileDir;
  return cp;
}
//----------------------------------------------------------------------------
void lhpOpImporterMeshFeBio::OpRun()   
//----------------------------------------------------------------------------
{
  int result = OP_RUN_CANCEL;
  m_File = "";
  wxString pgd_wildc	= "FEBio File (*.*)|*.*";
  wxString f;
  f = mafGetOpenFile(m_FileDir,pgd_wildc).c_str(); 
  if(!f.IsEmpty() && wxFileExists(f))
  {
    m_File = f;
    Read();
    result = OP_RUN_OK;
  }
  mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void lhpOpImporterMeshFeBio::Read()   
//----------------------------------------------------------------------------
{
  if (!m_TestMode)
  {
    wxBusyInfo wait("Please wait, working...");
    mafEventMacro(mafEvent(this,PROGRESSBAR_SHOW));
  }

  mafNEW(m_Mesh);
  vtkNEW(m_MeshData);
  vtkNEW(m_Cells); 
  vtkNEW(m_Points);
  
  wxString path, name, ext;
  wxSplitPath(m_File.c_str(),&path,&name,&ext);
  m_Mesh->SetName(name);


  mafLogMessage(wxString::Format("start_import %s", name).c_str());

  mafTagItem tag_Nature;
  tag_Nature.SetName("VME_NATURE");
  tag_Nature.SetValue("NATURAL");

  m_Mesh->GetTagArray()->SetTag(tag_Nature);

  wxFileInputStream inputFile( m_File );
  wxTextInputStream text( inputFile );

  m_Reader = new lhpMeshFeBioReader();
  m_Reader->ReadConfigFile(m_File);
 
  if (!m_TestMode)
  {
    mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
  }

  int pointsCounter = 0;

  for(int n = 0; n < m_Reader->GetNumberOfPoints(); n++)
  {
    double coords[3];
    m_Reader->GetPointCoords(n, coords);
    m_Points->InsertPoint(pointsCounter, coords);

    pointsCounter++;
  }

  m_MeshData->SetPoints(m_Points);
  m_MeshData->Update();

  int n_Hex8   = m_Reader->GetNumberOfHex8();
  int n_Penta6 = m_Reader->GetNumberOfPenta6();
  int n_Tet4   = m_Reader->GetNumberOfTeta4();
  int n_Quad4  = m_Reader->GetNumberOfQuad4();
  int n_Tri3  = m_Reader->GetNumberOfTri3();
  
  //////////////////////////////////////////////////////////////////////////
  // HEHAEDRON CELLS
  //////////////////////////////////////////////////////////////////////////
  for(int i=0;i<n_Hex8;i++)
  {
    int pointsId[8];

    vtkHexahedron *hex;
    vtkNEW(hex);

    m_Reader->GetHex8PointsId(i, pointsId);
    for(int j=0;j<8;j++)
    { 
      hex->GetPointIds()->SetId(j,pointsId[j]-1);
    }

    m_Cells->InsertNextCell(hex->GetPointIds());
    m_MeshData->Update();

    vtkDEL(hex);
  }
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // PENTAHEDRON CELLS
  //////////////////////////////////////////////////////////////////////////
  for(int i=0;i<n_Penta6;i++)
  {
    int pointsId[6];

    vtkWedge *wedge;
    vtkNEW(wedge);

    m_Reader->GetPenta6PointsId(i, pointsId);
    for(int j=0;j<6;j++)
    { 
      wedge->GetPointIds()->SetId(j,pointsId[j]-1);
    }

    m_Cells->InsertNextCell(wedge->GetPointIds());
    m_MeshData->Update();

    vtkDEL(wedge);
  }
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // TETRAHEDRON CELLS
  //////////////////////////////////////////////////////////////////////////
  for(int i=0;i<n_Tet4;i++)
  {
    int pointsId[4];

    vtkTetra *tetra;
    vtkNEW(tetra);

    m_Reader->GetTet4PointsId(i, pointsId);
    for(int j=0;j<4;j++)
    { 
      tetra->GetPointIds()->SetId(j,pointsId[j]-1);
    }

    m_Cells->InsertNextCell(tetra->GetPointIds());
    m_MeshData->Update();

    vtkDEL(tetra);
  }
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // QUAD CELLS
  //////////////////////////////////////////////////////////////////////////
  for(int i=0;i<n_Quad4;i++)
  {
    int pointsId[4];

    vtkQuad *quad;
    vtkNEW(quad);

    m_Reader->GetQuad4PointsId(i, pointsId);
    for(int j=0;j<4;j++)
    { 
      quad->GetPointIds()->SetId(j,pointsId[j]-1);
    }

    m_Cells->InsertNextCell(quad->GetPointIds());
    m_MeshData->Update();

    vtkDEL(quad);
  }
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // TRIANGLE CELLS
  //////////////////////////////////////////////////////////////////////////
  for(int i=0;i<n_Tri3;i++)
  {
    int pointsId[3];

    vtkTriangle *triangle;
    vtkNEW(triangle);

    m_Reader->GetTri3PointsId(i, pointsId);
    for(int j=0;j<3;j++)
    { 
      triangle->GetPointIds()->SetId(j,pointsId[j]-1);
    }

    m_Cells->InsertNextCell(triangle->GetPointIds());
    m_MeshData->Update();

    vtkDEL(triangle);
  }
  //////////////////////////////////////////////////////////////////////////

  if(!this->m_TestMode)
  {
    mafEventMacro(mafEvent(this,PROGRESSBAR_HIDE));
  }

  m_MeshData->SetCells(VTK_HEXAHEDRON,m_Cells);  

  m_Mesh->SetData(m_MeshData, 0);

  m_Output = m_Mesh;

  m_Output->ReparentTo(m_Input);
  mafLogMessage("end_import");

}