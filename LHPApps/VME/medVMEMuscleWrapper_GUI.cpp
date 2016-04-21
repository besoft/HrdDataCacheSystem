/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMEMuscleWrapper_GUI.cpp,v $
Language:  C++
Date:      $Date: 2012-04-30 14:52:43 $
Version:   $Revision: 1.1.2.6 $
Authors:   Josef Kohout
==========================================================================
Copyright (c) 2008-2012
University of Bedforshire, University of West Bohemia
=========================================================================*/

#include "mafDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------
#include "medVMEMuscleWrapper.h"
#include "mafGUI.h"
#include "mafGUIValidator.h"

#include <assert.h>
#include "mafDbg.h"
#include "mafMemDbg.h"

#pragma region GUI and Events Handling
#include <wx/tooltip.h>
#include <wx/listctrl.h>

//-------------------------------------------------------------------------
mafGUI* medVMEMuscleWrapper::CreateGui()
	//-------------------------------------------------------------------------
{
	m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
	m_Gui->SetListener(this);
	bool motionFused = GetMusculoSkeletalModel() != GetMusculoSkeletalModel_RP();

#ifdef VTK2OBJ
	m_Gui->Button(ID_LAST, "To .OBJ");
#endif

	//make sure we have all links restored
	if (!m_bLinksRestored)
	{
		RestoreMeterLinks();
		m_bLinksRestored = true;
	}

	m_MuscleVmeName = m_MuscleVme == NULL ? wxT("") : m_MuscleVme->GetName();
	for (int i = 0; i < 2; i++){
		m_OIVMEName[i] = m_OIVME[i] == NULL ? wxT("") : m_OIVME[i]->GetName();
	}

#pragma region Generated Code from wxFormBuilder
	wxBoxSizer* bSizer18 = new wxBoxSizer( wxVERTICAL );
	//BES: Advanced mode is the only one supported in VPHOP
	wxStaticBoxSizer* sbSizer14 = new wxStaticBoxSizer(
		new wxStaticBox( m_Gui, wxID_ANY, wxT("Operational Mode") ), wxVERTICAL );

	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	wxCheckBox* chckUseFastChecks = new wxCheckBox( m_Gui, ID_USE_FASTCHECKS, wxT("Fast Checks"),
		wxDefaultPosition, wxDefaultSize, 0 );
	chckUseFastChecks->SetToolTip( wxT("If checked the method does not perform a detection, whether "
		"input has changed externally. It allows faster run but in some special case it might not "
		"reflect changes of input muscles or wrappers, transformations, etc. ") );
	bSizer30->Add( chckUseFastChecks, 1, wxALL, 5 );
	sbSizer14->Add( bSizer30, 1, wxEXPAND, 1 );

	wxCheckBox* checkBox9 = new wxCheckBox( m_Gui, ID_GENERATE_FIBERS,
		wxT("Generate fibers"), wxDefaultPosition, wxDefaultSize, 0 );
	checkBox9->SetToolTip( wxT("If checked, the output are muscle fibers instead of deformed surface.") );
	sbSizer14->Add( checkBox9, 0, wxALL, 5 );
	//bSizer18->Add( sbSizer14, 0, wxEXPAND, 1 );

	bSizer18->Add( sbSizer14, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* sbSizer8 = new wxStaticBoxSizer(
		new wxStaticBox( m_Gui, wxID_ANY, wxT("Muscle Surface") ), wxVERTICAL );

	wxBoxSizer* bSizer19 = new wxBoxSizer( wxHORIZONTAL );
	wxTextCtrl* MuscleCtrl = new wxTextCtrl( m_Gui, wxID_ANY,
		wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer19->Add( MuscleCtrl, 1, wxALL, 1 );

	wxButton* bttnSelectMuscle = new wxButton( m_Gui,
		ID_RESTPOSE_MUSCLE_LINK, wxT("Select"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bttnSelectMuscle->SetToolTip( wxT("Selects the VME representing the muscle surface to be wrapped.") );

	bSizer19->Add( bttnSelectMuscle, 0, wxALL, 1 );
	sbSizer8->Add( bSizer19, 1, wxEXPAND, 1 );
	bSizer18->Add( sbSizer8, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* sbSizer9 = new wxStaticBoxSizer(
		new wxStaticBox( m_Gui, wxID_ANY, wxT("Wrappers") ), wxVERTICAL );

	m_WrappersCtrl = new wxListCtrl( m_Gui, ID_LIST_WRAPPERS, wxDefaultPosition,
		wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL );
	sbSizer9->Add( m_WrappersCtrl, 1, wxALL|wxEXPAND, 1 );

	wxBoxSizer* bSizer121 = new wxBoxSizer( wxHORIZONTAL );
	bSizer121->Add( new wxPanel( m_Gui, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, wxTAB_TRAVERSAL ), 1, wxALL, 5 );

	m_BttnRemoveWrapper = new wxButton( m_Gui, ID_REMOVEWRAPPER, wxT("Remove"),
		wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_BttnRemoveWrapper->Enable( false );
	m_BttnRemoveWrapper->SetToolTip( wxT("Removes selected wrapper.") );
	bSizer121->Add( m_BttnRemoveWrapper, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	sbSizer9->Add( bSizer121, 0, wxEXPAND, 1 );

#pragma region Add Wrapper
	wxStaticBoxSizer* sbSizer13 = new wxStaticBoxSizer(
		new wxStaticBox( m_Gui, wxID_ANY, wxT("Add New Wrapper") ), wxVERTICAL );

	wxBoxSizer* bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	m_LabelRP = new wxStaticText( m_Gui, wxID_ANY, wxT("RP:"), wxDefaultPosition,
		wxSize( 25,-1 ), wxALIGN_RIGHT );	

	bSizer5->Add( m_LabelRP, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	m_RPNameCtrl = new wxTextCtrl( m_Gui, ID_RESTPOSE_WRAPPER_LINK, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	
	bSizer5->Add( m_RPNameCtrl, 1, wxALL, 1 );

	m_BttnSelRP = new wxButton( m_Gui, ID_SELECT_RP, wxT("Select"),
		wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_BttnSelRP->SetToolTip(
		wxT("Selects VME with the wrapper (in the rest pose). \n\n"
		"Note: the mesh deformation is governed by the difference between the output "
		"from wrappers in the rest pose and the current pose.") );

	bSizer5->Add( m_BttnSelRP, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sbSizer13->Add( bSizer5, 0, wxEXPAND, 1 );

	wxBoxSizer* bSizer51 = new wxBoxSizer( wxHORIZONTAL );
	bSizer51->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("CP:"), wxDefaultPosition,
		wxSize( 25,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* CPNameCtrl = new wxTextCtrl( m_Gui, ID_CURRENTPOSE_WRAPPER_LINK, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer51->Add( CPNameCtrl, 1, wxALL, 1 );

	wxButton* bttnSelCP = new wxButton( m_Gui, ID_SELECT_CP, wxT("Select"),
		wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bttnSelCP->SetToolTip(
		wxT("Selects VME with the wrapper (in the current pose). \n\n"
		"Note: the mesh deformation is governed by the difference between the output "
		"from wrappers in the rest pose and the current pose.") );

	bSizer51->Add( bttnSelCP, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sbSizer13->Add( bSizer51, 0, wxEXPAND, 1 );

	wxBoxSizer* bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	m_LabelRP_RS = new wxStaticText( m_Gui, wxID_ANY, wxT("RP RS:"),
		wxDefaultPosition, wxSize( 35,-1 ), wxALIGN_RIGHT );
	bSizer29->Add( m_LabelRP_RS, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	m_RPRefSysVmeCtrl = new wxTextCtrl( m_Gui, ID_RESTPOSE_REFSYS_LINK,
		wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );	
	bSizer29->Add( m_RPRefSysVmeCtrl, 1, wxALL, 1 );

	m_BttnSelectRPRefSys = new wxButton( m_Gui, ID_SELECT_RP_REFSYS_LINK,
		wxT("Select"), wxDefaultPosition, wxSize( 50,-1 ), 0 );	
	m_BttnSelectRPRefSys->SetToolTip(
		wxT("[OPTIONAL] Selects a VME that serves as a reference coordinate system for "
		"deformation (see Use RS option).") );
	bSizer29->Add( m_BttnSelectRPRefSys, 0, wxALL, 1 );
	sbSizer13->Add( bSizer29, 1, wxEXPAND, 1 );

	wxBoxSizer* bSizer291 = new wxBoxSizer( wxHORIZONTAL );
	bSizer291->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("CP RS:"), wxDefaultPosition,
		wxSize( 35,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* CPRefSysVmeCtrl = new wxTextCtrl( m_Gui, ID_CURRENTPOSE_REFSYS_LINK,
		wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer291->Add( CPRefSysVmeCtrl, 1, wxALL, 1 );

	wxButton* bttnSelectCPRefSys = new wxButton( m_Gui, ID_SELECT_CP_REFSYS_LINK,
		wxT("Select"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bttnSelectCPRefSys->SetToolTip(
		wxT("[OPTIONAL] Selects a VME that serves as a reference coordinate system for "
		"deformation (see Use RS option).") );

	bSizer291->Add( bttnSelectCPRefSys, 0, wxALL, 1 );
	sbSizer13->Add( bSizer291, 1, wxEXPAND, 1 );

	wxBoxSizer* bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	bSizer12->Add( new wxPanel( m_Gui, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTAB_TRAVERSAL ), 1, wxALL, 1 );

	m_BttnAddWrapper = new wxButton( m_Gui, ID_ADDWRAPPER, wxT("Add"),
		wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_BttnAddWrapper->Enable( false );
	m_BttnAddWrapper->SetToolTip( wxT("Adds a new wrapper") );

	bSizer12->Add( m_BttnAddWrapper, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );
	sbSizer13->Add( bSizer12, 1, wxEXPAND, 1 );
	sbSizer9->Add( sbSizer13, 0, wxEXPAND, 1 );
	bSizer18->Add( sbSizer9, 1, wxEXPAND, 1 );
#pragma endregion Add Wrapper

#pragma region Fibers Options
	wxStaticBoxSizer* sbSizer15 = new wxStaticBoxSizer(
		new wxStaticBox( m_Gui, wxID_ANY, wxT("Fibers Options") ), wxVERTICAL );

	wxBoxSizer* bSizer15m = new wxBoxSizer( wxHORIZONTAL );
	bSizer15m->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Method:"), wxDefaultPosition,
		wxSize( 50,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxComboBox* fibersMethod = new wxComboBox( m_Gui, ID_DECOMPOSITION_METHOD, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );

	fibersMethod->Append(wxT("Simple Slicing (LHDL)"));
	fibersMethod->Append(wxT("Advanced Slicing (VPHOP)"));
	fibersMethod->Append(wxT("Mass-Spring System (VPHOP)"));
	fibersMethod->Append(wxT("Updated Particle (VPHOP)"));
#ifdef ADV_KUKACKA
	fibersMethod->Append(wxT("Advanced Kukacka (VPHOP)"));
#endif
	fibersMethod->SetToolTip(wxT("Selects the method to be used for fibers generation. "
		"Updated Particle (VPHOP) is only worked when selecting the Mass-spring system method as the muscle deformation method.") );

	bSizer15m->Add( fibersMethod, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	sbSizer15->Add(bSizer15m, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer511 = new wxBoxSizer( wxHORIZONTAL );
	bSizer511->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("O:"), wxDefaultPosition,
		wxSize( 30,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* OAreaName = new wxTextCtrl( m_Gui, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer511->Add( OAreaName, 1, wxALL, 1 );

	wxButton* bttnSelOA = new wxButton( m_Gui, ID_FIBERS_ORIGIN_LINK, wxT("Select"),
		wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bttnSelOA->SetToolTip(
		wxT("[OPTIONAL] Select the VME (a single landmark or a landmark cloud) representing "
		"the origin area.\n\nN.B. This information (together with I) is used to determine "
		"the direction of fibers. If neither O nor I is specified, the generated "
		"fibers might be incorrect.") );
	bSizer511->Add( bttnSelOA, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sbSizer15->Add( bSizer511, 0, wxEXPAND, 1 );

	wxBoxSizer* bSizer5111 = new wxBoxSizer( wxHORIZONTAL );
	bSizer5111->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("I:"), wxDefaultPosition,
		wxSize( 30,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* IAreaName = new wxTextCtrl( m_Gui, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer5111->Add( IAreaName, 1, wxALL, 1 );

	wxButton* bttnSelIA = new wxButton( m_Gui, ID_FIBERS_INSERTION_LINK, wxT("Select"),
		wxDefaultPosition, wxSize( 50,-1 ), 0 );
	bttnSelIA->SetToolTip(
		wxT("Select the VME (a single landmark or a landmark cloud) representing the "
		"insertion area.\n\nN.B. This information (together with O) is used to determine "
		"the direction of fibers. If neither O nor I is specified, the generated "
		"fibers might be incorrect.") );
	bSizer5111->Add( bttnSelIA, 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sbSizer15->Add( bSizer5111, 0, wxEXPAND, 1 );

	wxBoxSizer* bSizer51111 = new wxBoxSizer( wxHORIZONTAL );
	bSizer51111->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Type:"), wxDefaultPosition,
		wxSize( 30,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxString choice1Choices[] = { wxT("parallel"), wxT("pennate"), wxT("curved"),
		wxT("fanned"), wxT("rectus") };

	int choice1NChoices = sizeof( choice1Choices ) / sizeof( wxString );
	wxComboBox* choice1 = new wxComboBox( m_Gui, ID_FIBERS_TEMPLATE, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, choice1NChoices, choice1Choices, wxCB_READONLY );
	choice1->SetToolTip( wxT("Selects the geometry type of fibers for the current muscle.") );
	bSizer51111->Add( choice1, 1, wxALL, 1 );

	bSizer51111->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Num.:"), wxDefaultPosition,
		wxSize( 35,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* textCtrl20 = new wxTextCtrl( m_Gui, ID_FIBERS_NUMFIB, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, 0 );
	textCtrl20->SetToolTip( wxT("Specifies the number of fibers to be created within muscle volume.") );
	bSizer51111->Add( textCtrl20, 1, wxALL, 1 );
	sbSizer15->Add( bSizer51111, 0, wxEXPAND, 1 );

	wxBoxSizer* bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	bSizer58->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Res.:"), wxDefaultPosition,
		wxSize( 30,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* textCtrl201 = new wxTextCtrl( m_Gui, ID_FIBERS_RESOLUTION, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, 0 );
	textCtrl201->SetToolTip( wxT("Specifies the resolution of fiber.") );
	bSizer58->Add( textCtrl201, 1, wxALL, 1 );

	bSizer58->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Thick.:"),
		wxDefaultPosition, wxSize( 35,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	wxTextCtrl* textCtrl2011 = new wxTextCtrl( m_Gui, ID_FIBERS_THICKNESS,
		wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	textCtrl2011->SetToolTip(
		wxT("Specifies the thickness of fibers. If the thickness is 0, fibers are "
		"represented by polylines, otherwise they are represented by cylinders with "
		"radius equaled to the given thickness.") );

	bSizer58->Add( textCtrl2011, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	sbSizer15->Add( bSizer58, 0, wxEXPAND, 1 );

	wxCheckBox* checkBox12 = new wxCheckBox( m_Gui, ID_FIBLER_UNIFORM_SAMPLING, wxT("Uniform Sampling"), wxDefaultPosition, wxDefaultSize, 1 );
	checkBox12->SetToolTip(wxT("If checked, the fibers are sampled uniformly in the template space.") );
	sbSizer15->Add( checkBox12, 0, wxALL, 5 );

	wxCheckBox* checkBox10 = new wxCheckBox( m_Gui, ID_FIBERS_SMOOTH, wxT("Smooth fibers"),
		wxDefaultPosition, wxDefaultSize, 0 );
	checkBox10->SetToolTip( wxT("If checked, a smoothing process is applied on the generated fibers") );

	sbSizer15->Add( checkBox10, 0, wxALL, 5 );

#pragma region Smoothing Options
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Smoothing Options") ), wxVERTICAL );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );

	m_SmLabel1 = new wxStaticText( m_Gui, wxID_ANY, wxT("Steps:"), wxDefaultPosition, wxSize( 30,-1 ), wxALIGN_RIGHT );
	bSizer6->Add( m_SmLabel1, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	m_SmStepsCtrl = new wxTextCtrl( m_Gui, ID_FIBERS_SMOOTH_STEPS, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
	m_SmStepsCtrl->SetToolTip( wxT("Specifies the number of smoothing iterations (higher value means more smoothed fibres)") );

	bSizer6->Add( m_SmStepsCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	m_SmLabel2 = new wxStaticText( m_Gui, wxID_ANY, wxT("Weight:"), wxDefaultPosition, wxSize( 45,-1 ), wxALIGN_RIGHT );
	bSizer6->Add( m_SmLabel2, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );

	m_SmWeightCtrl = new wxTextCtrl( m_Gui, ID_FIBERS_SMOOTH_STEPS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_SmWeightCtrl->SetToolTip( wxT("Specifies the number of smoothing iterations (higher value means more smoothed fibres)") );

	bSizer6->Add( m_SmWeightCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
	sbSizer16->Add( bSizer6, 0, wxEXPAND, 5 );
	sbSizer15->Add( sbSizer16, 0, wxEXPAND, 5 );
#pragma endregion Smoothing Options
#pragma endregion Fibers Options
	bSizer18->Add( sbSizer15, 0, wxEXPAND, 1 );

#pragma region Particle Options
	if (!motionFused)
	{
		wxStaticBoxSizer* sbSizer17 = new wxStaticBoxSizer( new wxStaticBox( m_Gui, wxID_ANY, wxT("Particles Options") ), wxVERTICAL );

		wxBoxSizer* bSizer59 = new wxBoxSizer( wxHORIZONTAL );
		bSizer59->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("First Bone Distance:"), wxDefaultPosition, wxSize( 100,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
		wxTextCtrl* textCtrl202 = new wxTextCtrl( m_Gui, ID_PARTICLE_CONSTRAINT_THRESHOLD, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
		textCtrl202->SetToolTip( wxT("Specifies the threshold distance for the judgement of fixed particles.") );
		bSizer59->Add( textCtrl202, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
		sbSizer17->Add( bSizer59, 0, wxEXPAND, 1 );

		wxBoxSizer* bSizer61 = new wxBoxSizer( wxHORIZONTAL );
		bSizer61->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Second Bone Distance:"), wxDefaultPosition, wxSize( 100,-1 ), wxALIGN_RIGHT ), 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
		wxTextCtrl* textCtrl203 = new wxTextCtrl( m_Gui, ID_PARTICLE_BONE_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 30,-1 ), 0 );
		textCtrl203->SetToolTip( wxT("Specifies the distance threshold for the judgement of the second nearest bone of particles.") );
		bSizer61->Add( textCtrl203, 1, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
		sbSizer17->Add( bSizer61, 0, wxEXPAND, 1 );

		// layout
		wxBoxSizer* bSizer60 = new wxBoxSizer( wxHORIZONTAL );

		bSizer60->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxSize( 30,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
		wxString layoutTypes[] = { wxT("6modelCube"), wxT("26modelCube"), wxT("Delaunay"), wxT("N-nearest") };
		int layoutNum = sizeof( layoutTypes ) / sizeof( wxString );
		wxComboBox* layoutChoice = new wxComboBox( m_Gui, ID_SPRING_LAYOUT_TYPE, wxEmptyString, wxDefaultPosition, wxSize(40, -1), layoutNum, layoutTypes, wxCB_READONLY );
		layoutChoice->SetToolTip( wxT("Selects the layout model of the spring used in the mass spring method.") );
		bSizer60->Add( layoutChoice, 1, wxALL, 1 );

		bSizer60->Add( new wxStaticText( m_Gui, wxID_ANY, wxT("Num.:"), wxDefaultPosition, wxSize( 35,-1 ), wxALIGN_RIGHT ), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1 );
		wxTextCtrl* textCtrl204 = new wxTextCtrl( m_Gui, ID_CLOSEST_PARTICLE_NUMBER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
		textCtrl204->SetToolTip( wxT("Specifies the number of closest particles to be used when the N-nearest layout is chosen.") );
		bSizer60->Add( textCtrl204, 1, wxALL, 1 );
		sbSizer17->Add( bSizer60, 0, wxEXPAND, 1 );

		bSizer18->Add( sbSizer17, 0, wxEXPAND, 1 );

		//validators for Particles Options
		textCtrl202->SetValidator(mafGUIValidator(this, ID_PARTICLE_CONSTRAINT_THRESHOLD, textCtrl202, &m_PConstraintThreshold, 0.0, 100.0));
		textCtrl203->SetValidator(mafGUIValidator(this, ID_PARTICLE_BONE_DISTANCE, textCtrl203, &m_PBoneDistance, 0.0, 500.0));
		layoutChoice->SetValidator(mafGUIValidator(this, ID_SPRING_LAYOUT_TYPE, layoutChoice, &m_PSpringLayoutType));
		textCtrl204->SetValidator(mafGUIValidator(this, ID_CLOSEST_PARTICLE_NUMBER, textCtrl204, &m_PClosestParticleNum, 0, 100));
	}
#pragma endregion Particle Options

#ifdef _DEBUG_VIS_
	wxStaticBoxSizer* sbSizer15d = new wxStaticBoxSizer(
		new wxStaticBox( m_Gui, wxID_ANY, wxT("Debug Options") ), wxVERTICAL );

	wxStaticBoxSizer* bSizer15def = new wxStaticBoxSizer(new wxStaticBox( m_Gui, wxID_ANY, wxT("Deformation") ), wxHORIZONTAL );

	wxCheckBox* chckDebugD1 = new wxCheckBox( m_Gui, ID_DEFORM_DEBUG_SHOWALL, wxT("Show input"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebugD1->SetToolTip( wxT("If checked, the input for deformation is visualized.") );
	bSizer15def->Add( chckDebugD1, 0, wxALL, 5 );

	wxCheckBox* chckDebugD2 = new wxCheckBox( m_Gui, ID_DEFORM_DEBUG_SHOWPROGRESS, wxT("Show progress"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebugD2->SetToolTip( wxT("If checked, the successive deformation is visualized - only PK method.") );
	bSizer15def->Add( chckDebugD2, 0, wxALL, 5 );

	sbSizer15d->Add( bSizer15def, 0, wxEXPAND, 1 );

	wxStaticBoxSizer* bSizer15fib = new wxStaticBoxSizer(new wxStaticBox( m_Gui, wxID_ANY, wxT("Decomposition") ), wxVERTICAL );

	wxCheckBox* checkBox11 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWTEMPLATE, wxT("Show template"), wxDefaultPosition, wxDefaultSize, 0 );
	checkBox11->SetToolTip( wxT("If checked, the output is a set of fibres with a cube - target cube") );
	bSizer15fib->Add( checkBox11, 0, wxALL, 5 );

	wxBoxSizer* bSizer15fib1 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* chckDebug2 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWFITTING, wxT("Show fitting"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug2->SetToolTip( wxT("If checked, the fitting process is visualized") );
	bSizer15fib1->Add( chckDebug2, 0, wxALL, 5 );

	wxCheckBox* chckDebug3 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWFITTINGRES, wxT("Show fitting result"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug3->SetToolTip( wxT("If checked, the fitting process is visualized") );
	bSizer15fib1->Add( chckDebug3, 0, wxALL, 5 );

	bSizer15fib->Add( bSizer15fib1, 0, wxALL, 5 );

	wxBoxSizer* bSizer15fib2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* chckDebug4 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWSLICING, wxT("Show slicing"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug4->SetToolTip( wxT("If checked, the slicing process is visualized") );
	bSizer15fib2->Add( chckDebug4, 0, wxALL, 5 );

	wxCheckBox* chckDebug5 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWSLICINGRES, wxT("Show slicing result"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug5->SetToolTip( wxT("If checked, the slicing process is visualized") );
	bSizer15fib2->Add( chckDebug5, 0, wxALL, 5 );
	bSizer15fib->Add( bSizer15fib2, 0, wxALL, 5 );

	wxBoxSizer* bSizer15fib4 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* chckDebug41 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWOICONSTRUCT, wxT("Show attachment areas"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug4->SetToolTip( wxT("If checked, the process of construction of surface attachment areas is visualized") );
	bSizer15fib4->Add( chckDebug41, 0, wxALL, 5 );
#ifdef ADV_KUKACKA
	wxCheckBox* chckDebug51 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOWHARMFUNC, wxT("Show harmonic function"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug5->SetToolTip( wxT("If checked, the harmonic function on the surface is visualized") );
	bSizer15fib4->Add( chckDebug51, 0, wxALL, 5 );
#endif
	bSizer15fib->Add( bSizer15fib4, 0, wxALL, 5 );

	wxBoxSizer* bSizer15fib3 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* chckDebug6 = new wxCheckBox( m_Gui, ID_FIBERS_DEBUG_SHOW_CONSTRAINT_ONLY, wxT("Show contrainted particles only"), wxDefaultPosition, wxDefaultSize, 0 );
	chckDebug4->SetToolTip( wxT("If checked, only the contrainted particles are visualized") );
	bSizer15fib3->Add( chckDebug6, 0, wxALL, 5 );
	bSizer15fib->Add( bSizer15fib3, 0, wxALL, 5 );

	sbSizer15d->Add( bSizer15fib, 0, wxEXPAND, 1 );
	bSizer18->Add( sbSizer15d, 0, wxEXPAND, 1 );
#endif
#pragma endregion

	//create headers for the listctrl
	wxString cols[4] = { wxT("RP"), wxT("CP"), wxT("RP_RS"), wxT("CP_RS") };
	for (int i = 0; i < 4; i++){
		m_WrappersCtrl->InsertColumn(i, cols[i]);
	}

	m_WrappersCtrl->SetValidator(mafGUIValidator(this, ID_LIST_WRAPPERS, m_WrappersCtrl));

	//validators for the first part
	//radioBox1->SetValidator(mafGUIValidator(this, ID_INPUTMODE, radioBox1, &m_InputMode));
	checkBox9->SetValidator(mafGUIValidator(this, ID_GENERATE_FIBERS, checkBox9, &m_VisMode));
	chckUseFastChecks->SetValidator(mafGUIValidator(this, ID_USE_FASTCHECKS, chckUseFastChecks, &m_UseFastChecks));
	MuscleCtrl->SetValidator(mafGUIValidator(this, wxID_ANY, MuscleCtrl, &m_MuscleVmeName));
	bttnSelectMuscle->SetValidator(mafGUIValidator(this, ID_RESTPOSE_MUSCLE_LINK, bttnSelectMuscle));

	//validators for Wrappers
	m_BttnSelRP->SetValidator(mafGUIValidator(this, ID_SELECT_RP, m_BttnSelRP));
	bttnSelCP->SetValidator(mafGUIValidator(this, ID_SELECT_CP, bttnSelCP));
	m_BttnSelectRPRefSys->SetValidator(mafGUIValidator(this, ID_SELECT_RP_REFSYS_LINK, m_BttnSelectRPRefSys));
	bttnSelectCPRefSys->SetValidator(mafGUIValidator(this, ID_SELECT_CP_REFSYS_LINK, bttnSelectCPRefSys));
	m_BttnAddWrapper->SetValidator(mafGUIValidator(this, ID_ADDWRAPPER, m_BttnAddWrapper));
	m_BttnRemoveWrapper->SetValidator(mafGUIValidator(this, ID_REMOVEWRAPPER, m_BttnRemoveWrapper));
	m_RPNameCtrl->SetValidator(mafGUIValidator(this, ID_RESTPOSE_WRAPPER_LINK, m_RPNameCtrl, &m_WrappersVmeName[0]));
	CPNameCtrl->SetValidator(mafGUIValidator(this, ID_CURRENTPOSE_WRAPPER_LINK, CPNameCtrl, &m_WrappersVmeName[1]));
	m_RPRefSysVmeCtrl->SetValidator(mafGUIValidator(this, ID_RESTPOSE_REFSYS_LINK, m_RPRefSysVmeCtrl, &m_RefSysVmeName[0]));
	CPRefSysVmeCtrl->SetValidator(mafGUIValidator(this, ID_CURRENTPOSE_REFSYS_LINK, CPRefSysVmeCtrl, &m_RefSysVmeName[1]));

	//validators for Fiber Options
	fibersMethod->SetValidator(mafGUIValidator(this, ID_DECOMPOSITION_METHOD, fibersMethod, &m_DecompositionMethod));
	OAreaName->SetValidator(mafGUIValidator(this, wxID_ANY, OAreaName, &m_OIVMEName[0]));
	IAreaName->SetValidator(mafGUIValidator(this, wxID_ANY, IAreaName, &m_OIVMEName[1]));
	bttnSelOA->SetValidator(mafGUIValidator(this, ID_FIBERS_ORIGIN_LINK, bttnSelOA));
	bttnSelIA->SetValidator(mafGUIValidator(this, ID_FIBERS_INSERTION_LINK, bttnSelIA));
	choice1->SetValidator(mafGUIValidator(this, ID_FIBERS_TEMPLATE, choice1, &m_FbTemplate));
	textCtrl20->SetValidator(mafGUIValidator(this, ID_FIBERS_NUMFIB, textCtrl20, &m_FbNumFib, 1, 10000));
	textCtrl201->SetValidator(mafGUIValidator(this, ID_FIBERS_RESOLUTION, textCtrl201, &m_FbResolution, 1, 499));
	textCtrl2011->SetValidator(mafGUIValidator(this, ID_FIBERS_THICKNESS, textCtrl2011, &m_FbThickness, 0.0, MAXDOUBLE, -1));
	checkBox12->SetValidator(mafGUIValidator(this, ID_FIBLER_UNIFORM_SAMPLING, checkBox12, &m_FbUniformSampling));
	checkBox10->SetValidator(mafGUIValidator(this, ID_FIBERS_SMOOTH, checkBox10, &m_FbSmooth));
	m_SmStepsCtrl->SetValidator(mafGUIValidator(this, ID_FIBERS_SMOOTH_STEPS, m_SmStepsCtrl, &m_FbSmoothSteps, 1, 100));
	m_SmWeightCtrl->SetValidator(mafGUIValidator(this, ID_FIBERS_SMOOTH_WEIGHT, m_SmWeightCtrl, &m_FbSmoothWeight, 0.0, MAXDOUBLE, -1));

#ifdef _DEBUG_VIS_
	chckDebugD1->SetValidator(mafGUIValidator(this, ID_DEFORM_DEBUG_SHOWALL, chckDebugD1, &m_DefDebugShow));
	chckDebugD2->SetValidator(mafGUIValidator(this, ID_DEFORM_DEBUG_SHOWPROGRESS, chckDebugD2, &m_DefDebugShowSteps));
	checkBox11->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWTEMPLATE, checkBox11, &m_FbDebugShowTemplate));
	chckDebug2->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWFITTING, chckDebug2, &m_FbDebugShowFitting));
	chckDebug3->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWFITTINGRES, chckDebug3, &m_FbDebugShowFittingRes));
	chckDebug4->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWSLICING, chckDebug4, &m_FbDebugShowSlicing));
	chckDebug5->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWSLICINGRES, chckDebug5, &m_FbDebugShowSlicingRes));
	chckDebug41->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWOICONSTRUCT, chckDebug41, &m_FbDebugShowOIConstruction));
#ifdef ADV_KUKACKA
	chckDebug51->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOWHARMFUNC, chckDebug51, &m_FbDebugShowHarmFunc));
#endif
	chckDebug6->SetValidator(mafGUIValidator(this, ID_FIBERS_DEBUG_SHOW_CONSTRAINT_ONLY, chckDebug6, &m_FbDebugShowConstraints));	
#endif

	//  radioBox1->GetToolTip()->GetWindow()->SetMaxSize(wxSize(1600, 75));

	m_Gui->Add(bSizer18);
	m_Gui->FitGui();

	//populate list
	for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin();
		pItem != m_Wrappers.end(); pItem++){
			AddWrapper(*pItem);
	}

	m_BttnRemoveWrapper->Enable((int)m_Wrappers.size() > 0);


	UpdateControls();
	//InternalUpdate();  //DO NOT CALL InternalUpdate
	return m_Gui;
}

//-------------------------------------------------------------------------
void medVMEMuscleWrapper::OnEvent(mafEventBase *maf_event)
	//-------------------------------------------------------------------------
{
	// events to be sent up or down in the tree are simply forwarded
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		bool bNeedUpdate = false;
		bool bEventHandled = true;

		int nId = e->GetId();
		switch (nId)
		{
#ifdef VTK2OBJ
		case ID_LAST:
			ExportToOBJ(m_PolyData, m_MuscleVmeName);
			break;
#endif
		case ID_USE_FASTCHECKS:
			//change of checks mode => update everything
			for (CWrapperItemCollectionIterator pItem = m_Wrappers.begin();
				pItem != m_Wrappers.end(); pItem++){
					(*pItem)->Modified = CWrapperItem::EVERYTHING_MODIFIED;
			}

			m_Modified |= EVERYTHING_MODIFIED;
			bNeedUpdate = true;
			break;

#pragma region INPUT CHANGES
		case ID_RESTPOSE_MUSCLE_LINK:
			{
				if (bNeedUpdate = SelectVme(_("Choose muscle vme link (in the rest pose)"),
					(long)&medVMEMuscleWrapper::VMEAcceptMuscle, m_MuscleVme, m_MuscleVmeName))
				{
					//new muscle is here => we need to pass its matrix to our output
					mafMatrix matrix;
					m_MuscleVme->GetOutput()->GetMatrix(matrix, 0.0);	//get the matrix for the time 0.0
					this->SetMatrix(matrix);

					m_Modified |= INPUT_MUSCLE_MODIFIED;	//this will trigger DEFORMATION_OPTIONS_MODIFIED
				}
			}
			break;

		case ID_SELECT_RP_REFSYS_LINK:
			SelectVme(_("Choose the reference system for the rest pose wrapper"),
				(long)&medVMEMuscleWrapper::VMEAcceptRefSys, m_RefSysVme[0], m_RefSysVmeName[0]);
			break;

		case ID_SELECT_CP_REFSYS_LINK:
			SelectVme(_("Choose the reference system for the rest pose wrapper"),
				(long)&medVMEMuscleWrapper::VMEAcceptRefSys, m_RefSysVme[1], m_RefSysVmeName[1]);
			break;

		case ID_SELECT_RP:
			if (SelectVme(_("Choose wrapper vme link (in the rest pose)"),
				(long)&medVMEMuscleWrapper::VMEAcceptWrapper, m_WrappersVme[0], m_WrappersVmeName[0])
				)
				m_BttnAddWrapper->Enable(m_WrappersVme[1] != NULL);
			break;

		case ID_SELECT_CP:
			if (SelectVme(_("Choose wrapper vme link (in the current pose)"),
				(long)&medVMEMuscleWrapper::VMEAcceptWrapper, m_WrappersVme[1], m_WrappersVmeName[1])
				)
				m_BttnAddWrapper->Enable(m_WrappersVme[0] != NULL);
			break;

		case ID_ADDWRAPPER:
			AddWrapper(m_WrappersVme[0], m_WrappersVme[1], m_RefSysVme[0], m_RefSysVme[1]);
			for (int i = 0; i < 2; i++)
			{
				m_RefSysVme[i] = m_WrappersVme[i] = NULL;
				m_RefSysVmeName[i] = m_WrappersVmeName[i] = wxT("");
			}

			m_BttnAddWrapper->Enable(FALSE);
			m_Gui->Update();

			m_BttnRemoveWrapper->Enable();

			m_Modified |= INPUT_WRAPPER_MODIFIED;			//this will trigger DEFORMATION_OPTIONS_MODIFIED
			bNeedUpdate = true;
			break;

		case ID_REMOVEWRAPPER:
			{
				//find the selected item
				int nIndex = m_WrappersCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
				if (nIndex >= 0)
				{
					RemoveWrapper(nIndex);

					m_BttnRemoveWrapper->Enable((int)m_Wrappers.size() != 0);
					m_Modified |= DEFORMATION_OPTIONS_MODIFIED;	//we need just to call deformation
					bNeedUpdate = true;
				}

				break;
			}
#pragma endregion INPUT CHANGES

		case ID_GENERATE_FIBERS:
			//this switch just require that output is produced, nothing else
			m_Modified |= POLYDATA_MUST_UPDATE;
			bNeedUpdate = true;
			break;

#pragma region FIBRES OPTIONS CHANGES
		case ID_DECOMPOSITION_METHOD:
			//Decomposition method has been changed
			m_Modified |= FIBERS_OPTIONS_MODIFIED;
			bNeedUpdate = m_VisMode != 0;
			break;

		case ID_FIBERS_ORIGIN_LINK:
			if (bNeedUpdate = SelectVme(_("Choose a landmark cloud (or a landmark) that represent the origin area of muscle."),
				(long)&medVMEMuscleWrapper::VMEAcceptOIAreas, m_OIVME[0], m_OIVMEName[0]))
				m_Modified |= FIBERS_OPTIONS_MODIFIED;

			bNeedUpdate = bNeedUpdate && m_VisMode != 0;
			break;

		case ID_FIBERS_INSERTION_LINK:
			if (bNeedUpdate = SelectVme(_("Choose a landmark cloud (or a landmark) that represent the insertion area of muscle."),
				(long)&medVMEMuscleWrapper::VMEAcceptOIAreas, m_OIVME[1], m_OIVMEName[1]))
				m_Modified |= FIBERS_OPTIONS_MODIFIED;

			bNeedUpdate = bNeedUpdate && m_VisMode != 0;
			break;

		case ID_SPRING_LAYOUT_TYPE:
		case ID_CLOSEST_PARTICLE_NUMBER:
			m_Particles.empty();
			break;

			//case ID_PARTICLE_CONSTRAINT_THRESHOLD:
			//    if(!m_Particles.empty()) {
			//        GenerateParticleConstraints();
			//        if(m_FbDebugShowConstraints)
			//            UpdateChildrenParticleNode();
			//        bNeedUpdate = m_VisMode != 0;
			//    }
			//    break;

			//case ID_SPRING_LAYOUT_TYPE:
			//    if(!m_Particles.empty()) {
			//        GenerateParticleNeighbors();
			//        bNeedUpdate = m_VisMode != 0;
			//    }
			//    break;

			//case ID_CLOSEST_PARTICLE_NUMBER:
			//    if(m_PSpringLayoutType == 3 && !m_Particles.empty()) {
			//        GenerateParticleNeighbors();
			//        bNeedUpdate = m_VisMode != 0;
			//    }
			//    break;

		case ID_FIBERS_DEBUG_SHOW_CONSTRAINT_ONLY:
			UpdateChildrenParticleNode();
			bNeedUpdate = m_VisMode != 0;
			break;

		default:
			if (nId > ID_GENERATE_FIBERS && nId < ID_LAST)
			{
				bNeedUpdate = m_VisMode != 0;
				m_Modified |= FIBERS_OPTIONS_MODIFIED;

				if (nId == ID_FIBERS_SMOOTH)
					UpdateControls();

#ifdef _DEBUG_VIS_
				if (nId == ID_DEFORM_DEBUG_SHOWALL ||
					nId == ID_DEFORM_DEBUG_SHOWPROGRESS)
				{
					m_Modified |= DEFORMATION_OPTIONS_MODIFIED;
					bNeedUpdate = true;
				}
#endif
			}
#pragma endregion FIBRES OPTIONS CHANGES
			else
				bEventHandled = false;

			break;
		} //end switch

		if (bNeedUpdate)
		{
			this->Modified();
			GetOutput()->Update();

			//force redrawing
			mafEvent ev(this, VME_SELECTED,this);
			this->ForwardUpEvent(&ev);
			return;
		}

		if (bEventHandled)
			return;
	}

	//handle event called when any of descendant nodes or this one has been detached from the VME tree
	if (maf_event->GetId() == NODE_DETACHED_FROM_TREE)
	{
		medVMEMuscleWrapper* wrapper = medVMEMuscleWrapper::SafeDownCast(
			(mafObject*)maf_event->GetSender());
		if (wrapper != NULL)
		{
			//if we are the one to be destroyed  (otherwise the node being destructed will be notified later)
			if (wrapper == this)
			{
				m_Modified = EVERYTHING_MODIFIED;
				m_bLinksRestored = false;
				Modified();

				if (m_MasterVMEMuscleWrapper != NULL)
				{
					//we are the slave => detach ourselves from master list
					_ASSERTE(m_VMEMuscleWrappers.size() == 0);
					for (medVMEMusculoSkeletalModel::mafVMENodeList::iterator it = m_MasterVMEMuscleWrapper->m_VMEMuscleWrappers.begin();
						it != m_VMEMuscleWrappers.end(); it++)
					{
						wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
						if (wrapper->m_MasterVMEMuscleWrapper == this) {
							m_MasterVMEMuscleWrapper->m_VMEMuscleWrappers.erase(it);
							break;	//found it
						}
					}
					
					m_MasterVMEMuscleWrapper->Modified();
					m_MasterVMEMuscleWrapper = NULL;
				}
				else
				{
					//we are the master
					for (medVMEMusculoSkeletalModel::mafVMENodeList::const_iterator it = m_VMEMuscleWrappers.begin();
						it != m_VMEMuscleWrappers.end(); it++)
					{
						wrapper = medVMEMuscleWrapper::SafeDownCast(*it);
						if (wrapper->m_MasterVMEMuscleWrapper == this)
							wrapper->m_MasterVMEMuscleWrapper = NULL;	//we are no longer master

						wrapper->Modified();
					}

					//clear everything
					delete m_pParticleSystem;
					m_pParticleSystem = NULL;

					m_VMEMuscleWrappers.clear();
					m_pVMEMusculoSkeletalModel = NULL;
				}
			}
		}
	}

	Superclass::OnEvent(maf_event);
}

//------------------------------------------------------------------------
//Adds a new wrapper into the list of wrappers and GUI list
//pxP_RS denotes reference systems used for corresponding wrappers (optional).
void medVMEMuscleWrapper::AddWrapper(mafVME* pRP, mafVME* pCP,
	mafVME* pRP_RS, mafVME* pCP_RS)
	//------------------------------------------------------------------------
{
	CWrapperItem* pItem = new CWrapperItem();

	pItem->pVmeRP_CP[0] = pRP;
	pItem->pVmeRP_CP[1] = pCP;
	pItem->pVmeRefSys_RP_CP[0] = pRP_RS;
	pItem->pVmeRefSys_RP_CP[1] = pCP_RS;

	m_Wrappers.push_back(pItem);
	AddWrapper(pItem);
}

//------------------------------------------------------------------------
//Adds a new wrapper into the GUI list of wrappers
void medVMEMuscleWrapper::AddWrapper(CWrapperItem* pItem)
	//------------------------------------------------------------------------
{
	int nCount = m_WrappersCtrl->GetItemCount();

	wxString szName[4];
	for (int i = 0; i < 2; i++)
	{
		if (pItem->pVmeRP_CP[i] != NULL)
			szName[i] = pItem->pVmeRP_CP[i]->GetName();

		if (pItem->pVmeRefSys_RP_CP[i] != NULL)
			szName[2+i] = pItem->pVmeRefSys_RP_CP[i]->GetName();
	}

	m_WrappersCtrl->InsertItem(nCount, szName[0]);
	for (int i = 1; i <= 3; i++){
		m_WrappersCtrl->SetItem(nCount, i, szName[i]);
	}

	m_WrappersCtrl->SetItemState(nCount, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
		wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
	m_WrappersCtrl->EnsureVisible(nCount);

	//and save changes into Links
	StoreMeterLinks();
}

//------------------------------------------------------------------------
//Remove wrapper from the GUI and releases its memory
void medVMEMuscleWrapper::RemoveWrapper(int nIndex)
	//------------------------------------------------------------------------
{
	delete m_Wrappers[nIndex];
	m_Wrappers.erase(m_Wrappers.begin() + nIndex);

	m_WrappersCtrl->DeleteItem(nIndex);
	if (nIndex == m_WrappersCtrl->GetItemCount())
		nIndex--;

	if (nIndex >= 0)
	{
		m_WrappersCtrl->SetItemState(nIndex, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
			wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
		m_WrappersCtrl->EnsureVisible(nIndex);
	}

	//and save changes into Links
	StoreMeterLinks();
}

//------------------------------------------------------------------------
//Updates the visibility (etc) of GUI controls
void medVMEMuscleWrapper::UpdateControls()
	//------------------------------------------------------------------------
{
	m_BttnAddWrapper->Enable(m_WrappersVme[1] != NULL &&
		m_WrappersVme[0] != NULL);

	m_SmLabel1->Enable(m_FbSmooth != 0);
	m_SmLabel2->Enable(m_FbSmooth != 0);
	m_SmStepsCtrl->Enable(m_FbSmooth != 0);
	m_SmWeightCtrl->Enable(m_FbSmooth != 0);
}

//------------------------------------------------------------------------
//Shows dialog (with the message in title) where the user selects vme.
//The VME that can be selected are defined by accept_callback.
//If no VME is selected, the routine returns false, otherwise it returns
//reference to the VME, its name and updates GUI
bool medVMEMuscleWrapper::SelectVme(mafString title,
	long accept_callback, mafVME*& pOutVME, mafString& szOutVmeName)
	//------------------------------------------------------------------------
{
	mafEvent ev(this, VME_CHOOSE, accept_callback);
	ev.SetString(&title);
	ForwardUpEvent(ev);

	mafVME* vme = mafVME::SafeDownCast(ev.GetVme());
	if (vme == NULL)
		return false;

	szOutVmeName = (pOutVME = vme)->GetName();
	m_Gui->Update();
	return true;
}

#pragma region Accept VME Routines
//------------------------------------------------------------------------
/*static*/ bool medVMEMuscleWrapper::VMEAcceptMuscle(mafNode *node)
	//------------------------------------------------------------------------
{
	mafVME* vme = mafVME::SafeDownCast(node);
	if (vme != NULL)
	{
		if (vme->GetOutput()->IsA("mafVMEOutputSurface"))
			//|| vme->GetOutput()->IsA("mafVMEOutputMesh")
			return true;
	}

	return false;
}

//------------------------------------------------------------------------
/*static*/ bool medVMEMuscleWrapper::VMEAcceptWrapper(mafNode *node)
	//------------------------------------------------------------------------
{
	mafVME* vme = mafVME::SafeDownCast(node);
	if (vme != NULL)
	{
		if (
			vme->GetOutput()->IsA("mafVMEOutputMeter") ||
			vme->GetOutput()->IsA("medVMEOutputWrappedMeter") ||
			vme->GetOutput()->IsA("medVMEOutputComputeWrapping")  //TODO: why we cannot live with medVMEOutputWrappedMeter only?
			)
			return true;
	}

	return false;
}

//------------------------------------------------------------------------
/*static*/ bool medVMEMuscleWrapper::VMEAcceptOIAreas(mafNode *node)
	//------------------------------------------------------------------------
{
	mafVME* vme = mafVME::SafeDownCast(node);
	return vme != NULL &&
		(vme->IsA("mafVMELandmarkCloud") || vme->IsA("mafVMELandmark"));
}

//------------------------------------------------------------------------
/*static*/ bool medVMEMuscleWrapper::VMEAcceptRefSys(mafNode *node)
	//------------------------------------------------------------------------
{
	return mafVME::SafeDownCast(node) != NULL;
}
#pragma endregion Accept VME Routines
#pragma endregion GUI and Events Handling