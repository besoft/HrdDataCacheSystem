/*=========================================================================

Program: NMSBuilder
Module: lhpOpModifyOpenSimModelCreateCustomJointAPI
Authors: Stefano Perticoni

		 Copyright (c) B3C
		 All rights reserved. See Copyright.txt or
http://www.scsitaly.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
	the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

//==============================================================================
#include <OpenSim/OpenSim.h>
#include "lhpOpModifyOpenSimModel.h"

using namespace OpenSim;
using namespace std;
using SimTK::Vec3;

//==========================================================================================================
// Common Parameters for the simulations are just global.
const static double integ_accuracy = 1.0e-5;
const static double duration = 1.00;
const static Vec3 gravity_vec = SimTK::Vec3(0, -9.8065, 0);
//Femur
const static double femurMass = 8.806;
const static Vec3 femurCOM(0.00000000, -0.19503080, 0.00000000);
const static SimTK::Inertia femurInertiaAboutCOM(Vec3(0.1268, 0.0332, 0.1337));
//Tibia
const static SimTK::MassProperties tibiaMass(3.510, Vec3(0), SimTK::Inertia(Vec3(0.0477, 0.0048, 0.0484)));

// Joint locations
const static Vec3 hipInGround(0);
const static Vec3 hipInFemur(0.0, 0.0, 0.0);
const static Vec3 kneeInFemur(0.0, 0, 0);
const static Vec3 kneeInTibia(0.0, 0.0, 0.0);
//==========================================================================================================

#include "lhpBuilderDecl.h"

//----------------------------------------------------------------------------
// NOTE: Every CPP openSimFileName in the MAF must include "mafDefines.h" as first.
// This force to include Window, wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpOpModifyOpenSimModelCreateCustomJointAPI.h"

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
using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateCustomJointAPI);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_JOINT_TYPE = lhpOpModifyOpenSimModel::ID_LAST,
	ID_USE_GROUND_AS_PARENT,
	ID_CHOOSE_BODY_PARENT_VME_SURFACE,
	ID_CHOOSE_BODY_CHILD_VME_SURFACE,
	ID_GENERATE_JOINT,
	ID_CHOOSE_REF_SYS_VME,
	ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateCustomJointAPI::lhpOpModifyOpenSimModelCreateCustomJointAPI(const wxString &label) :
lhpOpModifyOpenSimModel(label)
	//----------------------------------------------------------------------------
{
	//------------------------------------
	m_TestMode = false;  // default to false
	m_UseGroundAsParent = false	;  // default to false
	//------------------------------------

	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 
	m_Debug = 0;

	m_DictionaryToConfigureFileName = "OSIM_CreateCustomJoint_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreateCustomJoint_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreateCustomJoint_Dictionary.txt";

	m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreateCustomJoint.cpp.in";
	m_ComponentToConfigureJointBetweenBodyAndGroundFileName = "OSIM_CreateCustomJointWithGround.cpp.in";

	m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;

	m_ConfiguredComponentFileName = "OSIM_CreateCustomJoint.cpp";

	m_FemurBodyVME = NULL;
	m_TibiaBodyVME = NULL;

	m_RefSysVME = NULL;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateCustomJointAPI::~lhpOpModifyOpenSimModelCreateCustomJointAPI( ) 
	//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateCustomJointAPI::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateCustomJointAPI(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateCustomJointAPI::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateCustomJointAPI::OpRun()   
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
void lhpOpModifyOpenSimModelCreateCustomJointAPI::CreateGui()
	//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

	m_Gui->Label("");
	//m_Gui->Button(ID_CHOOSE_REF_SYS_VME, "Reference  System");
	//m_Gui->Enable(ID_CHOOSE_REF_SYS_VME, true);
	m_Gui->Label("Custom Joint mockup for MAF/OpenSim");
	m_Gui->Label("API integration Demo");

	//m_Gui->Label("Use ground as parent");
	//m_Gui->Bool(ID_USE_GROUND_AS_PARENT, "", &m_UseGroundAsParent);
	//m_Gui->Enable(ID_USE_GROUND_AS_PARENT, false);
	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_BODY_PARENT_VME_SURFACE, "Parent Body");
	m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);
	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_BODY_CHILD_VME_SURFACE, "Child Body");
	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_JOINT, "Generate OpenSim Joint");
	m_Gui->Enable(ID_GENERATE_JOINT, true);
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



void lhpOpModifyOpenSimModelCreateCustomJointAPI::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{

		case ID_CHOOSE_REF_SYS_VME:
			{
				OnChooseRefSysVME();
			}
			break;

		case ID_USE_GROUND_AS_PARENT:
			{
				OnChooseGroundAsParent();
			}
			break;

		case ID_CHOOSE_BODY_PARENT_VME_SURFACE:
			{
				OnChooseParentBodyVME();
			}
			break;

		case ID_CHOOSE_BODY_CHILD_VME_SURFACE:
			{
				OnChooseChildBodyVME();
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

void lhpOpModifyOpenSimModelCreateCustomJointAPI::OnChooseParentBodyVME()
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

	m_FemurBodyVME = vme;

	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateCustomJointAPI::OnChooseChildBodyVME()
{
	mafString title = mafString("Select child VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateCustomJointAPI::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_TibiaBodyVME = vme;

	m_Gui->Enable(ID_GENERATE_JOINT, true);
	m_Gui->Update();

}
bool lhpOpModifyOpenSimModelCreateCustomJointAPI::IsVMESurfaceAndCanBeUsedAsOpenSimBody(mafNode *node) 
{  	

	bool hasInertialTensorComponents = node->GetTagArray()->IsTagPresent("PRINCIPAL_INERTIAL_TENSOR_COMPONENTS");
	bool hasMass = node->GetTagArray()->IsTagPresent("SURFACE_MASS");
	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;
	return hasInertialTensorComponents && hasMass && isVmeSurface;

}

void lhpOpModifyOpenSimModelCreateCustomJointAPI::OnGenerateJoint()
{
	Generate();
	// GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateCustomJointAPI::Generate()
{

	try {

		// Define spline data for the custom knee joint
		int npx = 12;
		double angX[] = {-2.094395102393, -1.745329251994, -1.396263401595, -1.047197551197, -0.698131700798, -0.349065850399, -0.174532925199, 0.197344221443, 0.337394955864, 0.490177570472, 1.521460267071, 2.094395102393};
		double kneeX[] = {-0.003200000000, 0.001790000000, 0.004110000000, 0.004100000000, 0.002120000000, -0.001000000000, -0.003100000000, -0.005227000000, -0.005435000000, -0.005574000000, -0.005435000000, -0.005250000000};
		int npy = 7;
		double angY[] = {-2.094395102393, -1.221730476396, -0.523598775598, -0.349065850399, -0.174532925199, 0.159148563428, 2.094395102393};
		double kneeY[] = {-0.422600000000, -0.408200000000, -0.399000000000, -0.397600000000, -0.396600000000, -0.395264000000, -0.396000000000 };

		OpenSim::SimmSpline tx(npx, angX, kneeX);
		OpenSim::SimmSpline ty(npy, angY, kneeY);;

		// Setup OpenSim model
		Model *model = new Model;
		model->setName("buildCustomJointExample");
		// OpenSim bodies
		// ground
		Body& ground = model->getGroundBody();

		// thigh
		Body *femur = new Body(m_FemurBodyVME->GetName(), femurMass, femurCOM, femurInertiaAboutCOM);

		wxString femurVTPFileName = m_FemurBodyVME->GetName() + wxString(".vtp");

		wxString outputFemurSurfaceFileName = GetOutputModelDirectory() + femurVTPFileName;

		medOpExporterVTKXML *exporterFemur=new medOpExporterVTKXML();
		exporterFemur->SetInput(m_FemurBodyVME);
		exporterFemur->TestModeOn();
		exporterFemur->SetFileName(outputFemurSurfaceFileName.c_str());
		exporterFemur->ExportAsBynaryOn();
		exporterFemur->ExportVTK();
		vtkDEL(exporterFemur);

		femur->addDisplayGeometry(femurVTPFileName.c_str());

		// Define hip coordinates and axes for custom joint
		// create pin hip joint
		// PinJoint(joint_name, Body& parent, Vec3 locationInParent, Vec3 orientationInParent,
		//			Body& body, Vec3 locationInBody, orientationInBody)
		PinJoint *hip = new PinJoint("customJoint", ground, hipInGround, Vec3(0), *femur, hipInFemur, Vec3(0));

		// Set the angle and position ranges for the hip
		double hipRange[2] = {-SimTK::Pi/2, SimTK::Pi/2};
		hip->getCoordinateSet()[0].setRange(hipRange);

		// Add the femur body which now also contains the hip joint to the model
		model->addBody(femur);

		// Tibia body to be connected via a knee joint
		Body *tibia = new Body(m_TibiaBodyVME->GetName(), tibiaMass.getMass(), tibiaMass.getMassCenter(), tibiaMass.getInertia());

		wxString surfaceVtkAbsFileName = GetVTKFileNameAbsPath(mafVMESurface::SafeDownCast(m_TibiaBodyVME));
		assert(wxFileExists(surfaceVtkAbsFileName));

		wxString tibiaVTPFileName = m_TibiaBodyVME->GetName() + wxString(".vtp");

		wxString outputTibiaSurfaceFileName = GetOutputModelDirectory() + tibiaVTPFileName;

		medOpExporterVTKXML *exporterTibia=new medOpExporterVTKXML();
		exporterTibia->SetInput(m_TibiaBodyVME);
		exporterTibia->TestModeOn();
		exporterTibia->SetFileName(outputTibiaSurfaceFileName.c_str());
		exporterTibia->ExportAsBynaryOn();
		exporterTibia->ExportVTK();
		vtkDEL(exporterTibia);

		tibia->addDisplayGeometry(tibiaVTPFileName.c_str());


		// Define knee coordinates and axes for custom joint's spatial transform.
		// A spatial transform describes the spatial (6-dof) permissible motion
		// between two joint frames. A SpatialTransform is composed of 6 transform axes
		// (X,Y,Z body fixed rotations & X, Y, Z translations of the child joint 
		// frame in the parent, in this order). An unspecified axis has no coordinates
		// associate and is a constant 0 value.
		SpatialTransform kneeTransform;
		// the single coordinate (knee_extension) parameterizing the motion of this joint
		string coord_name = "knee_extension";
		// the list of coordinates the spatial transform is dependent on 
		Array<string> coords(coord_name, 1, 1);

		// knee flexion/extension is rotation about joint Z (3rd axis)
		kneeTransform[2].setCoordinateNames(coords);
		kneeTransform[2].setFunction(new LinearFunction());
		// tibia translation X is first translation (4th axis)
		kneeTransform[3].setCoordinateNames(coords);
		kneeTransform[3].setFunction(tx);
		// tibia translation Y (5th axis)
		kneeTransform[4].setCoordinateNames(coords);
		kneeTransform[4].setFunction(ty);

		// create custom knee joint
		CustomJoint *knee = new CustomJoint("knee", *femur, kneeInFemur, Vec3(0), *tibia, kneeInTibia, Vec3(0), kneeTransform);

		// Set the angle and position ranges for the knee
		double kneeRange[2] = {-2*SimTK::Pi/3, 0};
		knee->getCoordinateSet()[0].setRange(kneeRange);

		// Add the shank body which now also contains the knee joint to the model
		model->addBody(tibia);

		// add forces
		model->setGravity(gravity_vec);

		// write out the model to file

		wxString oldDir = wxGetCwd();

		wxSetWorkingDirectory(GetOutputModelDirectory());

		wxString outputModelFileName = "CustomJointMockUp.osim";

		model->print(outputModelFileName.c_str());

		wxString outputModeABSFileName = GetOutputModelDirectory() + outputModelFileName;

		wxString msg = "Output model written to " + outputModeABSFileName;

		wxMessageBox(msg);

		mafLogMessage(msg);

		wxSetWorkingDirectory(oldDir);


	}
	catch (const OpenSim::Exception &ex)
	{
		ex.print(cout);
		return;
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what() << std::endl;
		return;
	}
	catch (...)
	{
		std::cout << "UNRECOGNIZED EXCEPTION" << std::endl;
		return;
	}

	std::cout << "OpenSim example completed successfully.\n";
	std::cin.get();
	return;

}

void lhpOpModifyOpenSimModelCreateCustomJointAPI::InitGUITestStuff()
{
	mafNode *root = m_Input->GetRoot();
	assert(root);

	if (m_UseGroundAsParent == false)
	{
		m_FemurBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("FemurL"));
		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;
		assert(m_FemurBodyVME);
	}
	else
	{
		m_FemurBodyVME = NULL;
		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetweenBodyAndGroundFileName ;
	}

	m_TibiaBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("TibiaL"));
	assert(m_TibiaBodyVME);

	m_RefSysVME = mafVMERefSys::SafeDownCast(root->FindInTreeByName("ref_sys"));
	assert(m_RefSysVME);

	m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);

	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
	m_Gui->Enable(ID_GENERATE_JOINT, true);

	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME, true);
	m_Gui->Enable(ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS, true);

	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateCustomJointAPI::OnChooseGroundAsParent()
{
	if (m_UseGroundAsParent == true)
	{
		m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, false);
		m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
		m_Gui->Update();

		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetweenBodyAndGroundFileName ;
	}
	else if (m_UseGroundAsParent == false)
	{
		m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);
		m_Gui->Update();

		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;
	}
}

bool lhpOpModifyOpenSimModelCreateCustomJointAPI::VMERefSysOrVMESurfaceAccept(mafNode *node) 
{  	
	bool isVmeRefSys = mafVMERefSys::SafeDownCast(node) != NULL;

	if (isVmeRefSys)
	{
		return true;
	}

	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;

	if (isVmeSurface)
	{
		return true;
	}

	return false;
}

void lhpOpModifyOpenSimModelCreateCustomJointAPI::OnChooseRefSysVME()
{
	mafString title = mafString("VME Ref Sys:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateCustomJointAPI::VMERefSysOrVMESurfaceAccept);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_RefSysVME = vme;

	m_Gui->Update();

	m_Gui->Enable(ID_USE_GROUND_AS_PARENT, true);
	m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);
	m_Gui->Update();

}
