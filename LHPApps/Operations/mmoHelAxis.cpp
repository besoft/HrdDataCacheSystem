/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoHelAxis.cpp,v $
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

#include "mmoHelAxis.h"
#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"
#include "mafPipeIntGraph.h"
#include "mafPlotMath.h"
#include "mafTransform.h"
#include "mafTransformFrame.h"

#include "mafSmartPointer.h"
#include "mafVMEHelAxis.h"
#include "mafVMEAFRefSys.h"
#include "mafVMELandmarkCloud.h"

//----------------------------------------------------------------------------
// Required for MSVC
//----------------------------------------------------------------------------
#ifdef _MSC_FULL_VER
#pragma warning (disable: 4786)
#endif

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafCxxTypeMacro(mmoHelAxis);
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Forward Refs
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mmoHelAxis::mmoHelAxis(wxString label) :
mafOp(label), m_DictionaryFName("")
//----------------------------------------------------------------------------
{
  m_OpType            = OPTYPE_OP;
  m_Canundo           = true;
  m_HelicalSys        = NULL;
}

//----------------------------------------------------------------------------
mmoHelAxis::~mmoHelAxis()
//----------------------------------------------------------------------------
{
  vtkDEL(m_HelicalSys);
}

//----------------------------------------------------------------------------
mafOp* mmoHelAxis::Copy()   
//----------------------------------------------------------------------------
{
  return new mmoHelAxis(m_Label);
}

//----------------------------------------------------------------------------
bool mmoHelAxis::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  if(!vme) return false;
  return true;
}

//----------------------------------------------------------------------------
// widget id's
//----------------------------------------------------------------------------
enum 
{
  ID_DEFAULT = MINID,
  ID_LAST,
  ID_FORCED_DWORD = 0x7fffffff
};

//----------------------------------------------------------------------------
void mmoHelAxis::OpRun()   
//----------------------------------------------------------------------------
{
  mafNEW(m_HelicalSys);
  m_HelicalSys->SetName("Helical_axis");
  //CreateGui();
  mafEventMacro(mafEvent(this,OP_RUN_OK)); 
}

//----------------------------------------------------------------------------
void mmoHelAxis::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->SetListener(this);
  m_Gui->OkCancel();
  ShowGui();
}

//----------------------------------------------------------------------------
void mmoHelAxis::OpStop(int result)
//----------------------------------------------------------------------------
{
  HideGui();
  if (result == OP_RUN_CANCEL)
  {
    if(m_HelicalSys->GetParent())
    {
      mafEventMacro(mafEvent(this, VME_REMOVE, m_HelicalSys));
    }
  }
  mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
void mmoHelAxis::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{ 
  switch(maf_event->GetId())
  {
    case wxOK:          
    { 
      OpStop(OP_RUN_OK);
    }
    break;
    case wxCANCEL:
    {    
      OpStop(OP_RUN_CANCEL);
    }
    break;
    default:
    {
      mafEventMacro(*maf_event); 
    }
    break;
  }
}

//----------------------------------------------------------------------------
void mmoHelAxis::OpDo()
//----------------------------------------------------------------------------
{
  assert(m_HelicalSys);
  m_HelicalSys->ReparentTo(m_Input);
  m_HelicalSys->SetScaleFactor(100.0);
  m_HelicalSys->Update();
  m_Output = m_HelicalSys;
}

