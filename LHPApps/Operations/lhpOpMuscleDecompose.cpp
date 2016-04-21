/*=========================================================================
Program: Musculoskeletal Modeling (VPHOP WP10)
Module: $RCSfile: lhpOpMuscleDecompose.cpp,v $
Language: C++
Date: $Date: 2011-09-07 05:42:37 $
Version: $Revision: 1.1.2.3 $
Authors: Josef Kohout
==========================================================================
Copyright (c) 2010-2011 University of West Bohemia (www.zcu.cz)
See the COPYINGS file for license details
=========================================================================
*/

#include "mafDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpMuscleDecompose.h"
#include "lhpOpSetLowerRes.h"
#include "wx/busyinfo.h"

#include "mafGUIDialog.h"
#include "mafRWIBase.h"
#include "mafRWI.h"
#include "mafDeviceButtonsPadMouse.h"
#include "mafEvent.h"

#include "mafGUIButton.h"
#include "mafGUIValidator.h"
#include "mafGUICheckListBox.h"
#include "mafVMESurface.h"
#include "mafVMELandmarkCloud.h"
#include "mafVMEPolyline.h"
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafGUI.h"
#include "mafNode.h"
#include "mafVME.h"

#include "vtkRenderWindow.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkMath.h"
#include "vtkMuscleDecomposer.h"
#include "mafMemDbg.h"
#include "mafDbg.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpMuscleDecompose);
//----------------------------------------------------------------------------

#define DEFAULT_INTERPOLATION_SUBDIVISION	10
#define DEFAULT_SURFACE_SUBDIVISION			10

//----------------------------------------------------------------------------
lhpOpMuscleDecompose::lhpOpMuscleDecompose(const wxString &label) : mafOp(label)
	//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo	= true;
	m_InputPreserving = true;

	m_MuscleMesh = NULL;
	m_OutputMesh = NULL;
	m_Selected = NULL;

	//m_OpSetHull = new lhpOpSetLowerRes();
	InterpolationSubdivision = DEFAULT_INTERPOLATION_SUBDIVISION;
	SurfaceSubdivision = DEFAULT_SURFACE_SUBDIVISION;
	int ShowData = true;
	int ShowMesh = true;
	int ShowResult = true;
}
//----------------------------------------------------------------------------
lhpOpMuscleDecompose::~lhpOpMuscleDecompose()
	//----------------------------------------------------------------------------
{
	//cppDEL(m_OpSetHull);

	mafDEL(m_Output);
}

//----------------------------------------------------------------------------
bool lhpOpMuscleDecompose::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	bool result = node && node->IsA("mafVMESurface");
	return result;
}
//----------------------------------------------------------------------------
mafOp *lhpOpMuscleDecompose::Copy()
	//----------------------------------------------------------------------------
{
	return (new lhpOpMuscleDecompose(m_Label));
}

