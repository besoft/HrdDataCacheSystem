/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: lhpViewVolumeBrush.cpp,v $
Language:  C++
Date:      $Date: 2011-05-27 07:52:28 $
Version:   $Revision: 1.1.2.7 $
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

#include "lhpViewVolumeBrush.h"
#include "lhpGizmoVolumeBrush.h"
#include "lhpPipeVolumeBrushSlice.h"
#include "vtkLHPSelectionVolumeGray.h"

#include "mafGUI.h"
#include "mafRWIBase.h"
#include "mafVME.h"
#include "mafGizmoSlice.h"
#include "mafViewSlice.h"
#include "mafGUILutSwatch.h"


#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"




mafCxxTypeMacro(lhpViewVolumeBrush)


// from mafViewOrthoSlice.cpp
extern enum CHILD_VIEW_ID
{
	CHILD_PERSPECTIVE_VIEW = 0,
	CHILD_ZN_VIEW,
	CHILD_XN_VIEW,
	CHILD_YN_VIEW,
	CHILD_VIEWS_NUMBER,
};

extern enum GIZMO_ID
{
  GIZMO_XN = 0,
  GIZMO_YN,
  GIZMO_ZN,
  GIZMOS_NUMBER,
};


extern enum ORTHOSLICE_SUBVIEW_ID
{
	PERSPECTIVE_VIEW = 0,
	XN_VIEW,
	YN_VIEW,
	ZN_VIEW,
	VIEWS_NUMBER,
};

//----------------------------------------------------------------------------
lhpViewVolumeBrush::lhpViewVolumeBrush(wxString label):mafViewOrthoSlice(label)
//----------------------------------------------------------------------------
{
	m_SelVolEnabled = false;
	m_BrushGizmo = NULL;
}

//----------------------------------------------------------------------------
lhpViewVolumeBrush::~lhpViewVolumeBrush()
//----------------------------------------------------------------------------
{
	
}

//----------------------------------------------------------------------------
mafGUI * lhpViewVolumeBrush::CreateGui()
//----------------------------------------------------------------------------
{

	wxString layout_choices[3] = {"default","layout 1","layout 2"};

	assert(m_Gui == NULL);

	//mafView::CreateGui();

	m_Gui = new mafGUI(this);

	// hints
	m_Gui->Label(_("CTRL + Mouse for painting"),true);
	m_Gui->Label(_("ALT + Mouse for erasing"),true);

	m_Gui->Combo(ID_LAYOUT_CHOOSER,"layout",&m_LayoutConfiguration,3,layout_choices);
	
	m_Gui->Divider();
	m_LutWidget = m_Gui->Lut(ID_LUT_CHOOSER,"lut",m_ColorLUT);

	m_Gui->Divider(2);

	//wxString sidesName[2];
	//sidesName[0] = "left";
	//sidesName[1] = "right";
	// m_Gui->Radio(ID_SIDE_ORTHO, "side", &m_Side, 2, sidesName, 2);

	//m_Gui->Bool(ID_SNAP,"Snap on grid",&m_Snap,1);

	// cause Crash
	//m_Gui->Button(ID_RESET_SLICES,"reset slices","");
	//m_Gui->Divider();
	
	// cause Crash
	//m_Gui->Bool(ID_ALL_SURFACE,"All Surface",&m_AllSurface);
	
	//m_Gui->FloatSlider(ID_BORDER_CHANGE,"Border",&m_Border,1.0,5.0);

	EnableWidgets(m_CurrentVolume != NULL);
	for(int i=1; i<m_NumOfChildView; i++)
	{
		m_ChildViewList[i]->GetGui();
	}
	
	/*
	// Added by Losi 11.25.2009
	if (m_CurrentVolume)
	{
		for (int i=0; i<m_NumOfChildView; i++)
		{
			mafPipeVolumeSlice_BES *p = NULL;
			p = mafPipeVolumeSlice_BES::SafeDownCast(((mafViewSlice *)m_ChildViewList[i])->GetNodePipe(m_CurrentVolume));
			if (p)
			{
				p->SetEnableGPU(m_EnableGPU);
			}
		}
	}
	m_Gui->Divider(1);
	m_Gui->Bool(ID_ENABLE_GPU,"Enable GPU",&m_EnableGPU,1);
	*/

	//m_Gui->Divider();
	return m_Gui;
}

