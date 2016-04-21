/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpFrameVolumeBrush.cpp,v $
Language:  C++
Date:      $Date: 2010-06-08 13:36:33 $
Version:   $Revision: 1.1.2.4 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpFrameVolumeBrush.h"


BEGIN_EVENT_TABLE(lhpFrameVolumeBrush, mafGUIMDIChild)
EVT_CLOSE(lhpFrameVolumeBrush::OnCloseWindow)
END_EVENT_TABLE()

//----------------------------------------------------------------------------
lhpFrameVolumeBrush::lhpFrameVolumeBrush(wxMDIParentFrame* parent, mafView *view) : mafGUIMDIChild(parent, view)
//----------------------------------------------------------------------------
{

}

//----------------------------------------------------------------------------
lhpFrameVolumeBrush::~lhpFrameVolumeBrush()
//----------------------------------------------------------------------------
{

}

//----------------------------------------------------------------------------
// disable original close event handling
void lhpFrameVolumeBrush::OnCloseWindow  (wxCloseEvent &event)
//----------------------------------------------------------------------------
{
	//mafEventMacro(mafEvent(this,VIEW_DELETE,m_View));
	
	// DISABLE WINDOW CLOSING
	wxMessageBox("Please use OK or Cancel button on the operation panel");
	
	//cppDEL(m_View);
	//m_View = NULL;
	//Destroy();

}