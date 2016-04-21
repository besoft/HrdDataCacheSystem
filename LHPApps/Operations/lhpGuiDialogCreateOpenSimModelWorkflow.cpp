/*=========================================================================
Program:   iPose
Module:    $RCSfile: lhpGuiDialogCreateOpenSimModelWorkflow.cpp,v $
Language:  C++
Date:      $Date: 2012-02-06 10:52:30 $
Version:   $Revision: 1.1.2.30 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2011
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

enum JOINT_TYPE {FREE_JOINT_WITH_GROUND = 0 , PIN_JOINT , BALL_JOINT, CUSTOM_JOINT};

#include "medDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpGuiDialogCreateOpenSimModelWorkflow.h"
#include "lhpOpModifyOpenSimModel.h"
#include "lhpOpModifyOpenSimModelCreateBodyFromSurface.h"

#include "mafGUIButton.h"
#include "mafGUIValidator.h"

#if _MSC_VER >= 1500
#define ssize_t VS2008_ssize_t_HACK 
#endif

#if _MSC_VER >= 1500
#undef VS2008_ssize_t_HACK
#endif

#include <string>
#include <algorithm>
#include "wx/filename.h"
#include <wx/dir.h>
#include "mafVMESurface.h"
#include "vtkPolyData.h"
#include "lhpOpCreateOpenSimModel.h"
#include "lhpOpModifyOpenSimModelCreateFreeJoint.h"
#include "lhpOpModifyOpenSimModelCreatePinJoint.h"
#include "lhpOpModifyOpenSimModelCreateBallJoint.h"
#include "mafVMEGroup.h"
#include "lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup.h"
#include "mafVMELandmarkCloud.h"
#include "lhpOpModifyOpenSimModelCreateMarkerSet.h"
#include "mafVMELandmark.h"
#include "lhpOpModifyOpenSimModelCreateMuscleMultiPoint.h"
#include "wx/busyinfo.h"
#include "lhpOpModifyOpenSimModelCreateMuscleFromMeter.h"
#include "medVMEComputeWrapping.H"
#include "lhpOpModifyOpenSimModelCreateCustomJoint.h"
#include "lhpOpCreateOpenSimModelWorkflow.h"

//----------------------------------------------------------------------------
lhpGuiDialogCreateOpenSimModelWorkflow::lhpGuiDialogCreateOpenSimModelWorkflow( const wxString& title, mafNode *input, int dialogType, mafObserver *listener /*= NULL */ ) : mafGUIDialog(title,mafCLOSEWINDOW | mafRESIZABLE)
	//----------------------------------------------------------------------------
{
	m_OpenSimAPIText = "";

	m_Input = input;
	m_DialogType = dialogType;

	m_JointType = FREE_JOINT_WITH_GROUND;

	m_ChildBodyVMEJointS1 = NULL;

	m_UseGroundAsParentJointS1 = 0;
	m_UseGroundAsParentBallJointS1 = 0;
	m_UseGroundAsParentCustomJointS1 = 0;

	m_RefSysVMEInParentBallJointS1 = NULL;
	m_ParentBodyVMEBallJointS1 = NULL;
	m_ChildBodyVMEBallJointS1 = NULL;

	m_Listener = listener;

	int x_pos,y_pos,w,h;
	mafGetFrame()->GetPosition(&x_pos,&y_pos);
	this->GetSize(&w,&h);
	this->SetSize(x_pos+5,y_pos+5,w,h);
	CreateGui();

	m_MarkerSetLandmarkCloud_S2 = NULL;

	m_MarkerSetBody_S2 = NULL;

	m_RefSysVMEInParentCustomJointS1 = NULL;	
	m_RefSysVMEInChildBallCustomS1 = NULL;

}
//----------------------------------------------------------------------------
lhpGuiDialogCreateOpenSimModelWorkflow::~lhpGuiDialogCreateOpenSimModelWorkflow()
	//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
