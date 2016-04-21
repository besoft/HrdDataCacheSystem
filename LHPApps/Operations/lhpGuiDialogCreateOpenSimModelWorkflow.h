/*=========================================================================
Program:   iPose
Module:    $RCSfile: lhpGuiDialogCreateOpenSimModelWorkflow.h,v $
Language:  C++
Date:      $Date: 2012-01-27 15:46:22 $
Version:   $Revision: 1.1.2.21 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2011
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it
=========================================================================*/

#ifndef __lhpGuiDialogCreateOpenSimModelWorkflow_H__
#define __lhpGuiDialogCreateOpenSimModelWorkflow_H__

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMERoot;

using namespace std;

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafGUIDialog.h"
#include "mafGUI.h"
#include "mafGUIRollout.h"
#include "mafVMESurface.h"
#include "mafVMELandmarkCloud.h"

//----------------------------------------------------------------------------
// Const:
//----------------------------------------------------------------------------


/** lhpGuiDialogCreateOpenSimModelWorkflow

STEP_0:
STEP_1:
STEP_2:

*/

enum OPENSIM_CREATE_MODEL_WORKFLOW_IDs
{
	wxID_RETURN_NEXT = wxID_HIGHEST,
	wxID_RETURN_PREV,
};


class lhpGuiDialogCreateOpenSimModelWorkflow : public mafGUIDialog
{
public:

	/** Different dialog types that can be created */
	enum OSIM_COMPONENT_DIALOG_TYPE {CREATE_BODIES = 0 , CREATE_JOINTS , CREATE_MARKERS , CREATE_MUSCLES , WIZARD_FINISH};

	lhpGuiDialogCreateOpenSimModelWorkflow( const wxString& title, mafNode *input, int dialogType, mafObserver *listener = NULL );
	virtual ~lhpGuiDialogCreateOpenSimModelWorkflow(); 
	
	/** process events coming from other components */
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	wxString GetOpenSimAPIText() {return m_OpenSimAPIText;};

	/** */
	static wxString GetModelHeader();

	/** */
	static wxString GetModelFooter();

protected:

	/** Create the interface */
	void CreateGui();

	enum ID_GUIs
	{
		ID_CANCEL = MINID,
		ID_CONTINUE_BUTTON,
		ID_BACK_BUTTON,

		//////////////////////////////////////////////////////////////////////////
		// step 0: Create Bodies
		//////////////////////////////////////////////////////////////////////////
		ID_CHOOSE_VME_SURFACES_S0,
		ID_CHOOSE_VME_GROUPS_S0,
		ID_GENERATE_MODEL_S0,

		//////////////////////////////////////////////////////////////////////////
		// step 1: Create Joints
		//////////////////////////////////////////////////////////////////////////
		ID_CHOOSE_JOINT_TYPE_S1,

		// Free joint with ground
		ID_CHOOSE_BODY_CHILD_VME_SURFACE_FREE_JOINT_S1,

		// Pin Joint
		ID_CHOOSE_BODY_PARENT_VME_SURFACE_JOINT_S1,
		ID_CHOOSE_REF_SYS_VME_IN_PARENT_JOINT_S1,
		ID_CHOOSE_BODY_CHILD_VME_SURFACE_S1,
		ID_CHOOSE_REF_SYS_VME_IN_CHILD_JOINT_S1,
		ID_USE_GROUND_AS_PARENT_JOINT_S1,
		
		// Ball Joint
		ID_CHOOSE_REF_SYS_VME_IN_PARENT_BALL_JOINT_S1,


		ID_USE_GROUND_AS_PARENT_VME_BALL_JOINT_S1,
		ID_CHOOSE_BODY_PARENT_VME_SURFACE_BALL_JOINT_S1,
		ID_CHOOSE_BODY_CHILD_VME_SURFACE_BALL_JOINT_S1,
		ID_CHOOSE_REF_SYS_VME_IN_CHILD_BALL_JOINT_S1,

		// Custom Joint
		ID_USE_GROUND_AS_PARENT_VME_CUSTOM_JOINT_S1,
		ID_CHOOSE_BODY_PARENT_VME_SURFACE_CUSTOM_JOINT_S1,
		ID_CHOOSE_REF_SYS_VME_IN_PARENT_CUSTOM_JOINT_S1,
		ID_CHOOSE_BODY_CHILD_VME_SURFACE_CUSTOM_JOINT_S1,
		ID_CHOOSE_REF_SYS_VME_IN_CHILD_CUSTOM_JOINT_S1,

