/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMarkerSet.cpp,v $
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

#include "lhpOpModifyOpenSimModelCreateMarkerSet.h"

#include "mafJointAnalysis.h"
#include <wx/mimetype.h>

#include "mafDecl.h"
#include "mafEvent.h"

#include "mafVMEExternalData.h"
#include "lhpUtils.h"
#include "mafGUI.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <iosfwd>
#include <string>
#include "mafVMESurface.h"
#include "mafDataVector.h"
#include "medOpImporterVTKXML.h"
#include "medOpExporterVTKXML.h"
#include "vtkPolyData.h"
#include "mafVMELandmark.h"
#include "mafTagArray.h"
#include "vtkMath.h"
#include "mafTransform.h"
#include "mafVMERefSys.h"
#include "mafMatrix3x3.h"
using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateMarkerSet);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD = lhpOpModifyOpenSimModel::ID_LAST,
	ID_CHOOSE_MARKER_SET_BODY,
	ID_GENERATE_MARKER_SET,
	ID_LISTBOX,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMarkerSet::lhpOpModifyOpenSimModelCreateMarkerSet(const wxString &label) :
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

	m_DictionaryToConfigureFileName = "OSIM_AssignMarkerSet_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_AssignMarkerSet_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_AssignMarkerSet_Dictionary.txt";

	m_ComponentToConfigureFileName = "OSIM_AssignMarkerSet.cpp.in";

	m_ConfiguredComponentFileName = "OSIM_AssignMarkerSet.cpp";

	m_MarkerSetBody = NULL;
	m_MarkerSetLandmarkCloud = NULL;

}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMarkerSet::~lhpOpModifyOpenSimModelCreateMarkerSet( ) 
	//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateMarkerSet::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateMarkerSet(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateMarkerSet::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMarkerSet::OpRun()   
	//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();

}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMarkerSet::CreateGui()
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
	m_Gui->Button(ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD, "Source Landmark cloud");
	m_Gui->Enable(ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD, true);

	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_MARKER_SET_BODY, "Target Body");
	m_Gui->Enable(ID_CHOOSE_MARKER_SET_BODY, false);

	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_MARKER_SET, "Generate Marker Set");
	m_Gui->Enable(ID_GENERATE_MARKER_SET, false);
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
	m_Gui->Enable(ID_GENERATE_MARKER_SET, false);
	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelCreateMarkerSet::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
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

	case ID_CHOOSE_MARKER_SET_LANDMARK_CLOUD:
		{
			OnChooseMarkerSetLandmarkCloud();
		}
		break;


	case ID_CHOOSE_MARKER_SET_BODY:
		{
			OnChooseMarkerSetBody();
		}
		break;

	case ID_GENERATE_MARKER_SET:
		{
			OnGenerateMarkerSet();
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

void lhpOpModifyOpenSimModelCreateMarkerSet::OnChooseMarkerSetBody()
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

	m_MarkerSetBody = vme;

	m_Gui->Enable(ID_GENERATE_MARKER_SET, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMarkerSet::OnChooseMarkerSetLandmarkCloud()
{
	mafString title = mafString("Choose Marker Set Landmark Cloud:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateMarkerSet::VMELandmarkCloudAcccept);
	mafEventMacro(e);
	mafVMELandmarkCloud *vme = mafVMELandmarkCloud::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_MarkerSetLandmarkCloud = vme;

	m_Gui->Enable(ID_CHOOSE_MARKER_SET_BODY, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMarkerSet::OnGenerateMarkerSet()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateMarkerSet::WriteNMSBuilderDataToFile()
{
	assert(m_MarkerSetBody);
	assert(m_MarkerSetLandmarkCloud);

	CreateCPPCodeTemplate();

	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream << 
		"[***bodyNameWithUnderscoresInsteadOfSpaces_FromNMSB***]" << "," << ReplaceSpaceWithUnderscore( m_MarkerSetBody->GetName() ) << std::endl <<
		"[***markerSet_FromNMSB***]" << "," << m_MarkerSetLandmarkCloud->GetName() << std::endl;

	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();

}


void lhpOpModifyOpenSimModelCreateMarkerSet::CreateCPPCodeTemplate()
{
	assert(m_MarkerSetBody);
	assert(m_MarkerSetLandmarkCloud);

	int numLandmarks = m_MarkerSetLandmarkCloud->GetNumberOfLandmarks();

	mafVMELandmark *lm = NULL;

	ostringstream stringStream;

	stringStream << 
   std::endl <<
	"//------------------------------------------------------" << std::endl <<
	"//" << std::endl <<
	"//  Create Marker Set from" << std::endl <<
	"//  Landmark Cloud: [***markerSet***]" << std::endl <<
	"//  assigned to " << std::endl <<
	"//  Body: " << m_MarkerSetBody->GetName() << std::endl <<
	"//------------------------------------------------------" << std::endl;
	
	// OpenSim::MarkerSet FemurLMarkerSet;
	stringStream << "OpenSim::MarkerSet markerSet[***bodyNameWithUnderscoresInsteadOfSpaces***];" << std::endl;
	stringStream << std::endl;
	
	for (int lmId = 0; lmId < numLandmarks; lmId++)
	{
		lm = m_MarkerSetLandmarkCloud->GetLandmark(lmId);

		string lmName = lm->GetName();
		double lmPos[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};
		lm->GetPoint(lmPos);

			stringStream << 

			//std::string landmark0FemurLName = "pippo";
			std::endl <<
			"std::string landmark" <<  lmId << "[***bodyNameWithUnderscoresInsteadOfSpaces***]Name = \"" << lmName << "\";" << std::endl <<

			//double landmark0FemurLPosition[3] = {-999, -999, -999};
			"double landmark" << lmId << "[***bodyNameWithUnderscoresInsteadOfSpaces***]Position[3] = { " << lmPos[0] / 1000 << " , " << lmPos[1] / 1000 << " , " << lmPos[2] / 1000 << " };" << std::endl <<

			//markerSetFemurL.addMarker(landmark0FemurLName , landmark0FemurLPosition, *FemurL);
			"markerSet[***bodyNameWithUnderscoresInsteadOfSpaces***].addMarker(landmark" << lmId << "[***bodyNameWithUnderscoresInsteadOfSpaces***]Name , landmark" << lmId << "[***bodyNameWithUnderscoresInsteadOfSpaces***]Position , *[***bodyNameWithUnderscoresInsteadOfSpaces***]);" << std::endl;

	}

	stringStream << std::endl <<

		// osimModel.updateMarkerSet(markerSetFemurL);
		"osimModel.updateMarkerSet(markerSet[***bodyNameWithUnderscoresInsteadOfSpaces***]);" << std::endl << 
		std::endl <<
		
	"//-----------------END SNIPPET--------------" << std::endl;

	ofstream myFile;

	wxString componentToConfigureABSFileName = GetOpenSimAPITemplatesDir() + m_ComponentToConfigureFileName.c_str();

	myFile.open(componentToConfigureABSFileName);
	myFile << stringStream.str().c_str();
	myFile.close();

	assert(wxFileExists(componentToConfigureABSFileName));

}

void lhpOpModifyOpenSimModelCreateMarkerSet::GenerateOpenSimComponentAPI()
{
	CreateCPPCodeTemplate();
	Superclass::GenerateOpenSimComponentAPI();
}

void lhpOpModifyOpenSimModelCreateMarkerSet::SetMarkerSetLandmarkCloud( mafVMELandmarkCloud *markerSetLandmarkCloud )
{
	if (markerSetLandmarkCloud != NULL)
	{
		lhpOpModifyOpenSimModel::VMELandmarkCloudAcccept(markerSetLandmarkCloud);
	}

	m_MarkerSetLandmarkCloud = markerSetLandmarkCloud;
}

void lhpOpModifyOpenSimModelCreateMarkerSet::SetMarkerSetBody( mafVME *markerSetBody )
{
	if (markerSetBody != NULL)
	{
		lhpOpModifyOpenSimModel::VMESurfaceAcceptForBodyGeneration(markerSetBody);
	}

	m_MarkerSetBody = markerSetBody;
}