void lhpGuiDialogCreateOpenSimModelWorkflow::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{

		case ID_HELP_BUTTON:
			{
				mafEvent helpEvent;
				helpEvent.SetSender(this);
				mafString operationLabel = (static_cast<lhpOpCreateOpenSimModelWorkflow *>(this->m_Listener))->m_Label;
				helpEvent.SetString(&operationLabel);
				helpEvent.SetId(OPEN_HELP_PAGE);
				mafEventMacro(helpEvent);
			}
			break;

		case ID_CHOOSE_VME_SURFACES_S0:
			{
				AddSingleSurfaceBodiesToModelStep0();
			}
			break;

		case ID_CHOOSE_VME_GROUPS_S0:
			{
				AddMultiSurfaceBodiesToModelStep0();
			}
			break;


		case ID_GENERATE_MODEL_S0:
			{
				//GenerateFullModel(m_Input);
			}
			break;


		case ID_CHOOSE_JOINT_TYPE_S1:
			{
	
				if (m_JointType == FREE_JOINT_WITH_GROUND)
				{
					EnableFreeJointGui(true);
				}
				else if (m_JointType == PIN_JOINT || m_JointType == BALL_JOINT || m_JointType == CUSTOM_JOINT )
				{
					EnableJointGui(true);
				}
				
				m_LateralGui->Enable(ID_ADD_JOINT_TO_MODEL_S1 , false);
			}

			break;



		//////////////////////////////////////////////////////////////////////////
		// FREE_JOINT_S1
		//////////////////////////////////////////////////////////////////////////
		case ID_CHOOSE_BODY_CHILD_VME_SURFACE_FREE_JOINT_S1:
			{
				OnChooseChildBodyVMEFreeJointS1();
			}
			break;

		//////////////////////////////////////////////////////////////////////////
		// JOINT_S1
		//////////////////////////////////////////////////////////////////////////

		case ID_USE_GROUND_AS_PARENT_JOINT_S1:
			{
				OnUseGroundAsParentJointS1();
			}
			break;

		
		case ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1:
			{
				OnChooseParentBodyVMEJointS1();
			}
			break;

		case ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1:
			{
				OnChooseRefSysVMEInParentJointS1();
			}
			break;

		case ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1:
			{
				OnChooseChildBodyVMEJointS1();
			}
			break;

		case ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1:
			{
				OnChooseRefSysVMEInChildJointS1();
			}
			break;

		case ID_CLEAR_JOINTS_FROM_MODEL_S1:
			{
				OnClearJointsFromModel();
			}
			break;

		case ID_ADD_JOINT_TO_MODEL_S1:
			{
			//	wxMessageBox("Create joint");

				OnCreateJointS1();
			}
			break;

		//////////////////////////////////////////////////////////////////////////
		// MARKER_SET_S2
		//////////////////////////////////////////////////////////////////////////

		case ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD_S2:
			{
				OnChooseMarkerSetLandmarkCloudS2();
			}
			break;

		case ID_CHOOSE_MARKER_SET_BODY_S2:
			{
				OnChooseMarkerSetBodyS2();
			}
			break;
	
		case ID_ADD_MARKER_SET_S2:
			{
				OnAddMarkerSetBodyS2();
			}
			break;

		//////////////////////////////////////////////////////////////////////////
		// Create Muscles
		//////////////////////////////////////////////////////////////////////////

		case ID_ADD_MUSCLES:
			{
				OnAddMuscles();
			}
			break;

		case ID_CANCEL:
			{
				this->EndModal(wxCANCEL);
			}
			break;

		case ID_CONTINUE_BUTTON:
			{
				this->EndModal(wxID_RETURN_NEXT);
				break;
			}

		case ID_BACK_BUTTON:
			{
				this->EndModal(wxID_RETURN_PREV);
				break;
			}

		default:
			mafEventMacro(*e);
		}
	}
}

