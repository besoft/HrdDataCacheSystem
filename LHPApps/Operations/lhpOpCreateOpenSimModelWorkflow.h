/*=========================================================================
Program:   iPose
Module:    $RCSfile: lhpOpCreateOpenSimModelWorkflow.h,v $
Language:  C++
Date:      $Date: 2012-03-21 07:29:03 $
Version:   $Revision: 1.1.2.11 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2011
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpOpCreateOpenSimModelWorkflow_H__
#define __lhpOpCreateOpenSimModelWorkflow_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include "lhpGuiDialogCreateOpenSimModelWorkflow.h"

enum ID_STEP {ID_STEP_CREATE_BODIES = 0, ID_STEP_CREATE_JOINTS , ID_STEP_CREATE_MARKERS , ID_STEP_CREATE_MUSCLES , ID_STEP_WIZARD_FINISH, ID_STEP_LAST};

//enum IPOSE_PLANNING_WORKFLOW_IDs
//{
//	wxID_RETURN_NEXT = wxID_HIGHEST,
//	wxID_RETURN_PREV,
//	wxID_RETURN_ADD_CASE,
//	wxID_RESTART_PLANNING,
//};

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafGUIDialog;

//----------------------------------------------------------------------------
// lhpOpCreateOpenSimModelWorkflow :
//----------------------------------------------------------------------------
/** Create an OpenSim model through a wizard */
class LHP_OPERATIONS_EXPORT lhpOpCreateOpenSimModelWorkflow: public mafOp
{
public:

	/** constructor */
	lhpOpCreateOpenSimModelWorkflow(wxString label = "lhpOpCreateOpenSimModelWorkflow");
  
	/** destructor */
	~lhpOpCreateOpenSimModelWorkflow(); 

	/** RTTI macro */
	mafTypeMacro(lhpOpCreateOpenSimModelWorkflow, mafOp);

	/** Answer to the messages coming from interface. */
	/*virtual*/ void OnEvent(mafEventBase *maf_event);

	/** Return a copy of the operation.*/
	mafOp* Copy();

	/** Return true for the acceptable vme type. */
	/*virtual*/ bool Accept(mafNode* vme);

	/** Builds operation's interface by calling CreateOpDialog() method. */
	/*virtual*/ void OpRun();

	/** Execute the operation. */
	/*virtual*/ void OpDo();

protected:

	enum GUI_IDs
	{
		ID_ADD_CASE = MINID,
		ID_EXISTING_CASE,
	};

 
	int ShowDialog(int dialogMode);

	/** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
	/*virtual*/ void OpStop(int result);

	void GenerateFullModel( mafNode *inputVme );

	int m_CurrentStep;

	wxString m_Step0_CreateBodies_Text;
	wxString m_Step1_CreateJoints_Text;
	wxString m_Step2_CreateMarkers_Text;
	wxString m_Step3_CreateMuscles_Text;
	wxString m_Step4_WizardEnd_Text;

};
#endif
