/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoINPImporter.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev / Vladik Aranov
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mmoINPImporter.h"

#include "mafDecl.h"
#include "mafGUIDialog.h"

#include "mmaMaterial.h"

#include "mafINPReader.h"
#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafVMEGroup.h"
#include "mafTagArray.h"

#include "vtkMAFSmartPointer.h"
#include "vtkPolyData.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(mmoINPImporter);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
 mmoINPImporter::mmoINPImporter(const wxString &label) : mafOp(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_IMPORTER;
	//m_File		= "";
	m_Canundo	= true;
	//m_Surface = NULL;

 	m_FileDir = mafGetApplicationDirectory().c_str();
}
//----------------------------------------------------------------------------
 mmoINPImporter::~mmoINPImporter()
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Surfaces.size(); i++)
    mafDEL(m_Surfaces[i]);
}
//----------------------------------------------------------------------------
mafOp * mmoINPImporter::Copy()
//----------------------------------------------------------------------------
{
  mmoINPImporter *cp = new  mmoINPImporter(m_Label);
  cp->m_Files   = m_Files;
  cp->m_FileDir = m_FileDir;
  return cp; 
}
//----------------------------------------------------------------------------
void  mmoINPImporter::OpRun()   
//----------------------------------------------------------------------------
{
  mafString vrml_wildc = "AMIRA geometry (*.inp)|*.inp|AMIRA geometry in AF system (*.inp_AFs)|*.inp_AFs";
  std::vector<std::string> files;
  mafString f;

  m_Files.clear();
  //if (m_File.IsEmpty())
  {
    mafGetOpenMultiFiles(m_FileDir.GetCStr(),vrml_wildc.GetCStr(), files);
    for(unsigned i = 0; i < files.size(); i++)
    {
      f = files[i].c_str();
      m_Files.push_back(f);
    }
  }

  int result = OP_RUN_CANCEL;

  if(m_Files.size() != 0) 
  {
    result = OP_RUN_OK;
    ImportData();
  }

  mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
void  mmoINPImporter::ImportData()
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Surfaces.size(); i++)
    mafDEL(m_Surfaces[i]);
  m_Surfaces.clear();
  for(unsigned i = 0; i < m_Files.size(); i++)
  {
    if(m_Files[i].IsEmpty())
      continue;
    mafINPReader  *reader = mafINPReader::New();
    mafVMESurface *surface;

    reader->SetFileName(m_Files[i]);
    wxString path, name, ext;

    wxSplitPath(m_Files[i].GetCStr(),&path,&name,&ext);

    reader->Update();

    mafTimeStamp t;
    t = ((mafVME *)m_Input)->GetTimeStamp();
    mafNEW(surface);
    surface->SetName(name.c_str());
    vtkPolyData *data = reader->GetOutput();
    surface->SetData(data,t);

    m_Surfaces.push_back(surface);

    mafTagItem tag_Nature;
    tag_Nature.SetName("VME_NATURE");
    tag_Nature.SetValue("NATURAL");

    surface->GetTagArray()->SetTag(tag_Nature);

    vtkDEL(reader);
  }
}
//----------------------------------------------------------------------------
void mmoINPImporter::OpDo()   
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Surfaces.size(); i++)
  {
    if (m_Surfaces[i])
    {
      m_Surfaces[i]->ReparentTo(m_Input);
      mafEventMacro(mafEvent(this, VME_ADD, m_Surfaces[i]));
    }
  }
  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}

//----------------------------------------------------------------------------
void mmoINPImporter::OpUndo()   
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Surfaces.size(); i++)
  {
    if (m_Surfaces[i])
    {
      mafEventMacro(mafEvent(this, VME_REMOVE, m_Surfaces[i]));
    }
  }
  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}
