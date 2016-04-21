/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMuscleMultiPoint.cpp,v $
Language:  C++
Date:      $Date: 2012-04-11 17:06:29 $
Version:   $Revision: 1.1.2.3 $
Authors:   Stefano Perticoni 
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpModifyOpenSimModelCreateMuscleMultiPoint.h"

#include <wx/mimetype.h>

#include "mafDecl.h"
#include "mafEvent.h"

#include "lhpUtils.h"
#include "mafGUI.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <iosfwd>
#include <string>
#include "mafVMESurface.h"
#include "mafDataVector.h"
#include "vtkPolyData.h"
#include "mafVMELandmark.h"
#include "mafTagArray.h"
#include "mafVMERefSys.h"

using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateMuscleMultiPoint);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateMuscleMultiPoint_ID
{
	ID_LISTBOX = lhpOpModifyOpenSimModel::ID_LAST,
	ID_ADD_POINT,
	ID_REMOVE_POINT,
	ID_GENERATE_MUSCLE,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMuscleMultiPoint::lhpOpModifyOpenSimModelCreateMuscleMultiPoint(const wxString &label) :
lhpOpModifyOpenSimModel(label)
//----------------------------------------------------------------------------
{
	//------------------------------------
	m_TestMode = false;  // default to false
	//------------------------------------

	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 
	m_Debug = 0;

	m_DictionaryToConfigureFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleMultiPoint_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleMultiPoint_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleMultiPoint_Dictionary.txt";

	m_ComponentToConfigureFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleMultiPoint.cpp.in";

	m_ConfiguredComponentFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleMultiPoint.cpp";

	m_ListBox = NULL;

}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMuscleMultiPoint::~lhpOpModifyOpenSimModelCreateMuscleMultiPoint( ) 
	//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateMuscleMultiPoint::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateMuscleMultiPoint(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateMuscleMultiPoint::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::OpRun()   
	//----------------------------------------------------------------------------
{	
	CreateGui();
	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::CreateGui()
//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

	mafEvent buildHelpGui;
	buildHelpGui.SetSender(this);
	buildHelpGui.SetId(GET_BUILD_HELP_GUI);
	mafEventMacro(buildHelpGui);

	if (buildHelpGui.GetArg() == true)
	{
		m_Gui->Button(ID_HELP, "Help","");	
	}

	m_Gui->Label("");

	m_ListBox = m_Gui->ListBox(ID_LISTBOX , "" , 150);

	m_Gui->Button(ID_ADD_POINT, _("Add"), "" ,"");
	m_Gui->Button(ID_REMOVE_POINT, _("Remove"), "" ,"");
	
	m_Gui->Label("");

	m_Gui->Button(ID_GENERATE_MUSCLE, "Generate OpenSim Muscle");
	m_Gui->Enable(ID_GENERATE_MUSCLE, false);
	m_Gui->Label("");

	m_Gui->Divider(2);
	m_Gui->Label("Generate Model", true);
	m_Gui->Label("");
	m_Gui->Button(ID_EDIT_MODEL_SOURCE, "Edit Model Source");
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_MODEL, "Generate Model");
	m_Gui->Label("");
	m_Gui->Divider(2);
	m_Gui->Label("Post processing facilities", true);
	m_Gui->Label("");
	m_Gui->Button(ID_OPEN_OUTPUT_MODEL_DIRECTORY, "Open output model files dir");
	m_Gui->Label("");
	m_Gui->Button(ID_OPEN_MSF_DIRECTORY, "Open MSF directory");
	m_Gui->Label("");
	m_Gui->Enable(ID_GENERATE_MUSCLE, true);
	m_Gui->Label("");
	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *eventChooseBody = mafEvent::SafeDownCast(maf_event))
	{
		switch(eventChooseBody->GetId())
		{

		case ID_HELP:
			{
				mafEvent helpEvent;
				helpEvent.SetSender(this);
				mafString operationLabel = this->m_Label;
				helpEvent.SetString(&operationLabel);
				helpEvent.SetId(OPEN_HELP_PAGE);
				mafEventMacro(helpEvent);
			}
			break;

		case ID_GENERATE_MUSCLE:
			{
				OnGenerateMuscle();
			}
			break;


			//------------------------------------------------------------------
			//		from superclass
			//------------------------------------------------------------------
		case ID_EDIT_MODEL_SOURCE:
			{
				lhpOpModifyOpenSimModel::EditOpenSIMFile();
			}
			break;

		case ID_GENERATE_MODEL:
			{
				lhpOpModifyOpenSimModel::BuildOpenSIMFile();
			}
			break;

		case ID_VIEW_MODEL_IN_OPENSIM:
			{
				lhpOpModifyOpenSimModel::ViewModelInOpenSim();
			}
			break;


		case ID_OPEN_OUTPUT_MODEL_DIRECTORY:
			{
				lhpOpModifyOpenSimModel::OnOpenOutputModelDirectory();
			}
			break;

		case ID_OPEN_MSF_DIRECTORY:
			{
				lhpOpModifyOpenSimModel::OnOpenMSFDirectory();
			}
			break;

		case ID_ADD_POINT:
			{
				OnAddPoint();
				return;
			}
			break;

		case ID_REMOVE_POINT:
			{ 
				OnRemovePoint();
			}
			break;


		case wxOK:
			{
				OpStop(OP_RUN_OK);
			}
			break;

		case wxCANCEL:
			{
				OpStop(OP_RUN_CANCEL);
			}
			break;

		}
	}
}