//----------------------------------------------------------------------------
void lhpGuiDialogCreateOpenSimModelWorkflow::CreateGui()
//----------------------------------------------------------------------------
{
	wxPoint p = wxDefaultPosition;

	m_LateralGui = new mafGUI(this);
	m_LateralGui->Reparent(this);

	if (m_DialogType == CREATE_BODIES)
	{
		CreateGuiBodiesStep0();
	}

	if (m_DialogType == CREATE_JOINTS)
	{
		CreateGuiJointsStep1();
	}

	if (m_DialogType == CREATE_MARKERS)
	{
		CreateGuiMarkersStep2();
	}

	if (m_DialogType == CREATE_MUSCLES)
	{
		CreateGuiMusclesStep3();
	}

	if (m_DialogType == WIZARD_FINISH)
	{
		CreateGuiWizardFinish4();
	}

	m_LateralGui->FitGui();
	m_LateralGui->Update();

	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	mafGUIButton *b_accept = new mafGUIButton(this, ID_CONTINUE_BUTTON,_("continue"), p, wxSize(100,20));
	b_accept->SetValidator(mafGUIValidator(this,ID_CONTINUE_BUTTON,b_accept));
	mafGUIButton *b_cancel = new mafGUIButton(this, ID_CANCEL,_("cancel"), p, wxSize(100,20));
	b_cancel->SetValidator(mafGUIValidator(this,ID_CANCEL,b_cancel));
	mafGUIButton *b_reject = new mafGUIButton(this, ID_BACK_BUTTON,_("back"), p, wxSize(100,20));
	b_reject->SetValidator(mafGUIValidator(this,ID_BACK_BUTTON,b_reject));

	topSizer->Add(m_LateralGui,0);

	buttonSizer->Add(b_reject,0);
	buttonSizer->Add(b_cancel,0);
	buttonSizer->Add(b_accept,0);

	m_GuiSizer->Add(topSizer,0,wxCENTRE,5);
	m_GuiSizer->Add(buttonSizer,0,wxCENTRE,5);


}		

void lhpGuiDialogCreateOpenSimModelWorkflow::CreateGuiBodiesStep0()
{

	mafEvent buildHelpGui;
	buildHelpGui.SetSender(this);
	buildHelpGui.SetId(GET_BUILD_HELP_GUI);
	mafEventMacro(buildHelpGui);

	m_LateralGui->Label("");

	if (buildHelpGui.GetArg() == true)
	{
		m_LateralGui->Button(ID_HELP_BUTTON, "Help","");	
	}

	this->SetTitle("Create Bodies From Surfaces");
	m_LateralGui->Label("");
	m_LateralGui->Label("Add OpenSim bodies to the model");
	m_LateralGui->Label("from VMEs with inertial");
	m_LateralGui->Label("properties");
	m_LateralGui->Label("");
	m_LateralGui->Label("Single surface bodies");
	m_LateralGui->Button(ID_CHOOSE_VME_SURFACES_S0, "Add Surfaces");
	m_LateralGui->Label("");
	m_LateralGui->Label("Multi surface bodies");
	m_LateralGui->Button(ID_CHOOSE_VME_GROUPS_S0, "Add Groups");

	// m_LateralGui->Button(ID_GENERATE_MODEL_S0, "Test Gen Model");
	m_LateralGui->Label("");
}

void lhpGuiDialogCreateOpenSimModelWorkflow::CreateGuiJointsStep1()
{
	this->SetTitle("Create Joints");
	m_LateralGui->Label("");
	m_LateralGui->Label("Choose the joint to add to the");
	m_LateralGui->Label("model then press Add Joint.");
	m_LateralGui->Label("A custom joint specializing the");
	m_LateralGui->Label("selected joint will be created.");
	m_LateralGui->Label("You can add as many joints as");
	m_LateralGui->Label("you like: press continue when you");
	m_LateralGui->Label("are done");
	const wxString choices[] = {"Free joint with ground", "Pin joint" , "Ball joint" , "Custom Joint"};
	wxComboBox *cb = m_LateralGui->Combo(ID_CHOOSE_JOINT_TYPE_S1, "", &m_JointType, 4, choices);
	
	m_LateralGui->Label("");
	m_LateralGui->Label("Use ground as parent");
	m_LateralGui->Bool(ID_USE_GROUND_AS_PARENT_JOINT_S1, "", &m_UseGroundAsParentJointS1);
	m_LateralGui->Enable(ID_USE_GROUND_AS_PARENT_JOINT_S1, false);
	m_LateralGui->Button(ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1, "Parent Body");
	m_LateralGui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1, false);
	m_LateralGui->Button(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1, "RefSys in Parent");
	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1, false);
	m_LateralGui->Button(ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1, "Child Body");
	m_LateralGui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1, false);
	m_LateralGui->Button(ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1, "RefSys in Child");
	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1, false);
	

	cb->Select(FREE_JOINT_WITH_GROUND);
	EnableFreeJointGui(true);

	//////////////////////////////////////////////////////////////////////////
	m_LateralGui->Label("");
	//m_LateralGui->Button(ID_CLEAR_JOINTS_FROM_MODEL_S1, "Add Joint");
	//m_LateralGui->Enable(ID_CLEAR_JOINTS_FROM_MODEL_S1, false);
	//m_LateralGui->Label("");
	m_LateralGui->Button(ID_ADD_JOINT_TO_MODEL_S1, "Add Joint");
	m_LateralGui->Enable(ID_ADD_JOINT_TO_MODEL_S1, false);
	m_LateralGui->Label("");

	//	mafDEL(fj);

}

