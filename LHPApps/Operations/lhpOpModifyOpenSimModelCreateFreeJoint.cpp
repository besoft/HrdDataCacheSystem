/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateFreeJoint.cpp,v $
Language:  C++
Date:      $Date: 2012-03-16 14:27:41 $
Version:   $Revision: 1.1.2.2 $
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

#include "lhpOpModifyOpenSimModelCreateFreeJoint.h"

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
using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateFreeJoint);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_BODY_CHILD_VME_SURFACE,
	ID_GENERATE_JOINT,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateFreeJoint::lhpOpModifyOpenSimModelCreateFreeJoint(const wxString &label) :
lhpOpModifyOpenSimModel(label)
//----------------------------------------------------------------------------
{
	//------------------------------------
	m_TestMode = false;
	m_JointTestCase = BODY_WITH_GROUND;
	//------------------------------------

	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 
	m_Debug = 0;

	m_DictionaryToConfigureFileName = "OSIM_CreateFreeJoint_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreateFreeJoint_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreateFreeJoint_Dictionary.txt";

	// m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreateFreeJoint.cpp.in";
	m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreateFreeJointAsCustomJoint.cpp.in";

	m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;

	m_ConfiguredComponentFileName = "OSIM_CreateFreeJoint.cpp";

	m_JointId = 0;

	m_ArticularCentreVME = NULL;
	m_RotationAxisOriginVME = NULL;
	m_RotationAxisEndVME = NULL;

	m_ParentBodyVME = NULL;
	m_ChildBodyVME = NULL;

	m_RotationAxisDirectionCosines[X] = UNDEFINED_DOUBLE;
	m_RotationAxisDirectionCosines[Y] = UNDEFINED_DOUBLE;
	m_RotationAxisDirectionCosines[Z] = UNDEFINED_DOUBLE;

	m_UseGroundAsParent = true;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateFreeJoint::~lhpOpModifyOpenSimModelCreateFreeJoint( ) 
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateFreeJoint::Copy()   
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateFreeJoint(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateFreeJoint::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
 	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateFreeJoint::OpRun()   
//----------------------------------------------------------------------------
{	

	CreateGui();

	if (m_TestMode == true)
	{
		InitGUITestStuff();
	}

	ShowGui();

	
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateFreeJoint::CreateGui()
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
	m_Gui->Button(ID_CHOOSE_BODY_CHILD_VME_SURFACE, "Child Body");
	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
	m_Gui->Label("");

	m_Gui->Button(ID_GENERATE_JOINT, "Generate OpenSim Joint");
	m_Gui->Enable(ID_GENERATE_JOINT, false);
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
	m_Gui->Enable(ID_GENERATE_JOINT, false);
	m_Gui->OkCancel();

}



void lhpOpModifyOpenSimModelCreateFreeJoint::OnEvent(mafEventBase *maf_event)
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

		case ID_CHOOSE_BODY_CHILD_VME_SURFACE:
			{
				OnChooseChildVMESurface();
			}
			break;

		case ID_GENERATE_JOINT:
			{
				OnGenerateJoint();
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

void lhpOpModifyOpenSimModelCreateFreeJoint::OnChooseChildVMESurface()
{
	mafString title = mafString("Select child VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateFreeJoint::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ChildBodyVME = vme;

	m_Gui->Enable(ID_GENERATE_JOINT, true);
	m_Gui->Update();

}

bool lhpOpModifyOpenSimModelCreateFreeJoint::VmeSurfaceOpenSimBodyAccept(mafNode *node) 
{  	

	bool hasInertialTensorComponents = node->GetTagArray()->IsTagPresent("PRINCIPAL_INERTIAL_TENSOR_COMPONENTS");
	bool hasMass = node->GetTagArray()->IsTagPresent("SURFACE_MASS");
	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;
	return hasInertialTensorComponents && hasMass && isVmeSurface;

}

bool lhpOpModifyOpenSimModelCreateFreeJoint::VmeSurfaceAcceptLandmark(mafNode *node) 
{  	
	bool isVmeLandmark = mafVMELandmark::SafeDownCast(node) != NULL;
	return isVmeLandmark;
}

void lhpOpModifyOpenSimModelCreateFreeJoint::OnGenerateJoint()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateFreeJoint::WriteNMSBuilderDataToFile()
{
	wxString parentSurfaceVMEName;

	if (m_UseGroundAsParent == true)
	{
		parentSurfaceVMEName = "ground";
	}
	else
	{
		parentSurfaceVMEName = m_ParentBodyVME->GetName();
	}

	assert(m_ChildBodyVME);

	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream << 
		"[***ParentSurfaceVMEName_FromNMSB***]" << "," << ReplaceSpaceWithUnderscore(parentSurfaceVMEName.c_str()) << std::endl <<
		"[***ChildSurfaceVMEName_FromNMSB***]"  << "," << ReplaceSpaceWithUnderscore(m_ChildBodyVME->GetName()) << std::endl;
		
	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();

}


void lhpOpModifyOpenSimModelCreateFreeJoint::ComputeRotationAxisDirectionCosines()
{
	assert(m_RotationAxisOriginVME);
	assert(m_RotationAxisEndVME);

	double rotationAxisOriginABSPosition[3];
	double rotationAxisEndABSPosition[3];

	mafTransform::GetPosition(m_RotationAxisOriginVME->GetAbsMatrixPipe()->GetMatrix(), rotationAxisOriginABSPosition);
	mafTransform::GetPosition(m_RotationAxisEndVME->GetAbsMatrixPipe()->GetMatrix(), rotationAxisEndABSPosition);

	double dx = rotationAxisEndABSPosition[X] - rotationAxisOriginABSPosition[X];
	double dy = rotationAxisEndABSPosition[Y] - rotationAxisOriginABSPosition[Y];
	double dz = rotationAxisEndABSPosition[Z] - rotationAxisOriginABSPosition[Z];

	double d = sqrt( dx*dx + dy*dy + dz*dz);

	double cosAlfa = dx / d;
	double cosBeta = dy / d;
	double cosGamma = dz / d;

	m_RotationAxisDirectionCosines[X] = cosAlfa;
	m_RotationAxisDirectionCosines[Y] = cosBeta;
	m_RotationAxisDirectionCosines[Z] = cosGamma;

}

void lhpOpModifyOpenSimModelCreateFreeJoint::InitGUITestStuff()
{
	mafNode *root = m_Input->GetRoot();
	assert(root);

	m_ArticularCentreVME = mafVMELandmark::SafeDownCast(root->FindInTreeByName("J2TibiaL"));
	assert(m_ArticularCentreVME);

	m_RotationAxisOriginVME = mafVMELandmark::SafeDownCast(root->FindInTreeByName("LME"));
	assert(m_RotationAxisOriginVME);

	m_RotationAxisEndVME = mafVMELandmark::SafeDownCast(root->FindInTreeByName("LLE"));
	assert(m_RotationAxisEndVME);

	m_UseGroundAsParent = true;
	m_ParentBodyVME = NULL;
	
	m_ChildBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("TibiaL"));
	assert(m_ChildBodyVME);

	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
	m_Gui->Enable(ID_GENERATE_JOINT, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateFreeJoint::SetChildBodyVME( mafVME *surface )
{
	assert( VMEAcceptAsBody(surface));
	m_ChildBodyVME = surface;
}
