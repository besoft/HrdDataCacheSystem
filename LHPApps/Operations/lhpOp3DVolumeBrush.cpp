/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOp3DVolumeBrush.cpp,v $
Language:  C++
Date:      $Date: 2010-06-24 15:37:38 $
Version:   $Revision: 1.1.2.17 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2001/2005 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "lhpOp3DVolumeBrush.h"

#include "mafVME.h"
#include "mafVMEVolumeGray.h"
#include "mafGUI.h"

#include "mafDeviceManager.h"
#include "medDeviceButtonsPadMouseDialog.h"
#include "mafInteractorPicker.h"
#include "mafAction.h"
#include "mafInteractorSER.h"
#include "mafInteractionFactory.h"
#include "mafGUIMDIFrame.h"
//#include "mafGUIMDIChild.h"

#include "lhpViewVolumeBrush.h"
#include "lhpFrameVolumeBrush.h"
#include "lhpInteractorVolumeBrush.h"
#include "vtkLHPSelectionVolumeGray.h"

#include "vtkImageData.h"
#include "vtkStructuredPoints.h"
#include "vtkRectilinearGrid.h"


enum VOLUMEBRUSH_ID
{
	ID_BRUSH_SHAPE = mafViewOrthoSlice::ID_LAST,
	ID_SLIDER_BRUSHSIZE,
	ID_RENDER_MODE,
	ID_BUTTON_LOAD_SELVOL,
	ID_BUTTON_SAVE,
	ID_BUTTON_CLEAR,
	ID_BUTTON_THRESHOLD_SEL,
	ID_THRESHOLD,
	ID_COLOUR_HIGHLIGHT,
	ID_SLIDER_SEL_TRANS

};


// RTTI support
mafCxxTypeMacro(lhpOp3DVolumeBrush);

//----------------------------------------------------------------------------
// Constructor
lhpOp3DVolumeBrush::lhpOp3DVolumeBrush(wxString label):mafOp(label)
//----------------------------------------------------------------------------
{
	m_BrushShape = 0;
	m_BrushSize = 20;
	m_RenderMode = 0;

	m_DeviceManager = NULL;
	m_DialogMouse = NULL;
	m_Volume = NULL;
	m_View = NULL;
	// not use m_Picker to support zooming when mouse is inside the VME
	m_Picker = NULL;

	m_Spacing[0] = m_Spacing[1] = m_Spacing[2] = 1.0;
	m_Origin[0] = m_Origin[1] = m_Origin[2] = 0.0;

	m_SelectionVME = NULL;
	m_LoadedSelectionVME = NULL;
	m_SelectionVolume = NULL;

	m_LowerThreshold = 0;
	m_UpperThreshold = 0;

	m_TransHighlight = 0.65 * 255;
	// yellow
	m_ColourHighlight = new wxColour(255, 255, 0);

}

//----------------------------------------------------------------------------
// Destructor
lhpOp3DVolumeBrush::~lhpOp3DVolumeBrush(void)
//----------------------------------------------------------------------------
{
	if (m_SelectionVME != m_LoadedSelectionVME)
		mafDEL(m_SelectionVME);

	delete m_ColourHighlight;
	m_ColourHighlight = NULL;
}

mafOp * lhpOp3DVolumeBrush::Copy()
{
	return new lhpOp3DVolumeBrush();
}

//----------------------------------------------------------------------------
// Accept surface or volume data
bool lhpOp3DVolumeBrush::Accept(mafNode * vme)
//----------------------------------------------------------------------------
{
	return (vme != NULL &&  vme->IsMAFType(mafVMEVolumeGray)) ;
}


