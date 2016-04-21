/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpFrameVolumeBrush.h,v $
Language:  C++
Date:      $Date: 2010-06-08 13:36:34 $
Version:   $Revision: 1.1.2.3 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpVolumeBrushFrame_H__
#define __lhpVolumeBrushFrame_H__

//#include "mafGuiViewFrame.h"

#include "mafGUIMDIChild.h"

/** 
This class is simply used to disable the closing behaviour of 
the Volume Brush window 
*/
class lhpFrameVolumeBrush : public mafGUIMDIChild
{
public:

	lhpFrameVolumeBrush(wxMDIParentFrame* parent, mafView *view);

	~lhpFrameVolumeBrush(); 

protected:
	
	/** disable the event to destroy the owned view. */
	void OnCloseWindow  (wxCloseEvent &event);


	DECLARE_EVENT_TABLE()

};

#endif