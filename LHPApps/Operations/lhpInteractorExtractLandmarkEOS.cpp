/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpInteractorVolumeBrush.cpp,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.9 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2011
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpInteractorExtractLandmarkEOS.h"
/*
class vtkRenderer;
class	vtkRenderWindowInteractor;

*/
#include "mafEventInteraction.h"
#include "medDeviceButtonsPadMouseDialog.h"
#include "lhpOpExtractLandmarkEOS.h"

#include "mafVME.h"

mafCxxTypeMacro(lhpInteractorExtractLandmarkEOS)

lhpInteractorExtractLandmarkEOS::lhpInteractorExtractLandmarkEOS()
{
	m_Mouse = NULL;
	m_DraggingLeft = false;
	m_Op = NULL;
}

lhpInteractorExtractLandmarkEOS::~lhpInteractorExtractLandmarkEOS()
{

}

void lhpInteractorExtractLandmarkEOS::SetOperation(lhpOpExtractLandmarkEOS * pOp)
{
	m_Op = pOp;
}

/*
void lhpInteractorExtractLandmarkEOS::OnEvent(mafEventBase *e)
{
	mafInteractorPER::OnEvent(e);
}
*/

void lhpInteractorExtractLandmarkEOS::OnLeftButtonDown(mafEventInteraction *e)
{
	//mafLogMessage("Left Button Down");

	//e->Get2DPosition(m_PickedPoint2D);

	//mafEventMacro(mafEvent(this, CAMERA_UPDATE));

	mafDevice *device = mafDevice::SafeDownCast((mafDevice*)e->GetSender());
	medDeviceButtonsPadMouseDialog  *mouse  = medDeviceButtonsPadMouseDialog::SafeDownCast(device);

	if (NULL == m_Mouse)
	{
		m_Mouse = mouse;
	}

	m_CurrentRenderer = m_Mouse->GetRenderer();
	m_Rwi = m_Mouse->GetInteractor();
	mafView * pView = m_Op->GetView(m_Rwi);
	if (pView)
		m_Mouse->SetView(pView);

	
	mafInteractorPER::OnLeftButtonDown(e);

	//m_DraggingLeft = true;

	  mafVME *vme = GetPickedVME(device);
	  
      if(vme)
      {
		  m_Op->SetPickedVME(vme);
		  /*
        mafInteractor *bh = vme->GetBehavior(); //can be NULL
        if (bh)
        {
          bh->OnEvent(e); // forward to VME behavior
        }
		*/
      }
	 


}