//----------------------------------------------------------------------------
// Static copy of accept function
bool lhpOp3DVolumeBrush::AcceptStatic(mafNode* vme)
//----------------------------------------------------------------------------
{
	return (vme != NULL && vme->IsMAFType(mafVMEVolumeGray)) ;
}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::OnEvent(mafEventBase * e)
//----------------------------------------------------------------------------
{

	if ((e->GetId() < mafViewOrthoSlice::ID_LAST) && (e->GetId() > MINID))
		m_View->OnEvent(e);
	else {
		switch (e->GetId())
		{
			case ID_BRUSH_SHAPE:
				if (m_BrushShape == 0)
					m_PER->SetBrushShape(BRUSH_SHAPE_SPHERE);
				else if (m_BrushShape == 1)
					m_PER->SetBrushShape(BRUSH_SHAPE_CUBE);
				break;
			case ID_SLIDER_BRUSHSIZE:
				m_PER->SetBrushSize(m_BrushSize);
				break;
			case ID_RENDER_MODE :
				if (0 == m_RenderMode)
					m_PER->SetSelectionVolumeRenderMode(RENDER_MODE_NONE);
				else if (1 == m_RenderMode)
					m_PER->SetSelectionVolumeRenderMode(RENDER_MODE_ISOSURFACE);
				else if (2 == m_RenderMode)
					m_PER->SetSelectionVolumeRenderMode(RENDER_MODE_VOLUME);
				break;
			case ID_BUTTON_LOAD_SELVOL:
				OnLoadSelectionVolume();
				break;
			case ID_BUTTON_SAVE:
				break;
			case ID_BUTTON_CLEAR :
				m_PER->ClearSelectionVolume();
				break;
			case ID_BUTTON_THRESHOLD_SEL:
				m_PER->SelectByThreshold(m_LowerThreshold, m_UpperThreshold);
				break;
			case ID_COLOUR_HIGHLIGHT:
				m_PER->SetHighlightColour(m_ColourHighlight);
				break;
			case ID_SLIDER_SEL_TRANS:
				m_PER->SetHighlightTransparency(m_TransHighlight);
				break;
			case wxOK:
				OpStop(OP_RUN_OK);
				break;
			case wxCANCEL:
					OpStop(OP_RUN_CANCEL);
				break;
		}
	}
	
}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::OpRun()
//----------------------------------------------------------------------------
{
	// mafVMEVolumeGray can be vtkImageData or vtkRectilinearGrid
	m_Volume=mafVMEVolumeGray::SafeDownCast(m_Input);

	// get extent
	vtkImageData * pData = vtkImageData::SafeDownCast(m_Volume->GetOutput()->GetVTKData());

	// rectilinear
	if (NULL == pData)
	{
		//vtkRectilinearGrid * pDataRect = vtkRectilinearGrid::SafeDownCast(m_Volume->GetOutput()->GetVTKData());
		//pDataRect->GetDimensions(m_Dimensions);
		wxString message = _("vtkRectilinearGrid currently not supported");
		wxMessageBox(message);

		OpStop(OP_RUN_CANCEL);
		return;

	}
	else
	{
		pData->GetDimensions(m_Dimensions);
		pData->GetSpacing(m_Spacing);
		pData->GetOrigin(m_Origin);
	}

	//create selection volume
	vtkNEW(m_SelectionVolume); // = lhpSelectionVolumeGray::New();
	m_SelectionVolume->Create(m_Dimensions, m_Spacing, m_Origin);


	//Setup a new mouse (medDeviceButtonsPadMouseDialog)

	//Create the device manager
	mafNEW(m_DeviceManager);
	m_DeviceManager->SetListener(this);
	m_DeviceManager->SetName("DialogDeviceManager");
	m_DeviceManager->Initialize();

	//Create the static event router and connect it
	mafNEW(m_SER);
	m_SER->SetName("StaticEventRouter");
	m_SER->SetListener(this);
	//Define the action for pointing and manipulating
	mafAction *pntAction = m_SER->AddAction("pntAction",-10);
	//Create a Mouse device
	mafPlugDevice<medDeviceButtonsPadMouseDialog>("Mouse");
	m_DialogMouse = (medDeviceButtonsPadMouseDialog *)m_DeviceManager->AddDevice("medDeviceButtonsPadMouseDialog",false); // add as persistent device
	assert(m_DialogMouse);
	m_DialogMouse->SetName("DialogMouse");

	
	CreateOpFrame();

	//create the positional event router
	mafNEW(m_PER);
	m_PER->SetName("m_PER");
	m_PER->SetVolumeData(pData);
	m_PER->SetSelectionVolume(m_SelectionVolume);
	m_PER->SetBrushSize(m_BrushSize);
	m_PER->SetBrushShape(m_BrushShape);
	m_PER->SetSelectionVolumeRenderMode(m_RenderMode);
	m_PER->SetListener(this);
	m_PER->SetView(m_View);
	m_PER->SetFrame(m_Frame);

	// not use m_Picker to support zooming when mouse is inside the VME
	//m_Picker = mafInteractorPicker::New();
	//m_Picker->SetListener(this);
	//m_Picker->AddObserver(this);
	//m_PER->AddObserver(m_Picker);
	

	m_OldBehavior=m_Volume->GetBehavior();
	//m_Volume->SetBehavior(m_Picker);
	m_Volume->SetBehavior(NULL);

	pntAction->BindDevice(m_DialogMouse);
	pntAction->BindInteractor(m_PER);
	
	
	CreateGUI();

	ShowGui();	

}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::CreateGUI()
//----------------------------------------------------------------------------
{
	// setup Gui on the right panel
	//m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

	m_Gui->Label(_("Selection highlighting"),true);
	m_Gui->Color(ID_COLOUR_HIGHLIGHT, _("Colour"),m_ColourHighlight);

	m_Gui->Slider(ID_SLIDER_SEL_TRANS, _("Opacity"), &m_TransHighlight, 0, 255, "", true );

	m_Gui->Divider(2);

	wxString sBrushType[2] = {_("sphere"), _("cube")};

	m_Gui->Label(_("Brush Shape"),true);
	m_Gui->Radio(ID_BRUSH_SHAPE,"",&m_BrushShape,2, sBrushType, 2);
	m_Gui->Divider(2);

	m_Gui->Label(_("Brush Size(Radius)"),true);
	m_Gui->Slider(ID_SLIDER_BRUSHSIZE, "", &m_BrushSize, 1, 30, "", true );
	m_Gui->Divider(2);

	wxString sRenderMode[3] = {_("none"), _("surface"),_("volume")};
	m_Gui->Label(_("Selection Rendering"),true);
	m_Gui->Radio(ID_RENDER_MODE,"", &m_RenderMode, 3, sRenderMode, 2);
	m_Gui->Divider(2);

	// to specify volume data ranges
	double sr[2];
	 m_Volume->GetOutput()->GetVTKData()->GetScalarRange(sr);
	m_Gui->Label(_("Selection"),true);
	m_Gui->TwoButtons(ID_BUTTON_LOAD_SELVOL, ID_BUTTON_CLEAR, "Load", "Clear");
	//m_Gui->Button(ID_BUTTON_SAVE, "Save");
	//m_Gui->Button(ID_BUTTON_CLEAR, "Clear");
	m_Gui->Divider(2);

	m_Gui->Label(_("Selection from threshold"),true);
	m_Gui->Double(ID_THRESHOLD,_("Lower"),&m_LowerThreshold,sr[0],sr[1]);
	m_Gui->Double(ID_THRESHOLD,_("Upper"),&m_UpperThreshold,sr[0],sr[1]);
	m_Gui->Button(ID_BUTTON_THRESHOLD_SEL, "Generate selection");

	m_Gui->Divider(2);

	m_Gui->OkCancel();
}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::OpDo()
//----------------------------------------------------------------------------
{
	Superclass::OpDo();
}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::OpUndo()
//----------------------------------------------------------------------------
{
	// todo
}