//----------------------------------------------------------------------------
// add remove selection volume from the perspective view
void lhpViewVolumeBrush::AddRenderingVolume(vtkVolume *volume)
//----------------------------------------------------------------------------
{
	if (NULL == volume)
	{
		return;
	}

	// only add the selection volume to the perspective view
	mafView * pView = GetSubView(CHILD_PERSPECTIVE_VIEW);
	vtkRendererCollection * rc = pView->GetRWI()->GetRenderWindow()->GetRenderers();

	vtkRenderer *r = NULL;
	rc->InitTraversal();
	while(r = rc->GetNextItem())
	{
		r->AddVolume(volume);
	}

}

//----------------------------------------------------------------------------
// remove selection volume from the perspective view
void lhpViewVolumeBrush::RemoveRenderingVolume(vtkVolume * volume)
//----------------------------------------------------------------------------
{
	mafView * pView = GetSubView(CHILD_PERSPECTIVE_VIEW);
	vtkRendererCollection * rc = pView->GetRWI()->GetRenderWindow()->GetRenderers();

	vtkRenderer *r = NULL;
	rc->InitTraversal();
	while(r = rc->GetNextItem())
	{
		r->RemoveVolume(volume);
	}
}

/*
//----------------------------------------------------------------------------
void lhpViewVolumeBrush::UpdateSlice()
//----------------------------------------------------------------------------
{
	// ??
	for(int i=0; i<m_NumOfChildView; i++)
	{
		lhpPipeVolumeBrushSlice *p = (lhpPipeVolumeBrushSlice *)((mafViewSlice *)m_ChildViewList[i])->GetNodePipe(m_CurrentVolume);
		p->UpdateSlice();

	}
	
}
*/

//----------------------------------------------------------------------------
int lhpViewVolumeBrush::GetSubviewId(vtkRenderWindowInteractor * interactor)
//----------------------------------------------------------------------------
{
	for (int i = 0; i < GetNumberOfSubView(); i++)
	{

		if  ( vtkRenderWindowInteractor::SafeDownCast(GetSubView(i)->GetRWI()) == interactor)
			return i;
	}

	return -1;
}

//----------------------------------------------------------------------------
bool lhpViewVolumeBrush::IsPerspectiveView(vtkRenderWindowInteractor * interactor)
//----------------------------------------------------------------------------
{
	if (CHILD_PERSPECTIVE_VIEW == GetSubviewId(interactor))
		return true;
	else
		return false;
}

//----------------------------------------------------------------------------
// constraint the clicked point to the slice plane
void lhpViewVolumeBrush::UpdatePointToSlicePlane(double Pick3D[], vtkRenderWindowInteractor * interactor)
//----------------------------------------------------------------------------
{

	switch (GetSubviewId(interactor))
	{
	case CHILD_XN_VIEW:
		Pick3D[0] = m_GizmoHandlePosition[0];
		break;
	case CHILD_YN_VIEW:
		Pick3D[1] = m_GizmoHandlePosition[1];
		break;
	case CHILD_ZN_VIEW:
		Pick3D[2] = m_GizmoHandlePosition[2];
		break;
	case CHILD_PERSPECTIVE_VIEW:
		break;
	}
	
	
}

//----------------------------------------------------------------------------
// Manually add gizmo vme to the view as it is not connected to ViewManager
// So it cannot add gizmo vmes by event handling
void lhpViewVolumeBrush::SliceGizmoAdd()
//----------------------------------------------------------------------------
{
	//Superclass::GizmoCreate();

	for(int gizmoId=GIZMO_XN; gizmoId<GIZMOS_NUMBER; gizmoId++) 
	{
		VmeAdd((mafNode *)m_Gizmo[gizmoId]->GetOutput());
	}

	// put them in the right views:
	// perspective view
	m_ChildViewList[0]->VmeShow((mafNode *)m_Gizmo[GIZMO_XN]->GetOutput(), true);
	m_ChildViewList[0]->VmeShow((mafNode *)m_Gizmo[GIZMO_YN]->GetOutput(), true);
	m_ChildViewList[0]->VmeShow((mafNode *)m_Gizmo[GIZMO_ZN]->GetOutput(), true);

	// ZN view
	m_ChildViewList[1]->VmeShow((mafNode *)m_Gizmo[GIZMO_XN]->GetOutput(), true);
	m_ChildViewList[1]->VmeShow((mafNode *)m_Gizmo[GIZMO_YN]->GetOutput(), true);

	// YN view
	m_ChildViewList[3]->VmeShow((mafNode *)m_Gizmo[GIZMO_XN]->GetOutput(), true);
	m_ChildViewList[3]->VmeShow((mafNode *)m_Gizmo[GIZMO_ZN]->GetOutput(), true);

	// ZN view
	m_ChildViewList[2]->VmeShow((mafNode *)m_Gizmo[GIZMO_YN]->GetOutput(), true);
	m_ChildViewList[2]->VmeShow((mafNode *)m_Gizmo[GIZMO_ZN]->GetOutput(), true);
}


