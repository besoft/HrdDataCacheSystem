/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpInteractorVolumeBrush.cpp,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.9 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2009
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpInteractorVolumeBrush.h"
#include "lhpOp3DVolumeBrush.h"
#include "lhpViewVolumeBrush.h"
#include "lhpVolumeBrushContextualMenu.h"

#include "vtkLHPSelectionVolumeGray.h"
#include "vtkLHPImageStencilData.h"
#include "vtkLHPVolumeTextureMapper3D.h"



#include "mafRWIBase.h"
#include "mafEventInteraction.h"
#include "mafDeviceButtonsPadMouse.h"
#include "vtkMAFContourVolumeMapper.h"

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkVolume.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
//#include "vtkVolumeRayCastMapper.h"
//#include "vtkVolumeRayCastCompositeFunction.h"
#include "vtkVolumeProperty.h"


mafCxxTypeMacro(lhpInteractorVolumeBrush)


//----------------------------------------------------------------------------
lhpInteractorVolumeBrush::lhpInteractorVolumeBrush()
//----------------------------------------------------------------------------
{

	m_Mouse = NULL;
	m_View = NULL;
	m_Frame = NULL;
	m_DraggingLeft = false;


	// brush 
	m_BrushShape = lhpOp3DVolumeBrush::BRUSH_SHAPE_SPHERE;
	m_BrushCenter[0] = m_BrushCenter[1] = m_BrushCenter[2] = 0;
	m_BrushSize[0] = m_BrushSize[1] = m_BrushSize[2] = 1;
	m_Origin[0] = m_Origin[1] = m_Origin[2] = 0.0;


	// stencil
	m_pBrushStencilData = NULL;
	m_pStrokeStencilData = NULL;
	m_StencilRenderMode = lhpOp3DVolumeBrush::RENDER_MODE_NONE;
	// TODO !
	m_CurrentSelectionLabel = 200;


	// selection volume 	
	m_SelectionVolumeRendered = vtkVolume::New();


	// 3d texture volume rendering for selection volume
	// 3d mapper not available in vtk 4.2
	m_SelectionVolumeMapper = vtkLHPVolumeTextureMapper3D::New();

	//vtkVolumeRayCastCompositeFunction * compositeFunction = vtkVolumeRayCastCompositeFunction::New();
	//vtkVolumeRayCastMapper * pVolumeMapper = vtkVolumeRayCastMapper::New();
	//pVolumeMapper->SetVolumeRayCastFunction(compositeFunction);
	//m_SelectionVolumeMapper = pVolumeMapper;

	m_OpacityTransferFunction = vtkPiecewiseFunction::New();
	m_OpacityTransferFunction->AddPoint(0, 0.0);
	m_OpacityTransferFunction->AddPoint(255, 0.3);

	// Create transfer mapping scalar value to color
	m_ColorTransferFunction = vtkColorTransferFunction::New();
	m_ColorTransferFunction->AddRGBPoint(20.0, 0.0, 0.0, 0.0);
	m_ColorTransferFunction->AddRGBPoint(64.0, 0.0, 0.0, 0.0);
	m_ColorTransferFunction->AddRGBPoint(128.0, 0.0, 0.0, 0.0);
	m_ColorTransferFunction->AddRGBPoint(192.0, 1.0, 1.0, 1.0);
	m_ColorTransferFunction->AddRGBPoint(255.0, 0.2, 0.2, 0.2);

	// The property describes how the data will look
	m_VolumeProperty = vtkVolumeProperty::New();
	m_VolumeProperty->SetColor(m_ColorTransferFunction);
	m_VolumeProperty->SetScalarOpacity(m_OpacityTransferFunction);

	
	// Isosurface rendering for selection volume 
	vtkNEW(m_ContourMapper);
	m_AlphaValue = 1.0;
	m_ContourMapper->AutoLODRenderOn();
	m_ContourMapper->SetAlpha(m_AlphaValue);

	//double range[2] = {0, 0};
	//dataset->GetScalarRange(range);
	//float value = 0.5f * (range[0] + range[1]);
	//while (value < range[1] && m_ContourMapper->EstimateRelevantVolume(value) > 0.3f)
	//	value += 0.05f * (range[1] + range[0]) + 1.f;
	m_ContourValue = m_CurrentSelectionLabel/255.0;
	m_ContourMapper->SetContourValue(m_ContourValue);


}


