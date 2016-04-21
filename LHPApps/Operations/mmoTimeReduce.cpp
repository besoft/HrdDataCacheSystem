/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoTimeReduce.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
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

#include "mmoTimeReduce.h"

#include "wx/textfile.h"
#include "wx/arrimpl.cpp"
#include <wx/wxprec.h>
#include "wx/busyinfo.h"
#include <math.h>

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"

#include "mafOpExplodeCollapse.h"

#include "mafSmartPointer.h"
#include "mafMatrixVector.h"
#include "mafDataVector.h"
#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafVMELandmark.h"
#include "mafVMELandmarkCloud.h"

//----------------------------------------------------------------------------
// Required for MSVC
//----------------------------------------------------------------------------
#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

//----------------------------------------------------------------------------
// widget id's
//----------------------------------------------------------------------------
enum 
{
  ID_DEFAULT = MINID,
  ID_NUMBER,
  ID_DELETE,
  ID_LAST,
  ID_FORCED_DWORD = 0x7fffffff,
  ID_HELP
};

//----------------------------------------------------------------------------
mmoTimeReduce::mmoTimeReduce(const wxString& label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType    = OPTYPE_OP;
  m_Canundo   = false;
  m_Delete    = false;
  m_Number    = 2;
}

//----------------------------------------------------------------------------
mmoTimeReduce::~mmoTimeReduce()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
mafOp* mmoTimeReduce::Copy()
//----------------------------------------------------------------------------
{
  mmoTimeReduce *op = new mmoTimeReduce(m_Label);
  op->m_Delete = m_Delete;
  op->m_Number = m_Number;
  return op;
}

//----------------------------------------------------------------------------
bool mmoTimeReduce::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  if(!vme) return false;

  if(mafVMEGenericAbstract::SafeDownCast(vme) == NULL)
  {
    return false;
  }
  if(!mafVMEGenericAbstract::SafeDownCast(vme)->IsAnimated())
  {
    return false;
  }
  if(mafVMELandmarkCloud::SafeDownCast(vme) != NULL)
  {
    if(mafVMELandmarkCloud::SafeDownCast(vme)->IsOpen())
      return false;
  }
  return true;
}


//----------------------------------------------------------------------------
void mmoTimeReduce::OpRun()   
//----------------------------------------------------------------------------
{
  CreateGui();
}

//----------------------------------------------------------------------------
void mmoTimeReduce::CreateGui()
//----------------------------------------------------------------------------
{
  if(m_Gui == NULL)
  {
    char strng[100];
    int nFrames = mafVMEGenericAbstract::SafeDownCast(m_Input)->GetNumberOfLocalTimeStamps();
    sprintf(strng, "Node has %d timestamps", nFrames);
    m_Gui = new mafGUI(this);
    m_Gui->SetListener(this);
	mafEvent buildHelpGui;
	
	buildHelpGui.SetSender(this);
	buildHelpGui.SetId(GET_BUILD_HELP_GUI);
	mafEventMacro(buildHelpGui);

	if (buildHelpGui.GetArg() == true)
	{
		m_Gui->Button(ID_HELP, "Help","");	
	}

    m_Gui->Label(strng);
    m_Gui->Label("Each frame number");
    m_Gui->Integer(ID_NUMBER, "", &(m_Number), 1, nFrames, "This is frame index");
    m_Gui->Bool(ID_DELETE, "will be deleted", &m_Delete, 1, "This is indication to delete or to save frames indicated");
    m_Gui->OkCancel();
  }
  ShowGui();
}

//----------------------------------------------------------------------------
/*void mmoTimeReduce::OpStop(int result)
//----------------------------------------------------------------------------
{
  if (result == OP_RUN_CANCEL)
  {
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
  else if (result == OP_RUN_OK)
  {
    HideGui();
    mafEventMacro(mafEvent(this,result));
  }
}*/
//----------------------------------------------------------------------------
void mmoTimeReduce::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{ 
  switch(maf_event->GetId())
  {
	case ID_HELP:
	{
		mafEvent helpEvent;
		helpEvent.SetSender(this);
		mafString operationLabel = this->m_Label;
		helpEvent.SetString(&operationLabel);
		helpEvent.SetId(OPEN_HELP_PAGE);
		mafEventMacro(helpEvent);
	}
	break;

    case wxOK:          
    { 
      OpStop(OP_RUN_OK);
      break;
    }
    case wxCANCEL:
    {    
      OpStop(OP_RUN_CANCEL);
      break;
    }
    case ID_DELETE:
      {
        break;
      }
    case ID_NUMBER:
      {
        break;
      }
    default:
    {
      mafEventMacro(*maf_event); 
    }
    break;
  }
}

//----------------------------------------------------------------------------
void mmoTimeReduce::OpDo()
//----------------------------------------------------------------------------
{
  //modified by Stefano. 18-9-2003
  wxBusyInfo wait("Please wait, working...");
  std::vector<mafTimeStamp> kframes;
  mafVMEGenericAbstract *vme = mafVMEGenericAbstract::SafeDownCast(m_Input);

  if(m_Number == 0)
    return;
  if(m_Number == 1)
    return;

  vme->GetLocalTimeStamps(kframes);
  if(m_Number == kframes.size() && m_Delete)
    return;

  if(m_Delete)
  {
    mafMatrixVector *mv = vme->GetMatrixVector();
    mafDataVector   *dv = vme->GetDataVector();
    for(int i = 0; i < kframes.size(); i++)
    {
      if(i % m_Number == m_Number - 1)
      {
        mafMatrixVector::TimeMap::iterator itm = mv->FindItem(kframes[i]);
        mafDataVector::TimeMap::iterator   itd = dv->FindItem(kframes[i]);
        if(itm != mv->End())
          mv->RemoveItem(itm);
        if(itd != dv->End())
          dv->RemoveItem(itd);
      }
    }
  }
  else
  {
    mafMatrixVector *mv = vme->GetMatrixVector();
    mafDataVector   *dv = vme->GetDataVector();
    for(int i = 0; i < kframes.size(); i++)
    {
      if(i % m_Number != m_Number - 1)
      {
        mafMatrixVector::TimeMap::iterator itm = mv->FindItem(kframes[i]);
        mafDataVector::TimeMap::iterator   itd = dv->FindItem(kframes[i]);
        if(itm != mv->End())
          mv->RemoveItem(itm);
        if(itd != dv->End())
          dv->RemoveItem(itd);
      }
    }
  }
  return;
}

//----------------------------------------------------------------------------
void mmoTimeReduce::OpUndo()
//----------------------------------------------------------------------------
{
}