//----------------------------------------------------------------------------
void lhpViewVolumeBrush::BrushGizmoAdd()
//----------------------------------------------------------------------------
{
	if (NULL == m_CurrentVolume)
		return;

	if (NULL == m_BrushGizmo)
		m_BrushGizmo = new lhpGizmoVolumeBrush(m_CurrentVolume, this);
	mafNode * node = (mafNode *)m_BrushGizmo->GetOutput();
	
	VmeAdd(node);

	for (int i = 0; i <CHILD_VIEWS_NUMBER; i++ )
		m_ChildViewList[i]->VmeShow(node, true);
}

//----------------------------------------------------------------------------
// only VmeRemove the brush gimzo but don't delete it
void lhpViewVolumeBrush::BrushGizmoRemove()
//----------------------------------------------------------------------------
{
	mafNode * node = (mafNode *)m_BrushGizmo->GetOutput();
	
	for (int i = 0; i <CHILD_VIEWS_NUMBER; i++ )
		m_ChildViewList[i]->VmeShow(node, false);

	VmeRemove(node);

}

//----------------------------------------------------------------------------
// remove and delete the brush gizmo
void lhpViewVolumeBrush::BrushGizmoDelete()
//----------------------------------------------------------------------------
{
	BrushGizmoRemove();
	cppDEL(m_BrushGizmo);
}
//----------------------------------------------------------------------------
void lhpViewVolumeBrush::ShowBrush(bool show)
//----------------------------------------------------------------------------
{
	if (m_BrushGizmo)
		m_BrushGizmo->Show(show);
}

//----------------------------------------------------------------------------
bool lhpViewVolumeBrush::GetBrushShow()
//----------------------------------------------------------------------------
{
	return m_BrushGizmo->GetShow();
}

//----------------------------------------------------------------------------
void lhpViewVolumeBrush::SetBrushSize(double s[3])
//----------------------------------------------------------------------------
{
	m_BrushGizmo->SetSize(s);
}

//----------------------------------------------------------------------------
void lhpViewVolumeBrush::SetBrushCenter(double pos[3])
//----------------------------------------------------------------------------
{
	m_BrushGizmo->SetCenter(pos);
}

//----------------------------------------------------------------------------
void lhpViewVolumeBrush::SetBrushShape(int t)
//----------------------------------------------------------------------------
{

	BrushGizmoRemove();
	
	m_BrushGizmo->SetBrushShape(t);

	BrushGizmoAdd();
}

//----------------------------------------------------------------------------
void  lhpViewVolumeBrush::SetHighlightColour(wxColour * colour)
//----------------------------------------------------------------------------
{
	for(int i=0; i<m_NumOfChildView; i++)
	{
		lhpPipeVolumeBrushSlice *p = (lhpPipeVolumeBrushSlice *)((mafViewSlice *)m_ChildViewList[i])->GetNodePipe(m_CurrentVolume);
		p->SetHighlightColour(colour);
		
	}
}

//----------------------------------------------------------------------------
void  lhpViewVolumeBrush::SetHighlightTransparency(int trans)
//----------------------------------------------------------------------------
{
		for(int i=0; i<m_NumOfChildView; i++)
	{
		lhpPipeVolumeBrushSlice *p = (lhpPipeVolumeBrushSlice *)((mafViewSlice *)m_ChildViewList[i])->GetNodePipe(m_CurrentVolume);
		p->SetHighlightTransparency(trans);
		
	}
}

//----------------------------------------------------------------------------
vtkSphereSource * lhpViewVolumeBrush::GetGizmoSphereSource()
//----------------------------------------------------------------------------
{
	return m_BrushGizmo->GetSphereSource();
}

