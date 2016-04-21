/*=========================================================================
Program:   iPose
Module:    $RCSfile: lhpOpCreateOpenSimModelWorkflow.cpp,v $
Language:  C++
Date:      $Date: 2012-03-21 07:29:03 $
Version:   $Revision: 1.1.2.21 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2011
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/


#include "medDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpCreateOpenSimModelWorkflow.h"

#include "mafNode.h"
#include "mafGUIDialog.h"
#include "mafVMERoot.h"

#include "vtkMAFSmartPointer.h"
#include "lhpOpCreateOpenSimModel.h"
#include "lhpGuiDialogCreateOpenSimModelWorkflow.h"
#include "lhpOpModifyOpenSimModel.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpCreateOpenSimModelWorkflow);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
lhpOpCreateOpenSimModelWorkflow::lhpOpCreateOpenSimModelWorkflow(wxString label) :
mafOp(label)
	//----------------------------------------------------------------------------
{

	m_OpType  = OPTYPE_OP;
	m_Canundo = true;
	m_InputPreserving = true;
	m_CurrentStep = lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_BODIES;

	m_Step0_CreateBodies_Text = "";
	m_Step1_CreateJoints_Text = "";
	m_Step2_CreateMarkers_Text = "";
	m_Step3_CreateMuscles_Text = "";
	m_Step4_WizardEnd_Text = "";
}
//----------------------------------------------------------------------------
lhpOpCreateOpenSimModelWorkflow::~lhpOpCreateOpenSimModelWorkflow()
	//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
mafOp* lhpOpCreateOpenSimModelWorkflow::Copy()
	//----------------------------------------------------------------------------
{
	/** return a copy of itself, needs to put it into the undo stack */
	return new lhpOpCreateOpenSimModelWorkflow(m_Label);
}
//----------------------------------------------------------------------------
bool lhpOpCreateOpenSimModelWorkflow::Accept(mafNode* vme)
	//----------------------------------------------------------------------------
{

	return vme != NULL;
		//&& mafVMERoot::SafeDownCast(vme);
}

//----------------------------------------------------------------------------
void lhpOpCreateOpenSimModelWorkflow::OpRun()
	//----------------------------------------------------------------------------
{
	int result = 0;

	while (m_CurrentStep != ID_STEP_LAST && result != wxCANCEL)
	{

		if (m_CurrentStep == ID_STEP_CREATE_BODIES)
		{
			result = ShowDialog(lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_BODIES);
			continue;
		}

		if (m_CurrentStep == ID_STEP_CREATE_JOINTS)
		{
			result = ShowDialog(lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_JOINTS);
			continue;
		}

		if (m_CurrentStep == ID_STEP_CREATE_MARKERS)
		{
			result = ShowDialog(lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_MARKERS);
			continue;
		}

		if (m_CurrentStep == ID_STEP_CREATE_MUSCLES)
		{
			result = ShowDialog(lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_MUSCLES);
			continue;
		}

		if (m_CurrentStep == ID_STEP_WIZARD_FINISH)
		{
			result = ShowDialog(lhpGuiDialogCreateOpenSimModelWorkflow::WIZARD_FINISH);
			continue;
		}

	}

	if (result == wxCANCEL)
	{
		OpStop(OP_RUN_CANCEL);
		return;
	}


	OpStop(OP_RUN_OK);
}
//----------------------------------------------------------------------------
void lhpOpCreateOpenSimModelWorkflow::OpDo()   
	//----------------------------------------------------------------------------
{
	if (m_Output)
	{
		m_Output->ReparentTo(m_Input);
		mafEventMacro(mafEvent(this,CAMERA_UPDATE));
	}
}
//----------------------------------------------------------------------------
void lhpOpCreateOpenSimModelWorkflow::OpStop(int result)   
	//----------------------------------------------------------------------------
{
	mafEventMacro(mafEvent(this,result));
}
//----------------------------------------------------------------------------
void lhpOpCreateOpenSimModelWorkflow::OnEvent(mafEventBase *maf_event)
	//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{	
		case wxOK:
			OpStop(OP_RUN_OK);        
			break;
		
		case wxCANCEL:
			OpStop(OP_RUN_CANCEL);        
			break;
		
		default:
			mafEventMacro(*e);
			break;
		}
	}
	else
	{
		mafEventMacro(*e);
	}
}

