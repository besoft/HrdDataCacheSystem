/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpTextureOrientationCallbacks.cpp,v $
Language:  C++
Date:      $Date: 2009-05-19 14:29:53 $
Version:   $Revision: 1.1 $
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

#include "mafEvent.h"
#include "lhpTextureOrientationCallbacks.h"
#include "lhpTextureOrientationFilter.h"


//------------------------------------------------------------------------------
// Handle user progress event from texture filter
// This avoids having to put widget code in the filter.
void lhpTextureOrientationProgressCallback::Execute(vtkObject *caller, unsigned long eventId, void* calldata)
//------------------------------------------------------------------------------
{
  lhpTextureOrientationFilter *texFilter = dynamic_cast<lhpTextureOrientationFilter*>(caller) ;
  if (texFilter == NULL)
    return ;

  mafEvent e(this, m_id) ;
  e.SetData(calldata) ;
  mafEventMacro(e) ;
}