void lhpGuiDialogCreateOpenSimModelWorkflow::CreateGuiMarkersStep2()
{
	this->SetTitle("Create Marker Sets");
	m_LateralGui->Label("");
	
	m_LateralGui->Label("Choose source landmark cloud for");
	m_LateralGui->Label("markers generation");
	
	m_LateralGui->Button(ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD_S2 , "Source Landmark cloud");
	m_LateralGui->Enable(ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD_S2, true);
	m_LateralGui->Label("");
	m_LateralGui->Label("Choose target body for markers");
	m_LateralGui->Button(ID_CHOOSE_MARKER_SET_BODY_S2 , "Target body");
	m_LateralGui->Enable(ID_CHOOSE_MARKER_SET_BODY_S2, false);
	m_LateralGui->Label("");
	m_LateralGui->Button(ID_ADD_MARKER_SET_S2, "Add Marker Set");
	m_LateralGui->Enable(ID_ADD_MARKER_SET_S2, false);
	m_LateralGui->Label("");
}

void lhpGuiDialogCreateOpenSimModelWorkflow::CreateGuiWizardFinish4()
{
	this->SetTitle("Create Muscles");
	this->SetTitle("Wizard Done!");
	m_LateralGui->Label("");
	m_LateralGui->Label("The wizard has finished.");
	m_LateralGui->Label("Press accept to add the model");
	m_LateralGui->Label("to the MSF tree ");
	m_LateralGui->Label("");
	m_LateralGui->Label("Generated model API will");
	m_LateralGui->Label("open in model editor");
	m_LateralGui->Label("");
}


void lhpGuiDialogCreateOpenSimModelWorkflow::AddSingleSurfaceBodiesToModelStep0()
{
	mafString title = "Choose the bodies to add to model";

	mafEvent ev(this, VME_CHOOSE);   
	ev.SetString(&title);
	ev.SetBool(true);
	ev.SetArg((long)&lhpOpModifyOpenSimModel::VMESurfaceAcceptForBodyGeneration);

	mafEventMacro(ev);

	std::vector< mafNode* > selectedVMEs;

	selectedVMEs = ev.GetVmeVector();

	std::ostringstream stringStream;
	
	int nCount = (int)selectedVMEs.size();

	mafVMESurface* currentVme = NULL;

	wxString bodiesModelText = "";

	for (int i = 0; i < nCount; i++)
	{
		wxBusyInfo wait("Please Wait...");	

		mafVMESurface* vme = mafVMESurface::SafeDownCast(selectedVMEs[i]);
		stringStream << "Choosed vme " << i << " name : " << vme->GetName() << std::endl;	

		assert(vme != NULL);		

		mafLogMessage(stringStream.str().c_str());

		vme = mafVMESurface::SafeDownCast(selectedVMEs[i]);

		lhpOpModifyOpenSimModelCreateBodyFromSurface *op = new lhpOpModifyOpenSimModelCreateBodyFromSurface();
		op->SetListener(this);
		op->SetInputVMEForComponentAPIGeneration(vme);
		op->GenerateOpenSimComponentAPI();
		wxString outputText = op->GetOpenSimComponentTextToBePasted();
		mafDEL(op);

		bodiesModelText.Append(outputText);
		bodiesModelText.append("");

	}

	
	if (bodiesModelText != "")
	{
		m_OpenSimAPIText.append(bodiesModelText);
		wxMessageBox("Bodies added to model");
	}
	
	
	// wxMessageBox(bodiesModelText);
}

wxString lhpGuiDialogCreateOpenSimModelWorkflow::GetModelHeader()
{
	wxString modelHeaderAbsFileName = lhpOpModifyOpenSimModel::GetOpenSimAPITemplatesDir();	
	modelHeaderAbsFileName = modelHeaderAbsFileName + "templateCreateModelHeader.cpp";
	assert(wxFileExists(modelHeaderAbsFileName));

	wxString modelHeaderText = lhpOpModifyOpenSimModel::FileToString(modelHeaderAbsFileName);

	return modelHeaderText;
}

