/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMuscleFromMeter.cpp,v $
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

#include "lhpOpModifyOpenSimModelCreateMuscleFromMeter.h"

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
#include "mafVMELandmarkCloud.h"
#include "medVMEComputeWrapping.h"
#include "mafVMEGroup.h"
#include "mafVME.h"

using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateMuscleFromMeter);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateMuscleFromMeter_ID
{
	ID_LISTBOX = lhpOpModifyOpenSimModel::ID_LAST,
	ID_ADD_POINT,
	ID_REMOVE_POINT,
	ID_GENERATE_MUSCLE,
	ID_CHOOSE_WRAPPED_METER,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMuscleFromMeter::lhpOpModifyOpenSimModelCreateMuscleFromMeter(const wxString &label) :
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

	m_DictionaryToConfigureFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleFromMeter_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleFromMeter_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleFromMeter_Dictionary.txt";

	m_ComponentToConfigureFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleFromMeter.cpp.in";

	m_ConfiguredComponentFileName = "OSIM_lhpOpModifyOpenSimModelCreateMuscleFromMeter.cpp";

	m_Meter = NULL;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMuscleFromMeter::~lhpOpModifyOpenSimModelCreateMuscleFromMeter( ) 
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateMuscleFromMeter::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateMuscleFromMeter(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateMuscleFromMeter::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMuscleFromMeter::OpRun()   
	//----------------------------------------------------------------------------
{	
	CreateGui();
	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMuscleFromMeter::CreateGui()
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

	m_Gui->Label("The following tags will be read");
	m_Gui->Label("from the action line if available");
	m_Gui->Label("otherwise default values will");
	m_Gui->Label("be used:");
	m_Gui->Label("maxIsometricForce");
	m_Gui->Label("optimalFiberLength");
	m_Gui->Label("tendonSlackLength");
	m_Gui->Label("");

	m_Gui->Button(ID_CHOOSE_WRAPPED_METER,"Source Action Line");
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

	m_Gui->Enable(ID_GENERATE_MUSCLE, false);
}



void lhpOpModifyOpenSimModelCreateMuscleFromMeter::OnEvent(mafEventBase *maf_event)
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

		case ID_CHOOSE_WRAPPED_METER:
			{
				// choose the compute wrapping vme
				mafString title = _("Choose action line vme");
				mafEvent eventChooseLandmark(this,VME_CHOOSE);
				eventChooseLandmark.SetString(&title);
				eventChooseLandmark.SetArg((long)&lhpOpModifyOpenSimModelCreateMuscleFromMeter::VMEMeterAccept);
				mafEventMacro(eventChooseLandmark);

				medVMEComputeWrapping *meterVme = medVMEComputeWrapping::SafeDownCast(eventChooseLandmark.GetVme());	

				if (meterVme == NULL)
				{
					return;
				}

				m_Meter = meterVme;

				Fill_m_Cloud_LandmarkIdVector(m_Meter);
				
				m_Gui->Enable(ID_GENERATE_MUSCLE, true);
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

void lhpOpModifyOpenSimModelCreateMuscleFromMeter::OnGenerateMuscle()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateMuscleFromMeter::WriteNMSBuilderDataToFile()
{
	CreateCPPCodeTemplate();

	std::string originPointLmName = "";
	std::string insertionPointLmName = "";

	originPointLmName = m_Cloud_LandmarkIdVector[0].first->GetLandmark(m_Cloud_LandmarkIdVector[0].second)->GetName();
	insertionPointLmName = m_Cloud_LandmarkIdVector[m_Cloud_LandmarkIdVector.size() - 1].first->GetLandmark(m_Cloud_LandmarkIdVector[m_Cloud_LandmarkIdVector.size() - 1].second)->GetName();

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


void lhpOpModifyOpenSimModelCreateMuscleFromMeter::CreateCPPCodeTemplate()
{
	
	wxString tagMaxIsometricForce = "maxIsometricForce";
	double maxIsometricForce =  1000.0;

	if (m_Meter->GetTagArray()->IsTagPresent(tagMaxIsometricForce.c_str()))
	{
		std::ostringstream stringStream;
		stringStream << tagMaxIsometricForce.c_str() << " tag found. Using value " << maxIsometricForce << std::endl;          			
		mafLogMessage(stringStream.str().c_str());

		maxIsometricForce = m_Meter->GetTagArray()->GetTag(tagMaxIsometricForce.c_str())->GetValueAsDouble();

	}
	else
	{
		std::ostringstream stringStream;
		stringStream << tagMaxIsometricForce.c_str() << " tag not found. Using default value ie " << maxIsometricForce << std::endl;          			
		mafLogMessage(stringStream.str().c_str());
	}


	wxString tagOptimalFiberLength = "optimalFiberLength";
	double optimalFiberLength = 0.1;

	if (m_Meter->GetTagArray()->IsTagPresent(tagOptimalFiberLength.c_str()))
	{
		std::ostringstream stringStream;
		stringStream << tagOptimalFiberLength.c_str() << " tag found. Using value " << optimalFiberLength << std::endl;          			
		mafLogMessage(stringStream.str().c_str());

		optimalFiberLength = m_Meter->GetTagArray()->GetTag(tagOptimalFiberLength.c_str())->GetValueAsDouble();
	}
	else
	{
		std::ostringstream stringStream;
		stringStream << tagOptimalFiberLength.c_str() << " tag not found. Using default value ie " << optimalFiberLength << std::endl;          			
		mafLogMessage(stringStream.str().c_str());
	}


	wxString tagTendonSlackLength = "tendonSlackLength";
	double tendonSlackLength = 0.2;

	if (m_Meter->GetTagArray()->IsTagPresent(tagTendonSlackLength.c_str()))
	{
		std::ostringstream stringStream;
		stringStream << tagTendonSlackLength.c_str() << " tag found. Using value " << tendonSlackLength << std::endl;          			
		mafLogMessage(stringStream.str().c_str());

		tendonSlackLength = m_Meter->GetTagArray()->GetTag(tagTendonSlackLength.c_str())->GetValueAsDouble();
	}
	else
	{
		std::ostringstream stringStream;
		stringStream << tagTendonSlackLength.c_str() << " tag not found. Using default value ie " << tendonSlackLength << std::endl;          			
		mafLogMessage(stringStream.str().c_str());
	}

	mafVMELandmark *lm = NULL;

	ostringstream stringStream;

	stringStream << 
		std::endl <<
		"//------------------------------------------------------" << std::endl <<
		"//" << std::endl <<
		"//  Create a new Thelen2003Muscle" << std::endl <<
		"//  Name: " << m_Meter->GetName() << std::endl <<
		"//  Origin: [***originPointLandmarkName***]" << std::endl <<
		"//  Insertion: [***insertionPointLandmarkName***]" << std::endl <<
		"//------------------------------------------------------" << std::endl


		<< std::endl <<
		"// Create a new muscle" << std::endl <<
		"double maxIsometricForce[***originPointLandmarkName***][***insertionPointLandmarkName***] = " << maxIsometricForce << ";" << std::endl <<
		"double optimalFiberLength[***originPointLandmarkName***][***insertionPointLandmarkName***] = " << optimalFiberLength << ";" << std::endl <<
		"double tendonSlackLength[***originPointLandmarkName***][***insertionPointLandmarkName***] = " << tendonSlackLength << ";" << std::endl <<
		"double pennationAngle[***originPointLandmarkName***][***insertionPointLandmarkName***] = 0.0;" << std::endl <<
		"double activation[***originPointLandmarkName***][***insertionPointLandmarkName***] = 0.0001;"  << std::endl <<
		"double deactivation[***originPointLandmarkName***][***insertionPointLandmarkName***] = 1.0;"  << std::endl <<

		std::endl <<
		"// Create new muscle using the Thelen 2003 muscle model" << std::endl <<
		"Thelen2003Muscle *muscle[***originPointLandmarkName***][***insertionPointLandmarkName***] = new Thelen2003Muscle(\"" << m_Meter->GetName() << "\"," << std::endl <<
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

	for (int lmId = 0; lmId < m_Cloud_LandmarkIdVector.size(); lmId++)
	{
		mafVMELandmark *lm = NULL;
		lm = m_Cloud_LandmarkIdVector[lmId].first->GetLandmark(m_Cloud_LandmarkIdVector[lmId].second);

		std::string lmName = lm->GetName();

		mafVMELandmark *marker = lm; // need the vme landmark to get the position
		assert(marker !=NULL );

		mafVME *body = lm->GetParent()->GetParent(); // need the body vme to get the body name
		assert(body->IsA("mafVMEGroup"));
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

void lhpOpModifyOpenSimModelCreateMuscleFromMeter::GenerateOpenSimComponentAPI()
{
	assert(m_Meter);
	Fill_m_Cloud_LandmarkIdVector(m_Meter);
	CreateCPPCodeTemplate();
	Superclass::GenerateOpenSimComponentAPI();
}



void lhpOpModifyOpenSimModelCreateMuscleFromMeter::Fill_m_Cloud_LandmarkIdVector( medVMEComputeWrapping *meterVme )
{
	if (meterVme == NULL) return;

	m_Cloud_LandmarkIdVector.clear();

	std::ostringstream stringStream;
	stringStream << "Choosed action line: " << meterVme->GetName() << std::endl;          

	// print start vme, middle points, end vme

	// get the name of the LC parent of the start landmark
	stringStream << "start vme: " << meterVme->GetStartVME()->GetName() << std::endl;

	// get the origin landmark
	mafVME *start_vme = meterVme->GetStartVME();
	if (start_vme && start_vme->IsMAFType(mafVMELandmarkCloud))
	{
		int sub_id = meterVme->GetLinkSubId("StartVME");
		mafVMELandmark *startLm = (sub_id != -1) ? ((mafVMELandmarkCloud *)start_vme)->GetLandmark(sub_id) : NULL;

		if (startLm != NULL)
		{
			stringStream << "start landmark name: " << startLm->GetName() << std::endl; 
			
			
			mafVMELandmarkCloud *lc = mafVMELandmarkCloud::SafeDownCast(startLm->GetParent());
			assert(lc);

			mafVMEGroup *b = mafVMEGroup::SafeDownCast(lc->GetParent());
			assert(b);

			mafVMEGroup *body = b;

			assert(body);

			m_Cloud_LandmarkIdVector.push_back(pair<mafVMELandmarkCloud *, int >(mafVMELandmarkCloud::SafeDownCast(start_vme), sub_id));
		}
	}
	
	// get the number of middle points
	stringStream << "Number of middle points: " << meterVme->GetNumberMiddlePoints() << std::endl;

	// get ordered middle points list

	std::vector<mafString> orderMiddlePointsNameVMEList; //order list of VME Name

	orderMiddlePointsNameVMEList.clear();
	for(int j=0; j<  meterVme->m_OrderMiddlePointsVMEList.size();j++)
	{
		for (mafNode::mafLinksMap::iterator i = meterVme->GetLinks()->begin(); i != meterVme->GetLinks()->end(); ++i)
		{	
			if(i->first.Equals("StartVME")) continue;
			else if(i->first.Equals("EndVME1")) continue;
			else if(i->first.Equals("EndVME2")) continue;
			else if(i->first.Equals("WrappedVME")) continue;
			else if(i->second.m_Node->GetId() == meterVme->m_OrderMiddlePointsVMEList[j])
			{
				if(mafVMELandmarkCloud *lc = mafVMELandmarkCloud::SafeDownCast(i->second.m_Node))
				{
					int idx = meterVme->m_OrderMiddlePointsVMEList[++j];
					mafVMELandmark *landmark  = lc->GetLandmark(idx);
					if(landmark)
					{
						orderMiddlePointsNameVMEList.push_back(landmark->GetName());

						m_Cloud_LandmarkIdVector.push_back(pair<mafVMELandmarkCloud *, int >(lc, idx));
					}
				}
				else
				{
					assert(false);
					//m_ListBox->Append(i->second.m_Node->GetName());

					//cloud_LandmarkIdVector.push_back(NULL , )
					//orderMiddlePointsNameVMEList.push_back(i->second.m_Node->GetName());
				}
			}
		}
	}

	for (int i = 0 ; i < orderMiddlePointsNameVMEList.size(); i++)
	{
		mafVMELandmarkCloud *landmarkCloud = NULL;
		landmarkCloud = m_Cloud_LandmarkIdVector[i].first;
		int landmarkId = -1;
		landmarkId = m_Cloud_LandmarkIdVector[i].second;
		mafVMELandmark *lm = NULL;
		lm = landmarkCloud->GetLandmark(landmarkId);

		stringStream << "Landmark cloud: " << landmarkCloud->GetName() << "     Landmark: " << lm->GetName()<< std::endl;
	}

	// get the name of the LC parent of the end landmark
	stringStream << "end1 vme: " << meterVme->GetEnd1VME()->GetName() << std::endl;				

	// get the insertion landmark
	mafVME *end_vme1 = meterVme->GetEnd1VME();
	if (end_vme1 && end_vme1->IsMAFType(mafVMELandmarkCloud))
	{
		int sub_id = meterVme->GetLinkSubId("EndVME1");
		mafVMELandmark *endLm = (sub_id != -1) ? ((mafVMELandmarkCloud *)end_vme1)->GetLandmark(sub_id) : NULL;

		if (endLm != NULL)
		{
			stringStream << "end landmark name: " << endLm->GetName() << std::endl; 
			m_Cloud_LandmarkIdVector.push_back(pair<mafVMELandmarkCloud *, int >(mafVMELandmarkCloud::SafeDownCast(end_vme1), sub_id));
		}
	}

	mafLogMessage(stringStream.str().c_str());

}