void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::OnGenerateMuscle()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::WriteNMSBuilderDataToFile()
{

	CreateCPPCodeTemplate();

	int listBoxSize = m_ListBox->GetCount();

	std::string originPointLmName = m_ListBox->GetString(0);
	std::string insertionPointLmName = m_ListBox->GetString(listBoxSize - 1);

	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream <<
	"[***originPointLandmarkName_FromNMSB***]" << "," << originPointLmName.c_str() << std::endl <<
	"[***insertionPointLandmarkName_FromNMSB***]" << "," << insertionPointLmName.c_str() << std::endl; // << 
		
	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();

}


void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::CreateCPPCodeTemplate()
{

	mafVMELandmark *lm = NULL;

	ostringstream stringStream;

	stringStream << 
		std::endl <<
		"//------------------------------------------------------" << std::endl <<
		"//" << std::endl <<
		"//  Create a new Thelen2003Muscle" << std::endl <<
		"//  Origin: [***originPointLandmarkName***]" << std::endl <<
		"//  Insertion: [***insertionPointLandmarkName***]" << std::endl <<
		"//------------------------------------------------------" << std::endl


		<< std::endl <<
		"// Create a new muscle" << std::endl <<
		"double maxIsometricForce[***originPointLandmarkName***][***insertionPointLandmarkName***] = 1000.0;" << std::endl <<
		"double optimalFiberLength[***originPointLandmarkName***][***insertionPointLandmarkName***] = 0.1;" << std::endl <<
		"double tendonSlackLength[***originPointLandmarkName***][***insertionPointLandmarkName***] = 0.2;" << std::endl <<
		"double pennationAngle[***originPointLandmarkName***][***insertionPointLandmarkName***] = 0.0;" << std::endl <<
		"double activation[***originPointLandmarkName***][***insertionPointLandmarkName***] = 0.0001;"  << std::endl <<
		"double deactivation[***originPointLandmarkName***][***insertionPointLandmarkName***] = 1.0;"  << std::endl <<

		std::endl <<
		"// Create new muscle using the Thelen 2003 muscle model" << std::endl <<
		"Thelen2003Muscle *muscle[***originPointLandmarkName***][***insertionPointLandmarkName***] = new Thelen2003Muscle(\"muscle\"," << std::endl <<
		"maxIsometricForce[***originPointLandmarkName***][***insertionPointLandmarkName***]," << std::endl <<
		"optimalFiberLength[***originPointLandmarkName***][***insertionPointLandmarkName***]," << std::endl <<
		"tendonSlackLength[***originPointLandmarkName***][***insertionPointLandmarkName***]," << std::endl <<
		"pennationAngle[***originPointLandmarkName***][***insertionPointLandmarkName***]);" << std::endl <<

		std::endl <<
		"muscle[***originPointLandmarkName***][***insertionPointLandmarkName***]->setActivationTimeConstant(activation[***originPointLandmarkName***][***insertionPointLandmarkName***]);" << std::endl <<
		"muscle[***originPointLandmarkName***][***insertionPointLandmarkName***]->setDeactivationTimeConstant(deactivation[***originPointLandmarkName***][***insertionPointLandmarkName***]);" << std::endl;


	//---------------------------------------------------------------------------------------
	// <start> code generation
	//---------------------------------------------------------------------------------------
	int listBoxSize = m_ListBox->GetCount();

	for (int lmId = 0; lmId < listBoxSize; lmId++)
	{
		std::string lmName = m_ListBox->GetString(lmId);

		mafVMELandmark *marker = m_LMNameLMVmeMap[lmName.c_str()];
		assert(marker !=NULL );

		mafVME *body = m_LMNameBodyVmeMap[lmName.c_str()];
		assert(body !=NULL );

		double lmPos[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};

		marker->GetPoint(lmPos);

		stringStream << std::endl <<
			"// Path for muscle"<< std::endl <<
			"muscle[***originPointLandmarkName***][***insertionPointLandmarkName***]->addNewPathPoint( \"" <<
			lmName.c_str() <<
		" \" , *" <<
			body->GetName() <<
		
		" , Vec3( " <<
			lmPos[0] / 1000 <<
		" , " << 
			lmPos[1] / 1000 <<
		" , " << 
			lmPos[2] / 1000 <<

		" ));" << std::endl;

	}

	//---------------------------------------------------------------------------------------
	// <end> code generation
	//---------------------------------------------------------------------------------------

	stringStream << std::endl <<

	"// Add the muscle (as force) to the model" << std::endl <<
	"osimModel.addForce(muscle[***originPointLandmarkName***][***insertionPointLandmarkName***]);" <<
	std::endl <<
	"//-----------------END SNIPPET--------------" << std::endl;

	ofstream myFile;
	wxString componentToConfigureABSFileName = GetOpenSimAPITemplatesDir() + m_ComponentToConfigureFileName.c_str();

	myFile.open(componentToConfigureABSFileName);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(componentToConfigureABSFileName));

}

void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::GenerateOpenSimComponentAPI()
{
	CreateCPPCodeTemplate();
	Superclass::GenerateOpenSimComponentAPI();
}

void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::OnAddPoint()
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

	mafString t;
	t = lmVme->GetName();

	m_ListBox->Append(_(t));
	m_ListBox->SetStringSelection(_(t));

	m_LMNameLMVmeMap[lmVme->GetName()] = lmVme;
	m_LMNameBodyVmeMap[lmVme->GetName()] = bodyVme;

	m_ListBox->Update();
	m_Gui->Update();
}

void lhpOpModifyOpenSimModelCreateMuscleMultiPoint::OnRemovePoint()
{
	if(m_ListBox->GetCount()!=0)
	{
		wxString name = m_ListBox->GetStringSelection();
		int number = m_ListBox->GetSelection();

		m_LMNameLMVmeMap.erase(name.c_str());
		m_LMNameBodyVmeMap.erase(name.c_str());

		m_ListBox->Delete(m_ListBox->FindString(m_ListBox->GetStringSelection()));          

	}
}