wxString lhpGuiDialogCreateOpenSimModelWorkflow::GetModelFooter()
{
	wxString modelFooterAbsFileName = lhpOpModifyOpenSimModel::GetOpenSimAPITemplatesDir();	
	modelFooterAbsFileName = modelFooterAbsFileName + "templateCreateModelFooter.cpp";
	assert(wxFileExists(modelFooterAbsFileName));

	wxString modelFooterText = lhpOpModifyOpenSimModel::FileToString(modelFooterAbsFileName);
	
	return modelFooterText;
}


void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseChildBodyVMEFreeJointS1()
{	
	mafString title = mafString("Select child VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ChildBodyVMEJointS1 = vme;

	m_LateralGui->Enable(ID_ADD_JOINT_TO_MODEL_S1, true);
	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnCreateJointS1()
{

	// lhpOpModifyOpenSimModelCreateFreeJoint(WithGround)
	// lhpOpModifyOpenSimModelCreatePinJoint
	// lhpOpModifyOpenSimModelCreateBallJoint
	// lhpOpModifyOpenSimModelCreateCustomJoint
	
	wxString apiText;

	if (m_JointType == FREE_JOINT_WITH_GROUND)
	{
		apiText = CreateFreeJointWithGround();
		EnableFreeJointGui(true);
	}
	else if (m_JointType == PIN_JOINT)
	{
		apiText = CreatePinJoint();
		EnableJointGui(true);
	}
	else if (m_JointType == BALL_JOINT)
	{
		apiText = CreateBallJoint();
		EnableJointGui(true);
	}
	else if (m_JointType == CUSTOM_JOINT)
	{
		apiText = CreateCustomJoint();
		EnableJointGui(true);
	}

	m_LateralGui->Enable(ID_ADD_JOINT_TO_MODEL_S1 , false);
	m_LateralGui->Update();

	m_OpenSimAPIText.append(apiText);
}

//////////////////////////////////////////////////////////////////////////
//  Free Joint with Ground
//////////////////////////////////////////////////////////////////////////

void lhpGuiDialogCreateOpenSimModelWorkflow::EnableFreeJointGui(bool enable)
{
	assert(m_LateralGui);
	m_LateralGui->Enable(ID_USE_GROUND_AS_PARENT_JOINT_S1, false);
	m_LateralGui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1, false);
	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1, false);
	m_LateralGui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1, enable);
	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1, false);
	m_UseGroundAsParentJointS1 = 0;
	m_LateralGui->Update();

}


wxString lhpGuiDialogCreateOpenSimModelWorkflow::CreateFreeJointWithGround()
{
	wxString freeJointText;

	lhpOpModifyOpenSimModelCreateFreeJoint *op = new lhpOpModifyOpenSimModelCreateFreeJoint();
	op->SetListener(this);

	assert(m_ChildBodyVMEJointS1);
	op->SetChildBodyVME(m_ChildBodyVMEJointS1);

	op->GenerateOpenSimComponentAPI();

	wxString outputText = op->GetOpenSimComponentTextToBePasted();
	mafDEL(op);

	freeJointText.Append(outputText);
	freeJointText.append("");		

	// wxMessageBox(freeJointText);
	
	wxMessageBox("Free Joint added to model");

	return freeJointText;
}

//////////////////////////////////////////////////////////////////////////
//  Pin Joint
//////////////////////////////////////////////////////////////////////////