//----------------------------------------------------------------------------
int lhpOpCreateOpenSimModelWorkflow::ShowDialog( int dialogMode )
//----------------------------------------------------------------------------
{
	lhpGuiDialogCreateOpenSimModelWorkflow *currentDialog = NULL;
	wxString dialogText = "dialog text";

	if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_BODIES)
	{
		dialogText = "step0 dialog text";
	}

	if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_JOINTS)
	{
		dialogText = "step1 dialog text";
	}

	if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_MARKERS)
	{
		dialogText = "step2 dialog text";
	}

	if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_MUSCLES)
	{
		dialogText = "step3 dialog text";
	}

	if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::WIZARD_FINISH)
	{
		dialogText = "step4 dialog text";
	}

	const wxString constCastDialogText = dialogText;

	currentDialog = new lhpGuiDialogCreateOpenSimModelWorkflow(constCastDialogText ,mafVMERoot::SafeDownCast(m_Input), dialogMode, this);

	int result = currentDialog->ShowModal();
	
	if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_BODIES)
	{

		if (result == wxID_RETURN_PREV)
		{
			// ???
		}
		else if (result == wxID_RETURN_NEXT)
		{

			wxString tmp = currentDialog->GetOpenSimAPIText();
			m_Step0_CreateBodies_Text = tmp;

			m_CurrentStep = ID_STEP_CREATE_JOINTS;
		}
		else
		{
			m_CurrentStep = ID_STEP_LAST;
		}

	}
	else if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_JOINTS)
	{
		if (result == wxID_RETURN_PREV)
		{
			m_CurrentStep = ID_STEP_CREATE_BODIES;
		}
		else if (result == wxID_RETURN_NEXT)
		{
			wxString tmp = currentDialog->GetOpenSimAPIText();
			m_Step1_CreateJoints_Text = tmp;

			m_CurrentStep = ID_STEP_CREATE_MARKERS;
		}
		else
		{
			m_CurrentStep = ID_STEP_LAST;
		}
	}
	
	else if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_MARKERS)
	{
		if (result == wxID_RETURN_PREV)
		{
			m_CurrentStep = ID_STEP_CREATE_BODIES;
		}
		else if (result == wxID_RETURN_NEXT)
		{
			wxString tmp = currentDialog->GetOpenSimAPIText();
			m_Step2_CreateMarkers_Text = tmp;

			m_CurrentStep = ID_STEP_CREATE_MUSCLES;
		}
		else
		{
			m_CurrentStep = ID_STEP_LAST;
		}
	}

	else if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::CREATE_MUSCLES)
	{
		if (result == wxID_RETURN_PREV)
		{
			m_CurrentStep = ID_STEP_CREATE_MARKERS;
		}
		else if (result == wxID_RETURN_NEXT)
		{
			wxString tmp = currentDialog->GetOpenSimAPIText();
			m_Step3_CreateMuscles_Text = tmp;

			m_CurrentStep = ID_STEP_WIZARD_FINISH;
		}
		else
		{
			m_CurrentStep = ID_STEP_LAST;
		}
	}
	else if (dialogMode == lhpGuiDialogCreateOpenSimModelWorkflow::WIZARD_FINISH)
	{
		if (result == wxID_RETURN_PREV)
		{
			m_CurrentStep = ID_STEP_CREATE_MARKERS;
		}
		else if (result == wxID_RETURN_NEXT)
		{
			assert(m_Input);
			GenerateFullModel(m_Input);
			m_CurrentStep = ID_STEP_LAST;
		}
		else
		{
			m_CurrentStep = ID_STEP_LAST;
		}
	}

	cppDEL(currentDialog);

	return result;
}

void lhpOpCreateOpenSimModelWorkflow::GenerateFullModel( mafNode *inputVme )
{
	wxString fullModel;

	wxString header = lhpGuiDialogCreateOpenSimModelWorkflow::GetModelHeader();
	wxString generateBodies = m_Step0_CreateBodies_Text;
	wxString generatedJoints = m_Step1_CreateJoints_Text;
	wxString generatedMarkers = m_Step2_CreateMarkers_Text;
	wxString generatedMuscles = m_Step3_CreateMuscles_Text;
	
	wxString footer = lhpGuiDialogCreateOpenSimModelWorkflow::GetModelFooter();

	fullModel.append(header);
	fullModel.append(generateBodies);
	fullModel.append(generatedJoints);
	fullModel.append(generatedMarkers);
	fullModel.append(generatedMuscles);
	fullModel.append(footer);

	// write the model to file
	wxString modelAbsFileName = lhpOpCreateOpenSimModel::GenerateOpenSimModelFromText(fullModel , inputVme);

	wxString path, localFileNameWithoutExtension, ext;

	wxSplitPath(modelAbsFileName, &path, &localFileNameWithoutExtension, &ext);

	std::ostringstream stringStream;
	stringStream << "Model: " << localFileNameWithoutExtension << " added to MSF tree" << std::endl;
	mafLogMessage(stringStream.str().c_str());

	wxMessageBox(stringStream.str().c_str());

	lhpOpModifyOpenSimModel::OpenFileWithNotepadPP(modelAbsFileName);
}