//----------------------------------------------------------------------------
lhpInteractorVolumeBrush::~lhpInteractorVolumeBrush(void)
//----------------------------------------------------------------------------
{
	vtkDEL(m_SelectionVolumeRendered);
	vtkDEL(m_SelectionVolumeMapper);
	vtkDEL(m_OpacityTransferFunction);;
	vtkDEL(m_ColorTransferFunction);
	vtkDEL(m_VolumeProperty);
	vtkDEL(m_ContourMapper);

	mafDEL(m_pBrushStencilData);
	mafDEL(m_pStrokeStencilData);
}


//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetBrushShape(int t)
//----------------------------------------------------------------------------
{
	if (t == m_BrushShape)
		return;

	m_BrushShape = t;

	if (m_View)
	{
		m_View->SetBrushShape(t);
		UpdateBrushStencil();

		if (m_BrushShape == lhpOp3DVolumeBrush::BRUSH_SHAPE_SPHERE)
		{
			m_pBrushStencilData->GenSphereStencil(m_View->GetGizmoSphereSource());
		}
		else if (m_BrushShape == lhpOp3DVolumeBrush::BRUSH_SHAPE_CUBE)
		{
			m_pBrushStencilData->GenBoxStencil(m_View->GetGizmoCubeSource());
		}
		
		if (m_View->GetBrushShow())
			UpdateAllViews();
	}
}