//----------------------------------------------------------------------------
// only accept vtkImageData (vtkStructuredPoints and vtkUniformGrid)
bool lhpOp3DVolumeBrush::AcceptSelectionVolume(mafNode * vme)
//----------------------------------------------------------------------------
{
	if (vme == NULL)
		return false;
	

	if (vme->IsMAFType(mafVMEVolumeGray)) 
	{
		mafVMEVolumeGray * volumeGray = mafVMEVolumeGray::SafeDownCast(vme);
		mafVMEOutputVolume * mafVolume = volumeGray->GetVolumeOutput();
		mafVolume->Update();
		const char * typeName = mafVolume->GetVTKDataTypeAsString();
		
		// compare
		if (!strcmp(typeName, "vtkLHPSelectionVolumeGray") || !strcmp(typeName, "vtkStructuredPoints") 
			|| !strcmp(typeName, "vtkUniformGrid")|| !strcmp(typeName, "vtkImageData"))
		{
			vtkImageData * imageData = mafVolume->GetStructuredData();
			int dim[3]; 
			double origin[3];
			double spacing[3];
			imageData->GetDimensions(dim);
			imageData->GetSpacing(spacing);
			imageData->GetOrigin(origin);


			// currently we require the dimensions of the seleciton volume is the same as the original volume data
			if ((m_Dimensions[0] != dim[0]) || (m_Dimensions[1] != dim[1]) || (m_Dimensions[0] != dim[0]))
			{
				wxString wmsg = _("The dimension of the selection volume data doesn't match the current volume data");
				wxMessageBox(wmsg,_("incorrect vme type"),wxOK|wxICON_ERROR);
				return false;
			}

			if ( fabs(m_Origin[0] - origin[0]) > 1e-7 || fabs(m_Origin[1] - origin[1]) > 1e-7 || fabs(m_Origin[2] - origin[2]) > 1e-7)
			{
				wxString wmsg = _("The origin of the selection volume data doesn't match the current volume data");
				wxMessageBox(wmsg,_("incorrect vme type"),wxOK|wxICON_ERROR);
				return false;
			}

			if ( fabs(m_Spacing[0] - spacing[0]) > 1e-7 || fabs(m_Spacing[1] - spacing[1]) > 1e-7 || fabs(m_Spacing[2] - spacing[2]) > 1e-7)
			{
				wxString wmsg = _("The spacing of the selection volume data doesn't match the current volume data");
				wxMessageBox(wmsg,_("incorrect vme type"),wxOK|wxICON_ERROR);
				return false;
			}
			
			// check if the data is unsigned char
			if (imageData->GetScalarType() != VTK_UNSIGNED_CHAR)
			{
				wxString wmsg = _("The size of the selection volume data doesn't match the current volume data");
				wxMessageBox(wmsg,_("incorrect vme type"),wxOK|wxICON_ERROR);
				return false;
			}

			return true;

		}
		else
		{
			char msg[120];
			sprintf(msg, "Selection volume cannot be %s\n please choose another unsigned char mafVMEVolumeGray\n", typeName); 
			wxString wmsg = _(msg);
			wxMessageBox(wmsg,_("incorrect vme type"),wxOK|wxICON_ERROR);
			return false;
		}

	}
	else
	{
		wxString wmsg = _("Selection volume must be a unsigned short mafVMEVolumeGray\n please choose another vme \n");
		wxMessageBox(wmsg,_("incorrect vme type"),wxOK|wxICON_ERROR);
		return false;
	}
}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::OpStop(int result)
//----------------------------------------------------------------------------
{
	if (m_View)
	{
		m_PER->HideSelectionVolume();
		m_PER->ShowBrush(false);
	}


	if (m_Gui)
		HideGui();
	//mafEventMacro(mafEvent(this,PER_POP));	
	
	mafDEL(m_PER);

	if (m_View)
		DeleteOpFrame();
	
	// add the selection VME to data tree
	if (result == OP_RUN_OK)
	{
		if (NULL == m_SelectionVME) 
		{
			if (NULL == m_LoadedSelectionVME)
			{
				mafNEW(m_SelectionVME);
				m_SelectionVME->SetName("selection volume");
			}
			else
			{
				m_SelectionVME = m_LoadedSelectionVME;
			}

		}
		m_SelectionVME->SetData(m_SelectionVolume, -1);
		m_Output = m_SelectionVME;
	}

	vtkDEL(m_SelectionVolume);

	

	mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
// adpated from medOpClassicICPRegistration::OnChooseTarget()
void lhpOp3DVolumeBrush::OnLoadSelectionVolume()
//----------------------------------------------------------------------------
{
	mafEvent e(this,VME_CHOOSE);
	mafEventMacro(e);
	mafNode *vme = e.GetVme();

	if(!vme) return; // the user choosed cancel - keep previous target

	if(!AcceptSelectionVolume(vme)) // the user choosed ok     - check if it is a valid vme
	{
		return;
	}

	// check if the volume is a unsigned byte volume
	// load the selection volume
	mafVMEVolumeGray * m_LoadedSelectionVME = mafVMEVolumeGray::SafeDownCast(vme);
	mafVMEOutputVolume * mafVolume = m_LoadedSelectionVME->GetVolumeOutput();
	mafVolume->Update();
	vtkImageData * imageData = mafVolume->GetStructuredData();
	//m_SelectionVME = volumeGray;
	m_SelectionVolume->DeepCopy(imageData);

	m_PER->ShowSelectionVolume();

	m_PER->UpdateAllViews();

}


//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::CreateOpFrame()
//----------------------------------------------------------------------------
{
	// the app frame ( main frame)
	wxWindow * appFrame = mafGetFrame();
	mafGUIMDIFrame * mdiFrame = (mafGUIMDIFrame *) appFrame;


	//Create rendering view
	m_View = new lhpViewVolumeBrush("VolumeBrushView");  

	m_View->PackageView();
	m_View->Create();
	m_View->VmeAdd(m_Input->GetRoot()); //add Root
	m_View->VmeAdd(m_Input); // add volume data

	// setting it after view created
	m_View->SetMouse(m_DialogMouse);
	m_DialogMouse->SetView(m_View);


	// create a child frame of the mainframe
	m_Frame = new lhpFrameVolumeBrush(mdiFrame,m_View);
	m_Frame->SetWindowStyleFlag(wxCAPTION | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxRESIZE_BORDER);
	m_Frame->Maximize();
	//c->SetListener(m_ViewManager);
	m_View->SetFrame(m_Frame);

	m_Gui = m_View->GetGui();

	m_View->VmeShow(m_Input, true);
	m_View->SliceGizmoAdd();
	m_View->BrushGizmoAdd();
	m_View->SetSelectionVolume(m_SelectionVolume);
	m_View->SetHighlightColour(m_ColourHighlight);
	m_View->SetHighlightTransparency(m_TransHighlight);


}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::DeleteOpFrame()
//----------------------------------------------------------------------------
{
	m_Volume->SetBehavior(m_OldBehavior);

	m_Frame->Show(false);

	m_View->SliceGizmoRemove();
	m_View->BrushGizmoDelete();

	cppDEL(m_View); 
	cppDEL(m_Frame);

	mafDEL(m_OldBehavior);

	m_DeviceManager->Shutdown();

	mafDEL(m_DeviceManager);
	mafDEL(m_PER);
	mafDEL(m_SER);
	mafDEL(m_Picker);
}

//----------------------------------------------------------------------------
void lhpOp3DVolumeBrush::HideGui()
//----------------------------------------------------------------------------
{
  assert(m_Gui); 
  mafEventMacro(mafEvent(this,OP_HIDE_GUI,(wxWindow *)m_Guih));
  
  // cannot delete m_Gui for it's from the view
  //delete m_Guih;
  //m_Guih = NULL;
  //m_Gui = NULL;
}
