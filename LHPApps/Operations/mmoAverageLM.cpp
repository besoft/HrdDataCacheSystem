/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoAverageLM.cpp,v $
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

#include "mmoAverageLM.h"

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
#include "mafVMELandmarkCloud.h"
#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafVMELandmark.h"

//----------------------------------------------------------------------------
// Required for MSVC
//----------------------------------------------------------------------------
#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
enum 
{
	ID_DEFAULT = MINID,
	ID_HELP,
};


//----------------------------------------------------------------------------
mmoAverageLM::mmoAverageLM(const wxString& label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType    = OPTYPE_OP;
  m_Canundo   = true;
  m_LimbCloud = NULL;
  m_NewIndex  = 0;
}

//----------------------------------------------------------------------------
mmoAverageLM::~mmoAverageLM()
//----------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------
mafOp* mmoAverageLM::Copy()
//----------------------------------------------------------------------------
{
  return new mmoAverageLM(m_Label);
}

//----------------------------------------------------------------------------
bool mmoAverageLM::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  if(!vme) return false;

  if(!vme->IsA("mafVMELandmarkCloud"))
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
void mmoAverageLM::OpRun()   
//----------------------------------------------------------------------------
{
  m_LimbCloud     = (mafVMELandmarkCloud *)m_Input;
  //CreateGui();
  mafEventMacro(mafEvent(this,OP_RUN_OK)); 
}

//----------------------------------------------------------------------------
void mmoAverageLM::CreateGui()
//----------------------------------------------------------------------------
{
  if(m_Gui == NULL)
  {
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

    m_Gui->Label("");
    m_Gui->OkCancel();
  }
  ShowGui();
}

//----------------------------------------------------------------------------
void mmoAverageLM::OpStop(int result)
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
}
//----------------------------------------------------------------------------
void mmoAverageLM::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{ 
  switch(maf_event->GetId())
  {
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
    default:
    {
      mafEventMacro(*maf_event); 
    }
    break;
  }
}

//----------------------------------------------------------------------------
void mmoAverageLM::OpDo()
//----------------------------------------------------------------------------
{
  wxInt32 nI, nJ;
  std::vector<mafTimeStamp> kframes;

  //modified by Stefano. 18-9-2003
  wxBusyInfo wait("Please wait, working...");

  bool bCloudClosed = !m_LimbCloud->IsOpen();
  if(!bCloudClosed)
  {
    mafOp *OpenOp = new mafOpExplodeCollapse("close cloud");
    OpenOp->SetInput(m_LimbCloud);
    OpenOp->SetListener(m_Listener);
    OpenOp->OpDo();
    cppDEL(OpenOp); 
  }


  m_LimbCloud->GetTimeStamps(kframes);
  char newLMName[100];
  int  avInd = 0;
  sprintf(newLMName, "Average");

  while(m_LimbCloud->FindLandmarkIndex(newLMName) >= 0)
  {
    sprintf(newLMName, "Average%d", avInd);
    avInd++;
  }
  m_NewIndex = m_LimbCloud->AppendLandmark(newLMName);

  //mafProgressBarShowMacro();
  //mafProgressBarSetTextMacro("Creating average landmark...");

  for(nI = 0; nI < kframes.size(); nI++)
  {
    double xa = 0.0, ya = 0.0, za = 0.0;
    //long p = nI * 100 / kframes.size();
    //mafProgressBarSetValueMacro(p);
    for(nJ = 0; nJ < m_LimbCloud->GetNumberOfLandmarks(); nJ++)
    {
      double x, y, z;
      if(m_NewIndex == nJ)
        continue;
      m_LimbCloud->GetLandmark(nJ, x, y, z, kframes[nI]);
      xa += x;
      ya += y;
      za += z;
    }
    xa /= m_LimbCloud->GetNumberOfLandmarks() - 1;
    ya /= m_LimbCloud->GetNumberOfLandmarks() - 1;
    za /= m_LimbCloud->GetNumberOfLandmarks() - 1;
    m_LimbCloud->SetLandmark(m_NewIndex, xa, ya, za, kframes[nI]);
    m_LimbCloud->Modified();
  }
  if(!bCloudClosed)
  {
    mafOp *OpenOp = new mafOpExplodeCollapse("open cloud");
    OpenOp->SetInput(m_LimbCloud);
    OpenOp->SetListener(m_Listener);
    OpenOp->OpDo();
    cppDEL(OpenOp); 
  }
  //mafProgressBarHideMacro(); 

  return;
}



//----------------------------------------------------------------------------
void mmoAverageLM::OpUndo()
//----------------------------------------------------------------------------
{
  bool bCloudWasOpen = m_LimbCloud->IsOpen();
  if(bCloudWasOpen)
  {
    mafOp *pCloseOp = new mafOpExplodeCollapse("close cloud");
    pCloseOp->SetInput(m_LimbCloud);
    pCloseOp->SetListener(m_Listener);
    pCloseOp->OpDo();
    cppDEL(pCloseOp); 
  }

  m_LimbCloud->RemoveLandmark(m_NewIndex);
  m_LimbCloud->Modified();
  
  if(bCloudWasOpen)
  {
    mafOp *OpenOp = new mafOpExplodeCollapse("open cloud");
    OpenOp->SetInput(m_LimbCloud);
    OpenOp->SetListener(m_Listener);
    OpenOp->OpDo();
    cppDEL(OpenOp); 
  }
  //restore previous selection
  mafEventMacro(mafEvent(this,VME_SELECT,m_Input));
}