		// Create Joint
		ID_CLEAR_JOINTS_FROM_MODEL_S1,
		ID_ADD_JOINT_TO_MODEL_S1,

		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// step 3: Create Markers
		//////////////////////////////////////////////////////////////////////////

		ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD_S2,
		ID_CHOOSE_MARKER_SET_BODY_S2,
		ID_ADD_MARKER_SET_S2,

		//////////////////////////////////////////////////////////////////////////
		// step 4: Create Muscles
		//////////////////////////////////////////////////////////////////////////

		ID_ADD_MUSCLES,
		ID_HELP_BUTTON,
	};

	void CreateGuiBodiesStep0();

	void CreateGuiJointsStep1();

	void CreateGuiMarkersStep2();

	void CreateGuiMusclesStep3();

	void CreateGuiWizardFinish4();

	void AddSingleSurfaceBodiesToModelStep0();

	void AddMultiSurfaceBodiesToModelStep0();

	/////////////////////////////////////// 
	// joints 
	/////////////////////////////////////// 

		void OnCreateJointS1();

		void EnableFreeJointGui(bool enable);
		void OnChooseChildBodyVMEFreeJointS1();
		wxString CreateFreeJointWithGround();
		
		void EnableJointGui(bool enable);
		void OnUseGroundAsParentJointS1();
		void OnChooseParentBodyVMEJointS1();
		void OnChooseRefSysVMEInParentJointS1();
		void OnChooseChildBodyVMEJointS1();
		void OnChooseRefSysVMEInChildJointS1();
		
		
		void EnableBallJointGui(bool enable);
		void OnUseGroundAsParentBallJointS1();
		void OnChooseRefSysVMEInParentBallJointS1();
		void OnChooseRefSysVMEInChildBallJointS1();
		void OnChooseParentBodyVMEBallJointS1();
		void OnChooseChildBodyVMEBallJointS1();
		
		void EnableCustomJointGui(bool enable);
		void OnChooseRefSysVMEInParentCustomJointS1();
		void OnChooseRefSysVMEInChildCustomJointS1();
		void OnUseGroundAsParentCustomJointS1();
		void OnChooseParentBodyVMECustomJointS1();
		void OnChooseChildBodyVMECustomJointS1();
		
		void OnClearJointsFromModel();

		wxString CreatePinJoint();
		wxString CreateBallJoint();
		wxString CreateCustomJoint();

	/////////////////////////////////////// 

		void OnChooseMarkerSetLandmarkCloudS2();
	void OnChooseMarkerSetBodyS2();
	void OnAddMarkerSetBodyS2();
	
	void OnAddPointS3();
	void OnRemovePointS3();
	void OnChooseViaPointBodyS3();
	void OnChooseViaPointLandmarkS3();
	void OnChooseInsertionPointBodyS3();
	void OnChooseInsertionPointLandmarkS3();
	void OnAddMuscles();

	
private:

	/** Register the dialog type */
	int m_DialogType;

	mafGUI *m_LateralGui;

	mafNode *m_Input;

	int m_JointType;

	mafVME *m_ChildBodyVMEJointS1;

	mafVME *m_ParentBodyVMEJointS1;

	mafVME *m_RefSysVMEInParentJointS1;

	mafVME *m_RefSysVMEInChildJointS1;

	/** Use the ground as parent body*/
	int m_UseGroundAsParentJointS1;
	int m_UseGroundAsParentBallJointS1;
	int m_UseGroundAsParentCustomJointS1;

	mafVME *m_RefSysVMEInParentBallJointS1;
	mafVME *m_RefSysVMEInChildBallJointS1;

	mafVME *m_ParentBodyVMEBallJointS1;

	mafVME *m_ChildBodyVMEBallJointS1;

	mafVMELandmarkCloud *m_MarkerSetLandmarkCloud_S2;

	mafVME *m_MarkerSetBody_S2;

	wxString m_OpenSimAPIText;

	std::map<std::string , mafVMELandmark *> m_LMNameLMVmeMap_S3;
	std::map<std::string , mafVME *> m_LMNameBodyVmeMap_S3;

	wxListBox *m_ListBox;
	
	mafVME * m_RefSysVMEInParentCustomJointS1;
	mafVME * m_RefSysVMEInChildBallCustomS1;
	mafVME * m_ParentBodyVMECustomJointS1;
	mafVME * m_ChildBodyVMECustomJointS1;
};

#endif
