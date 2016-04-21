/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoMTRImporter.cpp,v $
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

#include "mmoMTRImporter.h"

#include "mafDecl.h"
#include "mafTagArray.h"
#include "mafVMEGroup.h"
#include "mafMtrLMCReader.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(mmoMTRImporter);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
 mmoMTRImporter:: mmoMTRImporter(const wxString &label) : mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_IMPORTER;
  //m_File    = "";
  m_Canundo = true;
  //m_Group   = NULL;

  m_FileDir = mafGetApplicationDirectory().c_str();
}
//----------------------------------------------------------------------------
 mmoMTRImporter::~mmoMTRImporter()
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Groups.size(); i++)
    mafDEL(m_Groups[i]);
}  
//----------------------------------------------------------------------------
mafOp * mmoMTRImporter::Copy()
//----------------------------------------------------------------------------
{
  mmoMTRImporter *cp = new  mmoMTRImporter(m_Label);
  cp->m_Files   = m_Files;
  cp->m_FileDir = m_FileDir;
  return cp; 
}

//----------------------------------------------------------------------------
void  mmoMTRImporter::OpRun()   
//----------------------------------------------------------------------------
{
  mafString vrml_wildc  = "MTR FARO data (*.mtr)|*.mtr";
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
void  mmoMTRImporter::ImportData()
//----------------------------------------------------------------------------
{

  for(unsigned i = 0; i < m_Groups.size(); i++)
    mafDEL(m_Groups[i]);
  m_Groups.clear();

  for(unsigned fi = 0; fi < m_Files.size(); fi++)
  {
    if(m_Files[fi].IsEmpty())
      continue;
 
    
    wxString vmeName;
    //wxBusyInfo wait("Loading file: ...");  
    wxString path;
    wxString grpName, extension;
    mafVMEGroup *grp;

    wxSplitPath(m_Files[fi].GetCStr(),&path,&grpName,&extension);

    mafNEW(grp);
    grp->SetName(grpName.c_str());
    m_Groups.push_back(grp);

    while(TRUE)
    {
      mafMTRLMCReader *LMCReader = mafMTRLMCReader::New();
      LMCReader->SetFileName(m_Files[fi]);
      wxString name, ext;

      wxSplitPath(m_Files[fi].GetCStr(),&path,&name,&ext);

      LMCReader->SetSet(mafMTRLMCReader::SetNotDefined);
      LMCReader->Execute();

      if(LMCReader->GetPointsRead() == 0)
      {
        cppDEL(LMCReader);
        break;
      }

      const std::vector<std::pair<mafVMELandmarkCloud*, int> >& clouds = LMCReader->GetClouds();
      for(int i = 0; i < clouds.size(); i++)
      {
        vmeName = name + "_";
        wxString  curNumber("");
        curNumber.Printf("%d", i);
        vmeName = vmeName + curNumber;
        mafTagItem tag_Nature;
        tag_Nature.SetName("VME_NATURE");
        tag_Nature.SetValue("NATURAL");

        mafVMELandmarkCloud *cloud;
        cloud = clouds[i].first;
        cloud->SetName(vmeName);
        cloud->GetTagArray()->SetTag(tag_Nature);
        cloud->Close();
        cloud->ReparentTo(grp);
      }
      cppDEL(LMCReader);
      break;
    }

    mafEventMacro(mafEvent(this, VME_ADD, grp));
    mafEventMacro(mafEvent(this,CAMERA_UPDATE));
  }
}

//----------------------------------------------------------------------------
void mmoMTRImporter::OpDo()   
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Groups.size(); i++)
  {
    if (m_Groups[i])
    {
      m_Groups[i]->ReparentTo(m_Input);
      mafEventMacro(mafEvent(this, VME_ADD, m_Groups[i]));
    }
  }
  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}

//----------------------------------------------------------------------------
void mmoMTRImporter::OpUndo()   
//----------------------------------------------------------------------------
{
  for(unsigned i = 0; i < m_Groups.size(); i++)
  {
    if (m_Groups[i])
    {
      mafEventMacro(mafEvent(this, VME_REMOVE, m_Groups[i]));
    }
  }
  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}