//----------------------------------------------------------------------------
vtkCubeSource * lhpViewVolumeBrush::GetGizmoCubeSource()
//----------------------------------------------------------------------------
{
	return m_BrushGizmo->GetCubeSource();
}

//----------------------------------------------------------------------------
mafVME * lhpViewVolumeBrush::GetGizmoOutput()
//----------------------------------------------------------------------------
{
	return m_BrushGizmo->GetOutput();
}


//----------------------------------------------------------------------------
void lhpViewVolumeBrush::SliceGizmoRemove()
//----------------------------------------------------------------------------
{
	for(int gizmoId=GIZMO_XN; gizmoId<GIZMOS_NUMBER; gizmoId++) 
	{
		VmeRemove((mafNode *)m_Gizmo[gizmoId]->GetOutput());
	}

	GizmoDelete();
}


//----------------------------------------------------------------------------
void lhpViewVolumeBrush::SetSelectionVolume(vtkLHPSelectionVolumeGray * volume)
//----------------------------------------------------------------------------
{
	m_SelectionVolume = volume;

	// omitted ??
	for(int i=0; i<m_NumOfChildView; i++)
	{
		lhpPipeVolumeBrushSlice *p = (lhpPipeVolumeBrushSlice *)((mafViewSlice *)m_ChildViewList[i])->GetNodePipe(m_CurrentVolume);
		p->SetSelectionVolume(m_SelectionVolume);
		
	}

}


//----------------------------------------------------------------------------
void lhpViewVolumeBrush::EnableSelectionVolume(bool b)
//----------------------------------------------------------------------------
{
	m_SelVolEnabled = b;
}

//----------------------------------------------------------------------------
// adapted from the Superclass, use lhpPipeVolumeBrushSlice
void lhpViewVolumeBrush::PackageView()
//----------------------------------------------------------------------------
{
	int cam_pos[4] = {CAMERA_OS_P, CAMERA_OS_X, CAMERA_OS_Y, CAMERA_OS_Z};

	wxString viewName[4] = {"perspective","camera x","camera y","camera z"};

	bool TICKs[4]={false,false,true,true};
	for(int v=PERSPECTIVE_VIEW; v<VIEWS_NUMBER; v++)
	{
		m_Views[v] = new mafViewSlice(viewName[v], cam_pos[v],false,false,false,0,TICKs[v]);
		
		// Youbing: changed pipe here !!!
		m_Views[v]->PlugVisualPipe("mafVMEVolumeGray", "lhpPipeVolumeBrushSlice", MUTEX);    
		m_Views[v]->PlugVisualPipe("medVMELabeledVolume", "lhpPipeVolumeBrushSlice", MUTEX);
		m_Views[v]->PlugVisualPipe("mafVMEVolumeLarge", "lhpPipeVolumeBrushSlice", MUTEX);   //BES: 3.11.2009
		m_Views[v]->PlugVisualPipe("mafVMEImage", "mafPipeBox", NON_VISIBLE);
		
		
		// plug surface slice visual pipe in not perspective views
		if (v != PERSPECTIVE_VIEW)
		{
			m_Views[v]->PlugVisualPipe("mafVMESurface", "mafPipeSurfaceSlice",MUTEX);
			m_Views[v]->PlugVisualPipe("mafVMESurfaceParametric", "mafPipeSurfaceSlice",MUTEX);
			m_Views[v]->PlugVisualPipe("mafVMEMesh", "mafPipeMeshSlice");
			m_Views[v]->PlugVisualPipe("mafVMELandmark", "mafPipeSurfaceSlice",MUTEX);
			m_Views[v]->PlugVisualPipe("mafVMELandmarkCloud", "mafPipeSurfaceSlice",MUTEX);
			m_Views[v]->PlugVisualPipe("mafVMEPolyline", "mafPipePolylineSlice");
			m_Views[v]->PlugVisualPipe("mafVMEPolylineSpline", "mafPipePolylineSlice");
			m_Views[v]->PlugVisualPipe("mafVMEMeter", "mafPipePolyline");
		}
		else
		{
			m_Views[v]->PlugVisualPipe("mafVMESurface", "mafPipeSurface",MUTEX);
		}

	}

	PlugChildView(m_Views[PERSPECTIVE_VIEW]);
	PlugChildView(m_Views[ZN_VIEW]);
	PlugChildView(m_Views[XN_VIEW]);
	PlugChildView(m_Views[YN_VIEW]);

}