void lhpGuiDialogCreateOpenSimModelWorkflow::EnableJointGui(bool enable)
{
	assert(m_LateralGui);
	m_LateralGui->Enable(ID_USE_GROUND_AS_PARENT_JOINT_S1, enable);
	m_LateralGui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1, enable);
	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1, false);
	m_LateralGui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1, false);
	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1, false);
	m_UseGroundAsParentJointS1 = 0;
	m_LateralGui->Update();
}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseRefSysVMEInParentJointS1()
{
	mafString title = mafString("VME Ref Sys:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_RefSysVMEInParentJointS1 = vme;

	m_LateralGui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1 , true);
	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseRefSysVMEInChildJointS1()
{
	mafString title = mafString("VME Ref Sys:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_RefSysVMEInChildJointS1 = vme;

	m_LateralGui->Enable(ID_ADD_JOINT_TO_MODEL_S1 , true);
	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnUseGroundAsParentJointS1()
{
	if (m_UseGroundAsParentJointS1 == true)
	{
		m_LateralGui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1 , false);
		m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1 , true);
	}
	else if (m_UseGroundAsParentJointS1 == false)
	{
		m_LateralGui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1 , true);
		m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1 , false);
	}

	m_LateralGui->Update();
}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseParentBodyVMEJointS1()
{
	mafString title = mafString("Select parent VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ParentBodyVMEJointS1= vme;

	m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1 , true);
	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseChildBodyVMEJointS1()
{
	mafString title = mafString("Select child VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ChildBodyVMEJointS1 = vme;

	if (m_JointType == FREE_JOINT_WITH_GROUND)
	{
		m_LateralGui->Enable(ID_ADD_JOINT_TO_MODEL_S1, true);
	}
	else
	{
		m_LateralGui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1, true);
	}
	
	m_LateralGui->Update();

}

wxString lhpGuiDialogCreateOpenSimModelWorkflow::CreatePinJoint()
{
	wxBusyCursor wait_cursor;      
	wxBusyInfo wait("Please Wait...");	

	wxString pinJointText;

	lhpOpModifyOpenSimModelCreatePinJoint *op = new lhpOpModifyOpenSimModelCreatePinJoint();
	op->SetListener(this);

	op->SetUseGroundAsParent(m_UseGroundAsParentJointS1);
	op->SetRefSysVMEInParent(m_RefSysVMEInParentJointS1);

	if (m_UseGroundAsParentJointS1 == false)
	{
		op->SetParentBodyVME(m_ParentBodyVMEJointS1);
	}

	op->SetChildBodyVME(m_ChildBodyVMEJointS1);
	op->SetRefSysVMEInChild(m_RefSysVMEInChildJointS1);

	op->GenerateOpenSimComponentAPI();

	wxString outputText = op->GetOpenSimComponentTextToBePasted();
	mafDEL(op);

	pinJointText.Append(outputText);
	pinJointText.append("");		

	// wxMessageBox(pinJointText);

	wxMessageBox("Pin Joint added to model");

	return pinJointText;

}

wxString lhpGuiDialogCreateOpenSimModelWorkflow::CreateBallJoint()
{

	wxString ballJointText;

	lhpOpModifyOpenSimModelCreateBallJoint *op = new lhpOpModifyOpenSimModelCreateBallJoint();
	op->SetListener(this);

	op->SetUseGroundAsParent(m_UseGroundAsParentJointS1);
	op->SetRefSysVMEInParent(m_RefSysVMEInParentJointS1);
	if (m_UseGroundAsParentJointS1 == false)
	{
		op->SetParentBodyVME(m_ParentBodyVMEJointS1);
	}

	op->SetChildBodyVME(m_ChildBodyVMEJointS1);
	op->SetRefSysVMEInChild(m_RefSysVMEInChildJointS1);

	op->GenerateOpenSimComponentAPI();

	wxString outputText = op->GetOpenSimComponentTextToBePasted();
	mafDEL(op);

	ballJointText.Append(outputText);
	ballJointText.append("");		

	// wxMessageBox(pinJointText);

	wxMessageBox("Ball Joint added to model");

	return ballJointText;
}

wxString lhpGuiDialogCreateOpenSimModelWorkflow::CreateCustomJoint()
{

	wxString ballJointText;

	lhpOpModifyOpenSimModelCreateCustomJoint *op = new lhpOpModifyOpenSimModelCreateCustomJoint();
	op->SetListener(this);

	op->SetUseGroundAsParent(m_UseGroundAsParentJointS1);
	op->SetRefSysVMEInParent(m_RefSysVMEInParentJointS1);
	
	if (m_UseGroundAsParentJointS1 == false)
	{
		op->SetParentBodyVME(m_ParentBodyVMEJointS1);
	}

	op->SetChildBodyVME(m_ChildBodyVMEJointS1);
	op->SetRefSysVMEInChild(m_RefSysVMEInChildJointS1);

	op->GenerateOpenSimComponentAPI();

	wxString outputText = op->GetOpenSimComponentTextToBePasted();
	mafDEL(op);

	ballJointText.Append(outputText);
	ballJointText.append("");		

	// wxMessageBox(pinJointText);

	wxMessageBox("Custom Joint added to model");

	return ballJointText;
}

//////////////////////////////////////////////////////////////////////////

void lhpGuiDialogCreateOpenSimModelWorkflow::OnClearJointsFromModel()
{	
	wxMessageBox("Model joints removed");
}

void lhpGuiDialogCreateOpenSimModelWorkflow::AddMultiSurfaceBodiesToModelStep0()
{

	mafString title = "Choose the multibodies to add to model";

	mafEvent ev(this, VME_CHOOSE);   
	ev.SetString(&title);
	ev.SetBool(true);
	ev.SetArg((long)&lhpOpModifyOpenSimModel::VMEGroupAcceptForBodyGeneration);

	mafEventMacro(ev);

	std::vector< mafNode* > selectedVMEs;

	selectedVMEs = ev.GetVmeVector();

	std::ostringstream stringStream;

	int nCount = (int)selectedVMEs.size();

	mafVMEGroup* currentVme = NULL;

	wxString bodiesModelText = "";



	for (int i = 0; i < nCount; i++)
	{
		wxBusyInfo wait("Please Wait...");	

		mafVMEGroup* vme = mafVMEGroup::SafeDownCast(selectedVMEs[i]);
		stringStream << "Choosed vme " << i << " name : " << vme->GetName() << std::endl;	

		assert(vme != NULL);		

		mafLogMessage(stringStream.str().c_str());

		vme = mafVMEGroup::SafeDownCast(selectedVMEs[i]);
		
		lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup *op = new lhpOpModifyOpenSimModelCreateBodyFromSurfacesGroup();
		op->SetListener(this);
		op->SetInputVMEForComponentAPIGeneration(vme);
		op->GenerateOpenSimComponentAPI();
		wxString outputText = op->GetOpenSimComponentTextToBePasted();
		mafDEL(op);

		bodiesModelText.Append(outputText);
		bodiesModelText.append("");

	}

	if (bodiesModelText != "")
	{
		m_OpenSimAPIText.append(bodiesModelText);
		wxMessageBox("Bodies added to model");
	}

//////////////////////////////////////////////////////////////////////////
	
	// wxMessageBox(bodiesModelText);
}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseMarkerSetLandmarkCloudS2()
{
	mafString title = mafString("Choose Marker Set Landmark Cloud:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMELandmarkCloudAcccept);
	mafEventMacro(e);

	mafVMELandmarkCloud *vme = mafVMELandmarkCloud::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_MarkerSetLandmarkCloud_S2 = vme;

	m_LateralGui->Enable(ID_CHOOSE_MARKER_SET_BODY_S2, true);
	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnChooseMarkerSetBodyS2()
{
	mafString title = mafString("Choose Marker Set Body:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_MarkerSetBody_S2 = vme;

	m_LateralGui->Enable(ID_ADD_MARKER_SET_S2, true);
	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnAddMarkerSetBodyS2()
{
	assert(m_MarkerSetLandmarkCloud_S2);
	assert(m_MarkerSetBody_S2);

	lhpOpModifyOpenSimModelCreateMarkerSet *op = new lhpOpModifyOpenSimModelCreateMarkerSet();
	op->SetListener(this);
	op->SetMarkerSetLandmarkCloud(m_MarkerSetLandmarkCloud_S2);
	op->SetMarkerSetBody(m_MarkerSetBody_S2);
	op->GenerateOpenSimComponentAPI();
	wxString outputText = op->GetOpenSimComponentTextToBePasted();
	mafDEL(op);

	if (outputText != "")
	{
		m_OpenSimAPIText.append(outputText);
		wxMessageBox("Markers added to model");
	}

	m_LateralGui->Enable(ID_CHOOSE_MARKER_SET_BODY_S2, false);
	m_LateralGui->Enable(ID_ADD_MARKER_SET_S2, false);

	m_LateralGui->Update();

}

void lhpGuiDialogCreateOpenSimModelWorkflow::CreateGuiMusclesStep3()
{
	m_LateralGui->SetListener(this);

	this->SetTitle("Add Muscles");
	m_LateralGui->Label("");
	m_LateralGui->Label("Select muscles from vme tree");
	m_LateralGui->Label("");
	m_LateralGui->Label("The following tags will be read");
	m_LateralGui->Label("from the action line if available");
	m_LateralGui->Label("otherwise default values will");
	m_LateralGui->Label("be used:");
	m_LateralGui->Label("maxIsometricForce");
	m_LateralGui->Label("optimalFiberLength");
	m_LateralGui->Label("tendonSlackLength");
	m_LateralGui->Label("");


	m_LateralGui->Label("");
	m_LateralGui->Button(ID_ADD_MUSCLES, "Add Muscles");
	m_LateralGui->Enable(ID_ADD_MUSCLES, true);
	m_LateralGui->Label("");

}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnAddPointS3()
{
	mafString title = _("Choose a vme landmark");
	mafEvent eventChooseLandmark(this,VME_CHOOSE);
	eventChooseLandmark.SetString(&title);
	eventChooseLandmark.SetArg((long)&lhpOpModifyOpenSimModel::VMELandmarkAccept);
	mafEventMacro(eventChooseLandmark);

	mafVMELandmark *lmVme = mafVMELandmark::SafeDownCast(eventChooseLandmark.GetVme());	

	if (lmVme == NULL) return;

	wxString nameProfile ="";

	nameProfile = lmVme->GetName();

	if(wxNOT_FOUND != m_ListBox->FindString(lmVme->GetName()))
	{
		wxMessageBox(_("Can't introduce vme with the same name"));
		return;
	}

	title = mafString("Choose corresponding body:");
	mafEvent eventChooseBody(this,VME_CHOOSE);
	eventChooseBody.SetString(&title);
	eventChooseBody.SetArg((long)&lhpOpModifyOpenSimModel::VMEAcceptAsBody);
	mafEventMacro(eventChooseBody);
	mafVME *bodyVme = mafVME::SafeDownCast(eventChooseBody.GetVme());	

	if (bodyVme == NULL)
	{
		return;
	}

	m_LateralGui->Enable(ID_ADD_MUSCLES, true);

	mafString t;
	t = lmVme->GetName();

	m_ListBox->Append(_(t));
	m_ListBox->SetStringSelection(_(t));

	m_LMNameLMVmeMap_S3[lmVme->GetName()] = lmVme;
	m_LMNameBodyVmeMap_S3[lmVme->GetName()] = bodyVme;

	m_ListBox->Update();
	m_LateralGui->Update();
	
}

void lhpGuiDialogCreateOpenSimModelWorkflow::OnRemovePointS3()
{
	if(m_ListBox->GetCount()!=0)
	{
		wxString name = m_ListBox->GetStringSelection();
		int number = m_ListBox->GetSelection();

		m_LMNameLMVmeMap_S3.erase(name.c_str());
		m_LMNameBodyVmeMap_S3.erase(name.c_str());

		m_ListBox->Delete(m_ListBox->FindString(m_ListBox->GetStringSelection()));          

	}
}


void lhpGuiDialogCreateOpenSimModelWorkflow::OnAddMuscles()
{
	
	//////////////////////////////////////////////////////////////////////////////////

	mafString title = "Choose the muscles to add to model";

	mafEvent ev(this, VME_CHOOSE);   
	ev.SetString(&title);
	ev.SetBool(true);
	ev.SetArg((long)&lhpOpModifyOpenSimModel::VMEMeterAccept);

	mafEventMacro(ev);

	std::vector< mafNode* > selectedVMEs;

	selectedVMEs = ev.GetVmeVector();

	std::ostringstream stringStream;

	int nCount = (int)selectedVMEs.size();

	mafVMEGroup* currentVme = NULL;

	wxString musclesText = "";

	for (int i = 0; i < nCount; i++)
	{
		wxBusyInfo wait("Please Wait...");	

		medVMEComputeWrapping* vme = medVMEComputeWrapping::SafeDownCast(selectedVMEs[i]);
		stringStream << "Choosed vme " << i << " name : " << vme->GetName() << std::endl;	

		assert(vme != NULL);		

		mafLogMessage(stringStream.str().c_str());

		vme = medVMEComputeWrapping::SafeDownCast(selectedVMEs[i]);

		lhpOpModifyOpenSimModelCreateMuscleFromMeter *op = new lhpOpModifyOpenSimModelCreateMuscleFromMeter();
		op->SetListener(this);
		op->SetMeter(vme);

		op->GenerateOpenSimComponentAPI();
		wxString outputText = op->GetOpenSimComponentTextToBePasted();
		mafDEL(op);

		musclesText.Append(outputText);
		musclesText.append("");

	}


	if (musclesText != "")
	{
		m_OpenSimAPIText.append(musclesText);
		wxMessageBox("Muscles added to model");
	}

	// wxMessageBox(bodiesModelText);

}
