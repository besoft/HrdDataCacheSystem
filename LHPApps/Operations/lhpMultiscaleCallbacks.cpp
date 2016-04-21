/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMultiscaleCallbacks.cpp,v $
Language:  C++
Date:      $Date: 2011-05-27 07:52:28 $
Version:   $Revision: 1.1.1.1.2.2 $
Authors:   Nigel McFarlane
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "mafEvent.h"
#include "lhpMultiscaleCallbacks.h"
#include "mafDeviceButtonsPadMouse.h"


//------------------------------------------------------------------------------
// Start render event
void vtkStartRenderCallback::Execute(vtkObject *caller, unsigned long, void*)
//------------------------------------------------------------------------------
{
  vtkRenderer *renderer = dynamic_cast<vtkRenderer*>(caller);
  if (renderer == NULL)
    return ;

  mafEvent e(this, m_id) ;
  mafEventMacro(e) ;
}


//------------------------------------------------------------------------------
// Mouse click event
void vtkMouseClickCallback::Execute(vtkObject *caller, unsigned long, void*)
//------------------------------------------------------------------------------
{
  vtkRenderWindowInteractor *RWI = dynamic_cast<vtkRenderWindowInteractor*>(caller);
  if (RWI == NULL)
    return ;

  mafEvent e(this, m_id) ;
  mafEventMacro(e) ;
}



//------------------------------------------------------------------------------
// Double click event
void lhpMultiscaleDoubleClickCallback::OnEvent(mafEventBase *event)
//------------------------------------------------------------------------------
{
  mafID id=event->GetId();
  mafID ch=event->GetChannel();
  if (ch==MCH_INPUT){
    if (id == mafDeviceButtonsPadMouse::GetMouseDClickId()){}

    mafLogMessage("double click !") ;
    mafEvent e(this, m_id) ;
    mafEventMacro(e) ;
  }
}