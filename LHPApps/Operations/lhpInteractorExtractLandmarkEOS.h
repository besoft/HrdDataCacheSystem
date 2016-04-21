/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpInteractorVolumeBrush.h,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.8 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2011
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __lhpInteractorExtractLandmarkEOS_H__
#define __lhpInteractorExtractLandmarkEOS_H__

#include "mafInteractorPER.h"

class vtkRenderer;
class	vtkRenderWindowInteractor;
class	medDeviceButtonsPadMouseDialog;
class lhpOpExtractLandmarkEOS;

class lhpInteractorExtractLandmarkEOS : public mafInteractorPER
{
public:
	mafTypeMacro(lhpInteractorExtractLandmarkEOS, mafInteractorPER);

	explicit lhpInteractorExtractLandmarkEOS();
	virtual ~lhpInteractorExtractLandmarkEOS();

	void SetOperation(lhpOpExtractLandmarkEOS * pOp);

protected:


	// event handling
	//virtual void OnEvent(mafEventBase * e);
	virtual void OnLeftButtonDown   (mafEventInteraction *e);
	//virtual void OnLeftButtonUp     (mafEventInteraction *e);
	//virtual void OnMiddleButtonDown (mafEventInteraction *e);
	//virtual void OnMiddleButtonUp   (mafEventInteraction *e);
	//virtual void OnRightButtonDown  (mafEventInteraction *e);
	//virtual void OnRightButtonUp    (mafEventInteraction *e);
	
	//virtual void OnButtonDown       (mafEventInteraction *e);
	//virtual void OnButtonUp         (mafEventInteraction *e); 

	//virtual void OnMove             (mafEventInteraction *e);
	//virtual void OnChar             (mafEventInteraction *e) {};

	

	lhpOpExtractLandmarkEOS * m_Op;

	vtkRenderer * m_CurrentRenderer;
	vtkRenderWindowInteractor * m_Rwi;
	medDeviceButtonsPadMouseDialog * m_Mouse;

	/** if the mouse is dragging with left button down */
	//bool m_DraggingLeft;
	
	/** mouse click position in 2D */
	//double  m_PickedPoint2D[2];
	/** mouse click position in 3D */
	//double  m_PickedPoint3D[4];

};



#endif