//----------------------------------------------------------------------------
void lhpOpMuscleDecompose::OpRun()
	//----------------------------------------------------------------------------
{
	//create internal structures for the visualization
	if (!CreateInternalStructures())
	{
		mafEventMacro(mafEvent(this, OP_RUN_CANCEL));
		return;
	}

	// interface:
	CreateOpDialog();

	PopulateList();

	int result = m_Dialog->ShowModal() == wxID_OK ? OP_RUN_OK : OP_RUN_CANCEL;
	DeleteOpDialog();

	DeleteInternalStructures();
	mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
void lhpOpMuscleDecompose::OpDo()
	//----------------------------------------------------------------------------
{
	if (m_Output != NULL)
	{
		m_Output->ReparentTo(m_Input);

		//m_OpSetHull->SetInput(m_Input);
		//m_OpSetHull->SetNewHullVME(mafVME::SafeDownCast(m_Output));
		//m_OpSetHull->OpDo();
	}
}

//----------------------------------------------------------------------------
void lhpOpMuscleDecompose::OpUndo()
	//----------------------------------------------------------------------------
{
	if (m_Output != NULL)
	{
		m_Output->ReparentTo(NULL);
		mafDEL(m_Output);

		//m_OpSetHull->OpUndo();
	}
}

//----------------------------------------------------------------------------
void lhpOpMuscleDecompose::OpStop(int result)
	//----------------------------------------------------------------------------
{
	mafEventMacro(mafEvent(this,result));
}

//----------------------------------------------------------------------------
//Destroys GUI
void lhpOpMuscleDecompose::CreateOpDialog()
	//----------------------------------------------------------------------------
{
	wxBusyCursor wait;

	//===== setup interface ====
	m_Dialog = new mafGUIDialog("Muscle decomposition", mafCLOSEWINDOW | mafRESIZABLE);
	m_Dialog->SetWindowStyle(m_Dialog->GetWindowStyle() | wxMAXIMIZE_BOX);
	m_Dialog->SetMinSize(wxSize(800,600));

	//rendering window
	m_Rwi = new mafRWI(m_Dialog,ONE_LAYER,false);
	m_Rwi->SetListener(this);
	m_Rwi->CameraSet(CAMERA_PERSPECTIVE);
	m_Rwi->m_RenderWindow->SetDesiredUpdateRate(0.0001f);
	m_Rwi->SetSize(0,0,400,400);
	m_Rwi->Show(true);
	m_Rwi->m_RwiBase->SetMouse(m_Mouse);
	m_Rwi->SetBackgroundColor(wxColour(255,255,255));
	//The following code was originally generated using wxFormBuilder
	//and modified here to work with MAF
#pragma region //wxFormBuilder Component Construction
	// Controls
	wxStaticText* m_staticText4;

	wxStaticText* m_staticText1;

	wxButton* btnAddTendon;
	wxButton* btnRemoveTendon;

	wxButton* btnUpTendon;
	wxButton* btnDownTendon;

	wxStaticText* m_staticText11;

	wxButton* btnAddFiber;
	wxButton* btnRemoveFiber;

	wxButton* btnUpFiber;
	wxButton* btnDownFiber;

	wxTextCtrl* tbFiber;
	wxTextCtrl* tbSurface;

	wxStaticText* m_staticText7;
	wxStaticText* m_staticText71;

	wxCheckBox* chShowMesh;
	wxCheckBox* chShowData;
	wxCheckBox* chShowResult;

	wxButton* btnPreview;
	wxButton* btnCancel;
	wxButton* btnOK;

	// Layout

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	bSizer2->Add( m_Rwi->m_RwiBase, 1, wxALL|wxEXPAND, 5 );

	bSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( m_Dialog, wxID_ANY, wxT("Assign data") ), wxVERTICAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );

	m_staticText4 = new wxStaticText( m_Dialog, wxID_ANY, wxT("Unassigned:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer15->Add( m_staticText4, 0, wxLEFT, 5 );
//
	lbUnassigned = new wxListBox( m_Dialog, ID_UNASSIGNED_DATA, wxDefaultPosition, wxSize(0,0), 0);
	lbUnassigned->SetToolTip( wxT("This is the list of unassigned data. Every line here represents the data of a tendon or fiber. Please assingn the role of the data for the decomposition process.") );

	int test = wxHSCROLL;

	bSizer15->Add( lbUnassigned, 1, wxEXPAND | wxRIGHT|wxLEFT, 5 );

	bSizer4->Add( bSizer15, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );

	m_staticText1 = new wxStaticText( m_Dialog, wxID_ANY, wxT("Tendons"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer10->Add( m_staticText1, 0, wxRIGHT, 5 );

	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );

	bSizer14->Add( 0, 0, 1, wxEXPAND, 5 );

	btnAddTendon = new wxButton( m_Dialog, ID_ADD_TENDON, wxT(">>"), wxDefaultPosition, wxDefaultSize, 0 );
	btnAddTendon->SetToolTip( wxT("Assing data as tendon") );
	btnAddTendon->SetMinSize( wxSize( 25,20 ) );

	bSizer14->Add( btnAddTendon, 0, 0, 5 );

	btnRemoveTendon = new wxButton( m_Dialog, ID_REM_TENDON, wxT("<<"), wxDefaultPosition, wxDefaultSize, 0 );
	btnRemoveTendon->SetToolTip( wxT("Unassign tendon") );
	btnRemoveTendon->SetMinSize( wxSize( 25,20 ) );

	bSizer14->Add( btnRemoveTendon, 0, 0, 5 );

	bSizer14->Add( 0, 0, 1, wxEXPAND, 5 );

	bSizer13->Add( bSizer14, 0, wxEXPAND, 5 );

//
	lbTendons = new wxListBox( m_Dialog, ID_TENDONS, wxDefaultPosition, wxSize(0,0), 0, NULL,0 );

	bSizer13->Add( lbTendons, 1,  wxEXPAND |wxRIGHT|wxLEFT, 5 );

	wxBoxSizer* bSizer142;
	bSizer142 = new wxBoxSizer( wxVERTICAL );

	bSizer142->Add( 0, 0, 1, wxEXPAND, 5 );

	btnUpTendon = new wxButton( m_Dialog, ID_UP_TENDON, wxT("Up"), wxDefaultPosition, wxDefaultSize, 0 );
	btnUpTendon->SetToolTip( wxT("Move tendon up") );
	btnUpTendon->SetMinSize( wxSize( 25,20 ) );

	bSizer142->Add( btnUpTendon, 0, 0, 5 );

	btnDownTendon = new wxButton( m_Dialog, ID_DOWN_TENDON, wxT("Dn"), wxDefaultPosition, wxDefaultSize, 0 );
	btnDownTendon->SetToolTip( wxT("Move tendon down") );
	btnDownTendon->SetMinSize( wxSize( 25,20 ) );

	bSizer142->Add( btnDownTendon, 0, 0, 5 );

	bSizer142->Add( 0, 0, 1, wxEXPAND, 5 );

	bSizer13->Add( bSizer142, 0, wxEXPAND, 5 );

	bSizer10->Add( bSizer13, 1, wxEXPAND, 5 );

	bSizer12->Add( bSizer10, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxVERTICAL );

	m_staticText11 = new wxStaticText( m_Dialog, wxID_ANY, wxT("Fibres"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer101->Add( m_staticText11, 0, wxRIGHT, 5 );

	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer141;
	bSizer141 = new wxBoxSizer( wxVERTICAL );

	bSizer141->Add( 0, 0, 1, wxEXPAND, 5 );

	btnAddFiber = new wxButton( m_Dialog, ID_ADD_FIBER, wxT(">>"), wxDefaultPosition, wxDefaultSize, 0 );
	btnAddFiber->SetToolTip( wxT("Assing data as fiber") );
	btnAddFiber->SetMinSize( wxSize( 25,20 ) );

	bSizer141->Add( btnAddFiber, 0, 0, 5 );

	btnRemoveFiber = new wxButton( m_Dialog, ID_REM_FIBER, wxT("<<"), wxDefaultPosition, wxDefaultSize, 0 );
	btnRemoveFiber->SetToolTip( wxT("Unassign fiber") );
	btnRemoveFiber->SetMinSize( wxSize( 25,20 ) );

	bSizer141->Add( btnRemoveFiber, 0, 0, 5 );

	bSizer141->Add( 0, 0, 1, wxEXPAND, 5 );

	bSizer131->Add( bSizer141, 0, wxEXPAND, 5 );
//
	lbFibers = new wxListBox( m_Dialog, ID_FIBERS, wxDefaultPosition, wxSize(0,0), 0, NULL, 0 );

	bSizer131->Add( lbFibers, 1, wxEXPAND | wxRIGHT|wxLEFT, 5 );

	wxBoxSizer* bSizer1421;
	bSizer1421 = new wxBoxSizer( wxVERTICAL );

	bSizer1421->Add( 0, 0, 1, wxEXPAND, 5 );

	btnUpFiber = new wxButton( m_Dialog, ID_UP_FIBER, wxT("Up"), wxDefaultPosition, wxDefaultSize, 0 );
	btnUpFiber->SetToolTip( wxT("Move fiber up") );
	btnUpFiber->SetMinSize( wxSize( 25,20 ) );

	bSizer1421->Add( btnUpFiber, 0, 0, 5 );

	btnDownFiber = new wxButton( m_Dialog, ID_DOWN_FIBER, wxT("Dn"), wxDefaultPosition, wxDefaultSize, 0 );
	btnDownFiber->SetToolTip( wxT("Move fiber down") );
	btnDownFiber->SetMinSize( wxSize( 25,20 ) );

	bSizer1421->Add( btnDownFiber, 0, 0, 5 );

	bSizer1421->Add( 0, 0, 1, wxEXPAND, 5 );

	bSizer131->Add( bSizer1421, 0, wxEXPAND, 5 );

	bSizer101->Add( bSizer131, 1, wxEXPAND, 5 );

	bSizer12->Add( bSizer101, 0, wxEXPAND, 5 );

	bSizer4->Add( bSizer12, 1, wxEXPAND, 5 );

	sbSizer1->Add( bSizer4, 0, wxEXPAND, 5 );

	bSizer3->Add( sbSizer1, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( m_Dialog, wxID_ANY, wxT("Controls") ), wxVERTICAL );

	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText7 = new wxStaticText( m_Dialog, wxID_ANY, wxT("Fiber subdivision:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	fgSizer1->Add( m_staticText7, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	tbFiber = new wxTextCtrl( m_Dialog, ID_FIBRE_SUB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tbFiber->SetMinSize( wxSize( 55,20 ) );

	fgSizer1->Add( tbFiber, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxRIGHT, 5 );

	m_staticText71 = new wxStaticText( m_Dialog, wxID_ANY, wxT("Surface subdivision:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText71->Wrap( -1 );
	fgSizer1->Add( m_staticText71, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	tbSurface = new wxTextCtrl( m_Dialog, ID_SURFACE_SUB, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	tbSurface->SetMinSize( wxSize( 55,20 ) );

	fgSizer1->Add( tbSurface, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	chShowMesh = new wxCheckBox( m_Dialog, ID_SHOW_MESH, wxT("Show Mesh"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( chShowMesh, 0, wxALL, 5 );

	chShowData = new wxCheckBox( m_Dialog, ID_SHOW_DATA, wxT("Show Data"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( chShowData, 0, wxALL, 5 );

	chShowResult = new wxCheckBox( m_Dialog, ID_SHOW_RESULT, wxT("Show Result"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( chShowResult, 0, wxALL, 5 );

	bSizer16->Add( fgSizer1, 0, 0, 5 );

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );

	bSizer17->Add( 0, 0, 1, wxEXPAND, 5 );

	btnPreview = new wxButton( m_Dialog, ID_PREVIEW, wxT("Preview"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( btnPreview, 0, wxRIGHT|wxLEFT, 5 );

	btnCancel = new wxButton( m_Dialog, ID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( btnCancel, 0, wxALL, 5 );

	btnOK = new wxButton( m_Dialog, ID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( btnOK, 0, wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	bSizer16->Add( bSizer17, 0, wxEXPAND, 5 );

	bSizer23->Add( bSizer16, 1, wxEXPAND, 5 );

	sbSizer8->Add( bSizer23, 1, wxEXPAND, 5 );

	bSizer3->Add( sbSizer8, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	bSizer1->Add( bSizer3, 0, wxEXPAND|wxFIXED_MINSIZE, 5 );

#pragma endregion //wxFormBuilder
	//validators

	// buttons
	btnOK->SetValidator(mafGUIValidator(this, ID_OK, btnOK));
	btnCancel->SetValidator(mafGUIValidator(this, ID_CANCEL, btnCancel));
	btnPreview->SetValidator(mafGUIValidator(this, ID_PREVIEW, btnPreview));

	btnAddFiber->SetValidator(mafGUIValidator(this, ID_ADD_FIBER, btnAddFiber));
	btnAddTendon->SetValidator(mafGUIValidator(this, ID_ADD_TENDON, btnAddTendon));

	btnUpFiber->SetValidator(mafGUIValidator(this, ID_UP_FIBER, btnUpFiber));
	btnUpTendon->SetValidator(mafGUIValidator(this, ID_UP_TENDON, btnUpTendon));

	btnRemoveFiber->SetValidator(mafGUIValidator(this, ID_REM_FIBER, btnRemoveFiber));
	btnRemoveTendon->SetValidator(mafGUIValidator(this, ID_REM_TENDON, btnRemoveTendon));

	btnDownFiber->SetValidator(mafGUIValidator(this, ID_DOWN_FIBER, btnDownFiber));
	btnDownTendon->SetValidator(mafGUIValidator(this, ID_DOWN_TENDON, btnDownTendon));

	// show checkboxes
	chShowMesh->SetValidator(mafGUIValidator(this, ID_SHOW_MESH, chShowMesh, &(this->ShowMesh)));
	chShowData->SetValidator(mafGUIValidator(this, ID_SHOW_DATA, chShowData, &(this->ShowData)));
	chShowResult->SetValidator(mafGUIValidator(this, ID_SHOW_RESULT, chShowResult, &(this->ShowResult)));

	// textboxes
	tbFiber->SetValidator(mafGUIValidator(this, ID_FIBRE_SUB, tbFiber, &(this->InterpolationSubdivision), 0.0, 1000.0));
	tbSurface->SetValidator(mafGUIValidator(this, ID_SURFACE_SUB, tbSurface, &(this->SurfaceSubdivision), 0.0, 1000.0));

	lbUnassigned->SetValidator(mafGUIValidator(this, ID_UNASSIGNED_DATA, lbUnassigned));
	lbTendons->SetValidator(mafGUIValidator(this, ID_TENDONS, lbTendons));
	lbFibers->SetValidator(mafGUIValidator(this, ID_FIBERS, lbFibers));
	//lbUnassigned->Connect(ID_UNASSIGNED, wxEVT_COMMAND_LISTBOX_SELECTED, (wxObjectEventFunction)&lhpOpMuscleDecompose::OnListBoxSel);

	int x_pos,y_pos,w,h;
	mafGetFrame()->GetPosition(&x_pos,&y_pos);
	m_Dialog->GetSize(&w,&h);
	m_Dialog->SetSize(x_pos+5,y_pos+5,w,h);

	//and initialize the renderer window
	double bounds[6];
	m_MuscleMesh->pPoly->GetBounds(bounds);

	m_Rwi->m_RenFront->AddActor(m_MuscleMesh->pActor);
	m_Rwi->m_RenFront->ResetCamera(bounds);

	m_Dialog->Add(bSizer1, 1, wxEXPAND);

	//UpdateStatusInfo();
	UpdateVisibility();
}

//----------------------------------------------------------------------------
//Creates GUI including renderer window
void lhpOpMuscleDecompose::DeleteOpDialog()
	//----------------------------------------------------------------------------
{
	//remove all actors
	RemoveAllActors();

	m_Selected = NULL;

	delete[] m_Labels;

	cppDEL(m_Rwi);
	cppDEL(m_Dialog);
	cppDEL(m_DataSelectionDialog);
}

//----------------------------------------------------------------------------
void lhpOpMuscleDecompose::OnEvent(mafEventBase *maf_event)
	//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		wxArrayInt selections;
		switch(e->GetId())
		{
			case ID_SHOW_DATA:
			case ID_SHOW_MESH:
			case ID_SHOW_RESULT:
			//visibility options has changed
				UpdateVisibility();
				break;
		case ID_UNASSIGNED_DATA:
			 //MESH* mesh = ;
			lbTendons->DeselectAll();
			lbFibers->DeselectAll();
			lbUnassigned->GetSelections(selections);
			UpdateSelected((MESH*)(lbUnassigned->GetClientData(selections[0])));
			break;
		case ID_TENDONS:
			lbUnassigned->DeselectAll();
			lbFibers->DeselectAll();
			lbTendons->GetSelections(selections);
			UpdateSelected((MESH*)(lbTendons->GetClientData(selections[0])));
			break;
		case ID_FIBERS:
			lbUnassigned->DeselectAll();
			lbTendons->DeselectAll();
			lbFibers->GetSelections(selections);
			UpdateSelected((MESH*)(lbFibers->GetClientData(selections[0])));
			break;

		case ID_PREVIEW:
			OnPreview();
			break;

		case ID_ADD_TENDON:
			OnAddTendon();
			break;

		case ID_REM_TENDON:
			OnRemTendon();
			break;

		case ID_ADD_FIBER:
			OnAddFiber();
			break;

		case ID_REM_FIBER:
			OnRemFiber();
			break;

		case ID_UP_FIBER:
			OnMoveData(lbFibers, -1);
			break;
		case ID_UP_TENDON:
			OnMoveData(lbTendons, -1);
			break;
		case ID_DOWN_FIBER:
			OnMoveData(lbFibers, 1);
			break;
		case ID_DOWN_TENDON:
			OnMoveData(lbTendons, 1);
			break;

		case ID_OK:
			OnOk();
			m_Dialog->EndModal(wxID_OK);
			break;

		case ID_CANCEL:
			m_Dialog->EndModal(wxID_CANCEL);
			break;

		case ID_SEL_OK:
			dataNode = (mafNode*)lbDataSel->GetClientData(lbDataSel->GetSelection()); // the only handling done...
			m_DataSelectionDialog->EndModal(wxID_OK);
			break;

		case ID_SEL_CANCEL:
			m_DataSelectionDialog->EndModal(wxID_CANCEL);
			break;
		}
	}
}

#pragma region GUI Handlers
//------------------------------------------------------------------------
//Performs the deformation using the current settings.
/*virtual*/ void lhpOpMuscleDecompose::OnPreview()
	//------------------------------------------------------------------------
{
	DecomposeMuscle(true);

	//UpdateVisibility();
	//UpdateColors();
	this->m_Rwi->CameraUpdate();
}

//------------------------------------------------------------------------
//Performs the deformation and creates outputs
/*virtual*/ void lhpOpMuscleDecompose::OnOk()
	//------------------------------------------------------------------------
{
	DecomposeMuscle(false);

	//create VME
	mafVMEPolyline* line;

	mafNEW(line);

	line->SetName(wxString::Format(wxT("%s_MUSDEC"), m_Input->GetName()));
	line->SetData(m_OutputMesh->pPoly, 0, mafVMEGeneric::MAF_VME_REFERENCE_DATA);

	mafDEL(m_Output);
	m_Output = line;
}

//------------------------------------------------------------------------
void lhpOpMuscleDecompose::OnAddTendon()
//------------------------------------------------------------------------
{
	wxArrayInt selections;
	lbUnassigned->GetSelections(selections);
	int n = selections[0];
	lbTendons->Append(lbUnassigned->GetString(n), (lbUnassigned->GetClientData(n)));
	lbUnassigned->Delete(n);
	if (lbUnassigned->GetCount() > 0)
	{
		if (n > 0) {
			n--;
		}
		lbUnassigned->Select(n);
		UpdateSelected((MESH*)(lbUnassigned->GetClientData(n)));
	}
	UpdateVisibility();
}

//------------------------------------------------------------------------
void lhpOpMuscleDecompose::OnRemTendon()
//------------------------------------------------------------------------
{
	wxArrayInt selections;
	lbTendons->GetSelections(selections);
	int n = selections[0];
	lbUnassigned->Append(lbTendons->GetString(n), (lbTendons->GetClientData(n)));
	lbTendons->Delete(n);
	UpdateVisibility();
}

//------------------------------------------------------------------------
void lhpOpMuscleDecompose::OnAddFiber()
//------------------------------------------------------------------------
{
	wxArrayInt selections;
	lbUnassigned->GetSelections(selections);
	int n = selections[0];
	lbFibers->Append(lbUnassigned->GetString(n), (lbUnassigned->GetClientData(n)));
	lbUnassigned->Delete(n);
	if (lbUnassigned->GetCount() > 0)
	{
		if (n > 0) {
			n--;
		}
		lbUnassigned->Select(n);
		UpdateSelected((MESH*)(lbUnassigned->GetClientData(n)));
	}
	UpdateVisibility();
}

//------------------------------------------------------------------------
void lhpOpMuscleDecompose::OnRemFiber()
//------------------------------------------------------------------------
{
	wxArrayInt selections;
	lbFibers->GetSelections(selections);
	int n = selections[0];
	lbUnassigned->Append(lbFibers->GetString(n), (lbFibers->GetClientData(n)));
	lbFibers->Delete(n);
	UpdateVisibility();
}

void lhpOpMuscleDecompose::OnMoveData(wxListBox* lb, int dir)
{
	wxArrayInt selections;
	lb->GetSelections(selections);
	int sel = selections[0];
	if (sel + dir >= 0 && sel + dir < lb->GetCount())
	{
		void* data = lb->GetClientData(sel);
		wxString lbl = lb->GetString(sel);

		lb->SetClientData(sel, lb->GetClientData(sel+dir));
		lb->SetString(sel, lb->GetString(sel+dir));

		lb->SetClientData(sel+dir, data);
		lb->SetString(sel+dir, lbl);

		lb->Select(sel+dir);
	}
}

#pragma endregion //GUI Handlers

//------------------------------------------------------------------------
//Creates internal data structures used in the editor.
//Returns false, if an error occurs (e.g. unsupported input)
/*virtual*/ bool lhpOpMuscleDecompose::CreateInternalStructures()
	//------------------------------------------------------------------------
{
	mafVMESurface* surface = mafVMESurface::SafeDownCast(m_Input);
	_VERIFY_RETVAL(surface != NULL, false);

	vtkPolyData* pPoly = vtkPolyData::SafeDownCast(surface->GetOutput()->GetVTKData());
	_VERIFY_RETVAL(pPoly != NULL, false);
	pPoly->Register(NULL);  //this prevents its destruction

	mafNode* node = m_Input;
	mafNode* parentNode = node->GetParent();
	mafNode* muscleGeomNode = NULL;

	_VERIFY_RETVAL(parentNode != NULL, false);
	_VERIFY_EXPR_RPT_RETVAL(!strcmp(parentNode->GetName(), "Muscles"), "Selected node is not a muscle surface.", false)

	for (int i = 0; i < parentNode->GetChildren()->size(); i++)
	{
		if (!strcmp(parentNode->GetChild(i)->GetName(), "Muscle Geometry"))
		{
			muscleGeomNode = parentNode->GetChild(i);
			break;
		}
	}
	_VERIFY_EXPR_RPT_RETVAL(muscleGeomNode != NULL, "Selected muscle group doesn't have geometry defined.", false)

	CreateDataSelectionDialog(muscleGeomNode);

	if (m_DataSelectionDialog->ShowModal() != wxID_OK) // assigns dataNode
	{
		return false;
	}

	if (dataNode == NULL) return false;

	// now we are finally confident we CAN have all data needed
	// data may not be present, but we atleast have a way of detecting that

	// create muslce mesh

	m_MuscleMesh = CreateMesh(pPoly);                //original muscle mesh
	m_MuscleMesh->pActor->GetProperty()->SetRepresentationToWireframe();
	m_MuscleMesh->pActor->GetProperty()->SetAmbient(1);
	m_MuscleMesh->pActor->GetProperty()->SetAmbientColor(MUSCLE_COLOR);
	m_MuscleMesh->pActor->GetProperty()->SetDiffuse(0);
	m_MuscleMesh->pActor->GetProperty()->SetDiffuseColor(MUSCLE_COLOR);
	m_MuscleMesh->pActor->GetProperty()->SetColor(MUSCLE_COLOR); //white
	//m_MuscleMesh->pActor->GetProperty()->SetOpacity(0.5); //white

	m_OutputMesh = CreateMesh(vtkPolyData::New());  // empty
	m_OutputMesh->pActor->GetProperty()->SetColor(FIBER_COLOR);

	// create all fiber/tendon data
	dataCount = dataNode->GetChildren()->size();
	m_Labels = new wxString[dataCount];
	m_Data = new MESH*[dataCount];

	mafNode* dummy;  // for iteration only

	for	(int i = 0; i < dataCount; i++)
	{
		m_Data[i] = NULL;
		m_Labels[i] = wxString("");
		if ((dummy = dataNode->GetChild(i))->IsA("mafVMELandmarkCloud"))
		{
			mafVMELandmarkCloud* cloud = mafVMELandmarkCloud::SafeDownCast(dummy);

			// the filter accepts vtkPolyData on input anyways, we may as well cast it now to be sure
			vtkPolyData* data = vtkPolyData::SafeDownCast(cloud->GetOutput()->GetVTKData());
			data->Update();

			if (data != NULL)
			{
				ConvertToPolyline(data);
				data->Register(NULL);  //this prevents its destruction
				m_Labels[i] = dummy->GetName();
				m_Data[i] = CreateMesh(data);
				m_Data[i]->pActor->GetProperty()->SetColor(UNASSIGNED_COLOR);
				m_Data[i]->pActor->GetProperty()->SetLineWidth(1.5f);
				m_Data[i]->pMapper->ScalarVisibilityOff();
				//m_Data[i]->pActor->GetProperty()->SetRepresentationToWireframe();
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
//Destroys internal data structures created by CreateInternalStructures
/*virtual*/ void lhpOpMuscleDecompose::DeleteInternalStructures()
	//------------------------------------------------------------------------
{
	// TODO finish it all!
	// destroy meshes
	if (m_Data)
	{
		for (int i = 0; i < dataCount; i++)
		{
			if(m_Data[i])
			{
				vtkDEL(m_Data[i]->pActor);
				vtkDEL(m_Data[i]->pMapper);
				vtkDEL(m_Data[i]->pPoly);
				cppDEL(m_Data[i]);
			}
		}
	}
	cppDEL(m_Data);
	//delete[] m_Labels; // no idea how to do this...

	vtkDEL(m_MuscleMesh->pActor);
	vtkDEL(m_MuscleMesh->pMapper);
	vtkDEL(m_MuscleMesh->pPoly);
	cppDEL(m_MuscleMesh);

	vtkDEL(m_OutputMesh->pActor);
	vtkDEL(m_OutputMesh->pMapper);
	vtkDEL(m_OutputMesh->pPoly);
	cppDEL(m_OutputMesh);
}

//------------------------------------------------------------------------
//This method creates the internal mesh structure.
//It constructs also the VTK pipeline.
lhpOpMuscleDecompose::MESH* lhpOpMuscleDecompose::CreateMesh(vtkPolyData* pMesh)
	//------------------------------------------------------------------------
{
	MESH* pRet = new MESH;
	memset(pRet, 0, sizeof(MESH));

	pRet->pPoly = pMesh;
	pRet->pMapper = vtkPolyDataMapper::New();
	pRet->pMapper->ScalarVisibilityOff();

	pRet->pActor = vtkActor::New();
	pRet->pActor->SetMapper(pRet->pMapper);
	pRet->pActor->PickableOff();

	UpdateMesh(pRet);
	return pRet;
}

//------------------------------------------------------------------------
//Updates the VTK pipeline for the given curve.
//N.B. It does not connects actors into the renderer.
void lhpOpMuscleDecompose::UpdateMesh(MESH* pMesh)
	//------------------------------------------------------------------------
{
	if (pMesh->pPoly == NULL)
		pMesh->pActor->SetVisibility(0);
	else
	{
		pMesh->pMapper->SetInput(pMesh->pPoly);
		pMesh->pMapper->Update();

		pMesh->pActor->SetVisibility(1);
	}
}

//------------------------------------------------------------------------
//Updates the visibility of meshes
//It adds/removes actors from the renderer according to the
//status of their associated data and the visual options
//specified by the user in the GUI.
//This method is typically calls after UpdateMesh or
//UpdateControlCurve is finished
void lhpOpMuscleDecompose::UpdateVisibility()
	//------------------------------------------------------------------------
{
	//remove all actors and
	RemoveAllActors();

	if (ShowMesh)
	{
		m_Rwi->m_RenFront->AddActor(m_MuscleMesh->pActor);
	}

	if (ShowResult)
	{
		m_Rwi->m_RenFront->AddActor(m_OutputMesh->pActor);
	}

	if (ShowData)
	{
		for (int i = 0; i < dataCount; i++)
		{
			if(m_Data[i])
			{
				m_Rwi->m_RenFront->AddActor(m_Data[i]->pActor);
			}
		}
	}
	UpdateColors();
	//readd actors according to the visual options
	//for (int i = 0; i < 2; i++)
	//{
	//	if (m_Visibility[i] != 0);
	//			m_Rwi->m_RenFront->AddActor(m_Mesh->pActor);
	//}

	this->m_Rwi->CameraUpdate();
	this->m_Rwi->CameraUpdate(); // to fix rendering issues (dark lines);
}

void lhpOpMuscleDecompose::UpdateSelected(MESH* pMesh)
	//------------------------------------------------------------------------
{
	if (pMesh)
	{
		this->m_Selected = pMesh;
		UpdateColors();
		this->m_Rwi->CameraUpdate();
		this->m_Rwi->CameraUpdate();
	}
}

void lhpOpMuscleDecompose::UpdateColors()
{
	for (int i = 0; i < lbUnassigned->GetCount(); i++)
	{
		MESH* data = (MESH*)(lbUnassigned->GetClientData(i));
		if (data)
		{
			if (data == this->m_Selected)
			{
				data->pActor->GetProperty()->SetColor(SELECTION_COLOR);
			}
			else
			{
				data->pActor->GetProperty()->SetColor(UNASSIGNED_COLOR);
			}
		}
	}
	for (int i = 0; i < lbFibers->GetCount(); i++)
	{
		MESH* data = (MESH*)(lbFibers->GetClientData(i));
		if (data)
		{
			if (data == this->m_Selected)
			{
				data->pActor->GetProperty()->SetColor(SELECTION_COLOR);
			}
			else
			{
				data->pActor->GetProperty()->SetColor(FIBER_COLOR);
			}
		}
	}
	for (int i = 0; i < lbTendons->GetCount(); i++)
	{
		MESH* data = (MESH*)(lbTendons->GetClientData(i));
		if (data)
		{
			if (data == this->m_Selected)
			{
				data->pActor->GetProperty()->SetColor(SELECTION_COLOR);
			}
			else
			{
				data->pActor->GetProperty()->SetColor(TENDON_COLOR);
			}
		}
	}
}

//------------------------------------------------------------------------
//Removes all actors from the renderer.
void lhpOpMuscleDecompose::RemoveAllActors()
	//------------------------------------------------------------------------
{
	m_Rwi->m_RenFront->RemoveActor(m_MuscleMesh->pActor);
	m_Rwi->m_RenFront->RemoveActor(m_OutputMesh->pActor);
	for (int i = 0; i < dataCount; i++)
	{
		if(m_Data[i])
		{
			m_Rwi->m_RenFront->RemoveActor(m_Data[i]->pActor);
		}
	}
}

//------------------------------------------------------------------------
//Updates status info
void lhpOpMuscleDecompose::UpdateStatusInfo()
	//------------------------------------------------------------------------
{
	wxString szStr = wxString::Format("Original Mesh: %d vertices, %d polygons\r\n"
		"Hull Mesh: %d vertices, %d polygons\n");

	//m_StatusHelp->SetLabel(szStr);
}

//------------------------------------------------------------------------
// Populates the unassigned data list
void lhpOpMuscleDecompose::PopulateList()
//------------------------------------------------------------------------
{
	for (int i = 0; i < dataCount; i++)
	{
		if (m_Data[i])
		{
			lbUnassigned->Append(m_Labels[i], (void*) m_Data[i]); // label and ref to the data
		}
	}
}

void lhpOpMuscleDecompose::CreateDataSelectionDialog(mafNode* muscleGeometryNode)
{
	m_DataSelectionDialog = new mafGUIDialog("Select matching data", mafCLOSEWINDOW);
	//m_DataSelectionDialog->SetWindowStyle(m_DataSelectionDialog->GetWindowStyle());

	#pragma region wxFromBuilder

	wxString labelText = wxString::Format("Please select data matching the muscle mesh '%s'", m_Input->GetName());

	wxStaticText* lblText;
	//wxListBox* lbDataSel;

	wxButton* btnSelOK;
	wxButton* btnSelCancel;

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );

	lblText = new wxStaticText( m_DataSelectionDialog, wxID_ANY, wxT(labelText), wxDefaultPosition, wxSize( 300,30 ), 0 );
	lblText->Wrap( -1 );
	bSizer18->Add( lblText, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );

	lbDataSel = new wxListBox( m_DataSelectionDialog, ID_DATA_SEL, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	bSizer19->Add( lbDataSel, 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );

	bSizer20->Add( 0, 0, 1, wxEXPAND, 5 );

	btnSelOK = new wxButton( m_DataSelectionDialog, ID_SEL_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( btnSelOK, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	btnSelCancel = new wxButton( m_DataSelectionDialog, ID_SEL_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( btnSelCancel, 0, wxALL, 5 );

	bSizer20->Add( 0, 0, 1, wxEXPAND, 5 );

	bSizer19->Add( bSizer20, 0, wxEXPAND, 5 );

	bSizer18->Add( bSizer19, 1, wxEXPAND, 5 );

	m_DataSelectionDialog->Add(bSizer18);

	#pragma endregion wxFromBuilder

	// Validators
	btnSelOK->SetValidator(mafGUIValidator(this, ID_SEL_OK, btnSelOK));
	btnSelCancel->SetValidator(mafGUIValidator(this, ID_SEL_CANCEL, btnSelCancel));
	lbDataSel->SetValidator(mafGUIValidator(this, ID_DATA_SEL, lbDataSel));

	// Populate the data list
	int selections = muscleGeometryNode->GetChildren()->size();
	wxString* labels = new wxString[selections];
	mafNode** data = new mafNode*[selections];

	for	(int i = 0; i < selections; i++)
	{
		data[i] = muscleGeometryNode->GetChild(i);
		labels[i] = data[i]->GetName();
	}

	lbDataSel->Set(selections, labels, (void**)data);
}

void lhpOpMuscleDecompose::ConvertToPolyline(vtkPolyData* pointData)
{
	//vtkCellArray* lines = vtkCellArray::New();
	vtkIdType * lineIDs = new vtkIdType[2];
	pointData->SetLines(vtkCellArray::New());
	for	(int i = 1; i < pointData->GetPoints()->GetNumberOfPoints(); i++)
	{
		lineIDs[0] = i - 1;
		lineIDs[1] = i;
		pointData->GetLines()->InsertNextCell(2, lineIDs);
	}
}

//------------------------------------------------------------------------
//Computes the hull mesh for the input mesh
void lhpOpMuscleDecompose::DecomposeMuscle(bool useTuber)
//------------------------------------------------------------------------
{
	//mafDEL(m_OutputMesh->pPoly);
	//m_OutputMesh->pPoly = vtkPolyData::New();
	vtkMuscleDecomposer* musdec = vtkMuscleDecomposer::New();
	musdec->SetTendonConnectionMethod(CONNECT_CLOSEST_TENDON_POINT);
	musdec->SetArtifactEpsilon(10);
	musdec->SetInterpolationMethod(SPLINE_CONSTRAINED);
	musdec->SetInterpolationSubdivision(InterpolationSubdivision);
	musdec->SetSurfaceSubdivision(SurfaceSubdivision);

	musdec->AddMeshData(m_MuscleMesh->pPoly);

	for (int i = 0; i < lbTendons->GetCount(); i++)
	{
		MESH* data = (MESH*)(lbTendons->GetClientData(i));
		if (data)
		{
			musdec->AddTendonData(data->pPoly);
		}
	}

	for (int i = 0; i < lbFibers->GetCount(); i++)
	{
		MESH* data = (MESH*)(lbFibers->GetClientData(i));
		if (data)
		{
			musdec->AddFiberData(data->pPoly);
		}
	}

	if (useTuber)
	{
		vtkTubeFilter* tuber = vtkTubeFilter::New();
		tuber->SetNumberOfSides(4);
		//musdec->SetOutput(tuber->GetInput);
		tuber->SetInput(musdec->GetOutput(0));
		tuber->SetOutput(m_OutputMesh->pPoly);

		tuber->Update();

		musdec->SetOutput(NULL);	//detach output from the filter
		musdec->Delete();

		tuber->SetOutput(NULL);
		tuber->Delete();
	}
	else
	{
		musdec->SetOutput(m_OutputMesh->pPoly);
		musdec->Update();

		musdec->SetOutput(NULL);	//detach output from the filter
		musdec->Delete();
	}
}