//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetBrushSize(int s)
//----------------------------------------------------------------------------
{
	if ( s == m_BrushSize[0])
		return;

	m_BrushSize[0] = m_BrushSize[1] = m_BrushSize[2]  = s;
	
	if ( m_pBrushStencilData )
	{
		UpdateBrushBoundingBox();
		UpdateBrushGizmoPositionAndSize();
		UpdateBrushStencil();

		if (m_BrushShape == lhpOp3DVolumeBrush::BRUSH_SHAPE_SPHERE)
		{
			m_pBrushStencilData->GenSphereStencil(m_View->GetGizmoSphereSource());
		}
		else if (m_BrushShape == lhpOp3DVolumeBrush::BRUSH_SHAPE_CUBE)
		{
			m_pBrushStencilData->GenBoxStencil(m_View->GetGizmoCubeSource());
		}

		if (m_View && m_View->GetBrushShow())
			UpdateAllViews();
	}

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetSelectionVolumeRenderMode(int mode)
//----------------------------------------------------------------------------
{
	
	if (m_StencilRenderMode == mode)
		return;

	//TODO: ->IsClear()
	if (NULL == m_SelectionVolumeData)
	{
		m_StencilRenderMode = mode;
		return;
	}


	if (m_StencilRenderMode != lhpOp3DVolumeBrush::RENDER_MODE_NONE)
	{
		HideSelectionVolume();
	}

	m_StencilRenderMode = mode;
	
	// show stencil if not NONE
	if (m_StencilRenderMode != lhpOp3DVolumeBrush::RENDER_MODE_NONE)
		ShowSelectionVolume();

	UpdateAllViews();

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetHighlightColour(wxColour * colour)
//----------------------------------------------------------------------------
{
	m_View->SetHighlightColour(colour);
	UpdateAllViews();
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetHighlightTransparency(int trans)
//----------------------------------------------------------------------------
{
	m_View->SetHighlightTransparency(trans);
	UpdateAllViews();
}


//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::OnRightButtonDown(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
	mafInteractorPER::OnRightButtonDown(e);
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::OnRightButtonUp(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
	//mafInteractorPER::OnRightButtonUp(e);

	// show context menu
	
	mafDeviceButtonsPadMouse *mouse = mafDeviceButtonsPadMouse::SafeDownCast((mafDevice*)e->GetSender());

	if(m_ShowContextMenu && mouse)
	{
		//mafVME *vme = GetPickedVME(mouse);
		//InvokeEvent(SHOW_CONTEXTUAL_MENU,MCH_UP,vme);
		lhpVolumeBrushContextualMenu *contextMenu = new lhpVolumeBrushContextualMenu();
		
		contextMenu->SetListener(this);
		//mafView *v = m_ViewManager->GetSelectedView();

		//mafGUIMDIChild *c = (mafGUIMDIChild *)m_Win->GetActiveChild();

		if(m_Frame && m_View)
			contextMenu->ShowContextualMenu((wxFrame *) m_Frame,m_View,false);

		cppDEL(contextMenu);
	
	
	}

	mafInteractorPER::OnButtonUp(e);
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::OnLeftButtonDown(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
	//mafLogMessage("Left button down");

	e->Get2DPosition(m_PickedPoint2D);

	mafEventMacro(mafEvent(this, CAMERA_UPDATE));

	mafDevice *device = mafDevice::SafeDownCast((mafDevice*)e->GetSender());
	mafDeviceButtonsPadMouse  *mouse  = mafDeviceButtonsPadMouse::SafeDownCast(device);

	if (NULL == m_Mouse)
	{
		m_Mouse = mouse;
	}

	mafView * pView = m_Mouse->GetView();

	m_CurrentRenderer = m_Mouse->GetRenderer();
	m_Interactor = m_Mouse->GetInteractor();

	if (0 == e->GetModifiers())
	{
		mafInteractorPER::OnLeftButtonDown(e);

	}
	// volume brush mouse events
	else if (e->GetModifier(MAF_CTRL_KEY) || e->GetModifier(MAF_ALT_KEY))
	{

		// don't handle brush painting on perspective view
		if (m_View->IsPerspectiveView(m_Interactor))
			return;

		int op;
		
		if (e->GetModifier(MAF_CTRL_KEY))
		{
			// selecting
			// todo change it to macro
			op = BRUSH_OP_SELECT;
		}
		else if (e->GetModifier(MAF_ALT_KEY))
		{
			// deselecting
			op = BRUSH_OP_UNSELECT;
		}

		UpdateBrushPosition();
		UpdateBrushStencil();

		// show brush
		m_View->ShowBrush(true);

		if (IsBrushIntersectWithVolume())
		{
			
			UpdateStrokeStencil();
			UpdateSelectionVolume(op, m_CurrentSelectionLabel);
			ShowSelectionVolume();
			
		}
		
		UpdateAllViews();
		
	}

	m_DraggingLeft = true;

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetView(lhpViewVolumeBrush * pView)
//----------------------------------------------------------------------------
{
	m_View = pView;
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetFrame(lhpFrameVolumeBrush * pFrame)
//----------------------------------------------------------------------------
{
	m_Frame = pFrame;
}


//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::OnLeftButtonUp(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
	//mafLogMessage("Left button Up");

	double pos_2d[2] = {-1,-1};

	m_DraggingLeft = false;

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::OnMove(mafEventInteraction *e)
//----------------------------------------------------------------------------
{

	if (m_DraggingLeft)
	{
		e->Get2DPosition(m_PickedPoint2D);
		if (e->GetModifier(MAF_CTRL_KEY) || e->GetModifier(MAF_ALT_KEY))
		{
			// these two functions should be used together, may be better merged
			UpdateBrushPosition();
			UpdateBrushStencil();

			UpdateStrokeStencil();

			int op;
			if (e->GetModifier(MAF_CTRL_KEY))
			{
				op = BRUSH_OP_SELECT;
			}
			else if (e->GetModifier(MAF_ALT_KEY))
			{
				op = BRUSH_OP_UNSELECT;
			}
			

			UpdateSelectionVolume(op, m_CurrentSelectionLabel);

			UpdateAllViews();
			
		}
	}

	mafInteractorPER::OnMove(e);

}


//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetVolumeData(vtkImageData * pData)
//----------------------------------------------------------------------------
{
	m_VolumeData = pData;

	pData->GetExtent(m_Extent);
	pData->GetSpacing(m_Spacing);
	pData->GetOrigin(m_Origin);

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SetSelectionVolume(vtkLHPSelectionVolumeGray * vol)
//----------------------------------------------------------------------------
{
	m_SelectionVolumeData = vol;
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::UpdateBrushStencil()
//----------------------------------------------------------------------------
{

	if (NULL == m_pBrushStencilData)
	{
		m_pBrushStencilData = vtkLHPImageStencilData::New();

		// origin is useless for brush stencil
		//m_pBrushStencilData->SetOrigin(m_VolumeData->GetOrigin());
		m_pBrushStencilData->SetSpacing(m_VolumeData->GetSpacing());

		// fast generation of sphere and box stencils
		if (m_BrushShape == lhpOp3DVolumeBrush::BRUSH_SHAPE_SPHERE)
		{
			m_pBrushStencilData->GenSphereStencil(m_View->GetGizmoSphereSource());
		}
		else if (m_BrushShape == lhpOp3DVolumeBrush::BRUSH_SHAPE_CUBE)
		{
			m_pBrushStencilData->GenBoxStencil(m_View->GetGizmoCubeSource());
		}

	}

	// the original brush stencil position is at (0, 0, 0)
	// m_transBrushStencil is the translation for for the brush stencil
	m_transBrushStencil[0] = (m_BrushCenter[0] - m_Origin[0] -  m_BrushSize[0]) / m_Spacing[0];
	m_transBrushStencil[1] =  (m_BrushCenter[1] - m_Origin[1] - m_BrushSize[1]) / m_Spacing[1];
	m_transBrushStencil[2] = (m_BrushCenter[2] - m_Origin[2] - m_BrushSize[2]) / m_Spacing[2];

	//mafLogMessage("Translation: %d %d %d", m_transBrushStencil[0], m_transBrushStencil[1], m_transBrushStencil[2]);

}

//----------------------------------------------------------------------------
// stroke generation: to get a smooth shape of brush painting 
void lhpInteractorVolumeBrush::UpdateStrokeStencil()
//----------------------------------------------------------------------------
{
	if (NULL == m_pStrokeStencilData)
	{
		m_pStrokeStencilData = vtkLHPImageStencilData::New();

		m_pStrokeStencilData->SetOrigin(m_VolumeData->GetOrigin());
		m_pStrokeStencilData->SetSpacing(m_VolumeData->GetSpacing());
	}
	else
		m_pStrokeStencilData->Clear();
	
	int strokeExtent[6];

	if (m_DraggingLeft)
	{

		// mouse dragging - brush stroking
		// generate a smooth brush stroke stencil data
		
		// translation from the last brush position
		int trans[3];
		
		for (int i = 0; i < 3; i++)
		{
			trans[i] = (m_BrushCenter[i] - m_LastBrushCenter[i])/m_Spacing[i];


			if (m_BrushCenter[i] >= m_LastBrushCenter[i])
			{
				strokeExtent[i * 2] = int ((m_LastBrushCenter[i] - m_Origin[i] - m_BrushSize[i]) / m_Spacing[i]);
				strokeExtent[i * 2 + 1] = int ((m_BrushCenter[i] - m_Origin[i] + m_BrushSize[i]) / m_Spacing[i] + 0.5);
			}	
			else
			{
				strokeExtent[i * 2] = int ((m_BrushCenter[i] - m_Origin[i] - m_BrushSize[i]) / m_Spacing[i]);
				strokeExtent[i * 2 + 1] = int ((m_LastBrushCenter[i] - m_Origin[i] + m_BrushSize[i]) / m_Spacing[i] + 0.5);

			}
		}

		// step size influences the smoothness of user interactions because of 
		// stencil memory allocation for every stroke
		double len = sqrt(double (trans[0] * trans[0] + trans[1] * trans[1] + trans[2] * trans[2]));

		double step = m_BrushSize[0]/4.0;
		if (step < 1 )
			step = 1;
		if (len < step)
			step = len / 2.0;

		double stepNum = len / step;

		//mafLogMessage("step: %3.2f, stepNum: %10.2f, trans: %d %d %d", step, stepNum, trans[0], trans[1], trans[2]);

		// pre-allcoate extents to avoid frequent DeepCopy during a brush stroke
		m_pStrokeStencilData->AddAndResizeExtent(strokeExtent);
		m_pBrushStencilData->StartRecordTranslation();
		// move the stencil to the end point of the stroke
		m_pBrushStencilData->ApplyTranslation(m_transBrushStencil[0] ,	m_transBrushStencil[1],	m_transBrushStencil[2]);

		// to avoid the accumulation error for stoke stencil generation
		double accumulatedTrans[3], lastAccumulatedTrans[3], stepTrans[3], stepLen;
		lastAccumulatedTrans[0] = lastAccumulatedTrans[1] = lastAccumulatedTrans[2] = 0;
		for (int i = 1; i <= stepNum ; i++ )
		{
			m_pStrokeStencilData->AddWithExtentAllocated(m_pBrushStencilData);

			for (int j = 0; j < 3; j++)
			{
				accumulatedTrans[j] = - (int)trans[j] * i/stepNum;
				stepTrans[j] = accumulatedTrans[j] - lastAccumulatedTrans[j];
			}
			
			stepLen = sqrt(stepTrans[0] * stepTrans[0] + stepTrans[1] * stepTrans[1] + stepTrans[2] * stepTrans[2]);

			if (( stepLen >= step ) && ( fabs(stepTrans[0]) >= 1.0 || fabs(stepTrans[1]) >= 1.0  || fabs(stepTrans[2]) >= 1.0 ))
			{
				lastAccumulatedTrans[0] = (int) accumulatedTrans[0];
				lastAccumulatedTrans[1] = (int) accumulatedTrans[1];
				lastAccumulatedTrans[2] = (int) accumulatedTrans[2];
			
				m_pBrushStencilData->ApplyTranslation(stepTrans[0] , stepTrans[1], stepTrans[2]);
			
			}

		}
		// rollback the brush stencil to the origin
		m_pBrushStencilData->RollbackRecordedTranslation();
		m_pBrushStencilData->StopRecordTranslation();

	}
	else 
	{
		// single click, !m_DraggingLeft
		
		for (int i = 0; i < 3; i++)
		{
			strokeExtent[i * 2] = int ((m_BrushCenter[i] - m_BrushSize[i]) / m_Spacing[i]);
			strokeExtent[i * 2 + 1] = int ((m_BrushCenter[i] + m_BrushSize[i]) / m_Spacing[i] + 0.5);
		}

		m_pStrokeStencilData->AddAndResizeExtent(strokeExtent);
		m_pBrushStencilData->StartRecordTranslation();
		m_pBrushStencilData->ApplyTranslation(m_transBrushStencil[0], m_transBrushStencil[1], m_transBrushStencil[2]);
		m_pStrokeStencilData->AddWithExtentAllocated(m_pBrushStencilData);
		m_pBrushStencilData->RollbackRecordedTranslation();
		m_pBrushStencilData->StopRecordTranslation();
	}

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::UpdateBrushPosition()
//----------------------------------------------------------------------------
{
	// vtkRenderer is a ViewPort
	if (NULL == m_View)
		return;

	double z = m_CurrentRenderer->GetZ(m_PickedPoint2D[0],m_PickedPoint2D[1]);
	m_CurrentRenderer->SetDisplayPoint(m_PickedPoint2D[0],m_PickedPoint2D[1],z);
	m_CurrentRenderer->DisplayToWorld();
	// not correct for the 3D view
	m_CurrentRenderer->GetWorldPoint(m_PickedPoint3D);

	// to work on the slice plane
	m_View->UpdatePointToSlicePlane(m_PickedPoint3D, m_Interactor);

	for (int i = 0; i < 3; i++)
	{
		m_LastBrushCenter[i] = m_BrushCenter[i];
		// rounding to the nearest integer position
		m_BrushCenter[i] = int ((m_PickedPoint3D[i] - m_Origin[i])/m_Spacing[i] + 0.5) * m_Spacing[i] + m_Origin[i];
	}

	UpdateBrushBoundingBox();

	UpdateBrushGizmoPositionAndSize();

	//mafLogMessage("Brush Center: %.2f %.2f %.2f", m_BrushCenter[0], m_BrushCenter[1], m_BrushCenter[2]);
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::UpdateBrushBoundingBox()
//----------------------------------------------------------------------------
{
	for (int i = 0; i < 3; i++)
	{
		m_boundsBrush[i*2] = m_BrushCenter[i] - m_BrushSize[i];
		m_boundsBrush[i*2 + 1] = m_BrushCenter[i] + m_BrushSize[i];
	}
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::UpdateBrushGizmoPositionAndSize()
//----------------------------------------------------------------------------
{
	m_View->SetBrushCenter(m_BrushCenter);
	m_View->SetBrushSize(m_BrushSize);
}

//----------------------------------------------------------------------------
// test if the painting intersect with the volume 
bool lhpInteractorVolumeBrush::IsBrushIntersectWithVolume()
//----------------------------------------------------------------------------
{
	double boundVolume[6];
	m_VolumeData->GetBounds(boundVolume);

	if ((m_boundsBrush[1] < boundVolume[0]) || (m_boundsBrush[0] > boundVolume[1])
		|| (m_boundsBrush[3] < boundVolume[2]) || (m_boundsBrush[2] > boundVolume[3])
		|| (m_boundsBrush[5] < boundVolume[4]) || (m_boundsBrush[4] > boundVolume[5]))
	{
		return false;
	}
	else 
	{
		return true;
	}
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::UpdateSelectionVolume(int op, unsigned char label)
//----------------------------------------------------------------------------
{
	if (BRUSH_OP_SELECT == op)
	{
		// TODO: support multi-labeling ??
		m_SelectionVolumeData->Select(m_pStrokeStencilData, label);
	}
	else if (BRUSH_OP_UNSELECT == op)
	{
		m_SelectionVolumeData->UnSelect(m_pStrokeStencilData);
	}
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::ClearSelectionVolume()
//----------------------------------------------------------------------------
{
	m_SelectionVolumeData->Clear();
	m_ContourMapper->SetInput(m_SelectionVolumeData);

	UpdateAllViews();
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::SelectByThreshold(double lower, double upper)
//----------------------------------------------------------------------------
{
	m_SelectionVolumeData->SelectByThreshold(m_VolumeData, lower,upper, m_CurrentSelectionLabel);
	m_ContourMapper->SetInput(m_SelectionVolumeData);
	UpdateAllViews();
}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::ShowSelectionVolume()
//----------------------------------------------------------------------------
{

	if (m_StencilRenderMode == lhpOp3DVolumeBrush::RENDER_MODE_NONE)
		return;

	if (m_StencilRenderMode == lhpOp3DVolumeBrush::RENDER_MODE_ISOSURFACE)
	{
		// MAF ISO surface rendering
		m_ContourMapper->SetInput(m_SelectionVolumeData);
		m_SelectionVolumeRendered->SetMapper(m_ContourMapper);

		m_SelectionVolumeRendered->PickableOff();
		m_View->AddRenderingVolume(m_SelectionVolumeRendered);

	}
	else if (m_StencilRenderMode == lhpOp3DVolumeBrush::RENDER_MODE_VOLUME)
	{
		// 3d texture volume rendering
		m_SelectionVolumeMapper->SetInput(m_SelectionVolumeData);
		m_SelectionVolumeRendered->SetMapper(m_SelectionVolumeMapper);
		m_SelectionVolumeRendered->SetProperty(m_VolumeProperty);

		if (!m_SelectionVolumeMapper->IsRenderSupported(m_VolumeProperty))
		{
			wxMessageBox("3D texture volume rendering cannot be supported by your hardware or current implementation");
		}

		m_View->AddRenderingVolume(m_SelectionVolumeRendered);
		
	}

}

//----------------------------------------------------------------------------
void lhpInteractorVolumeBrush::HideSelectionVolume()
//----------------------------------------------------------------------------
{
	// already hided
	if (lhpOp3DVolumeBrush::RENDER_MODE_NONE == m_StencilRenderMode)
		return;

	if (m_SelectionVolumeRendered)
		m_View->RemoveRenderingVolume(m_SelectionVolumeRendered);

}

//----------------------------------------------------------------------------
// show brush gizmo
void lhpInteractorVolumeBrush::ShowBrush( bool b)
//----------------------------------------------------------------------------
{
	if (m_View)
		m_View->ShowBrush(b);
}

//----------------------------------------------------------------------------
// if it is put in lhpViewVolumeBrush, it will cause deadlock
void lhpInteractorVolumeBrush::UpdateAllViews()
//----------------------------------------------------------------------------
{

	if (NULL == m_View)
		return;

	for (int i = 0; i < m_View->GetNumberOfSubView(); i++)
	{
		mafView * pView = m_View->GetSubView(i);

		pView->GetRWI()->GetRenderWindow()->Render();
	}

}