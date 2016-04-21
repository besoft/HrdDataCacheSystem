/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateMuscle.cpp,v $
Language:  C++
Date:      $Date: 2012-03-30 14:39:56 $
Version:   $Revision: 1.1.2.1 $
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

#include "lhpOpModifyOpenSimModelCreateMuscle.h"

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
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateMuscle);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{

	ID_CHOOSE_JOINT_TYPE = lhpOpModifyOpenSimModel::ID_LAST,

	ID_CHOOSE_ORIGIN_POINT_BODY,
	ID_CHOOSE_ORIGIN_POINT_LANDMARK,

	ID_CHOOSE_VIA_POINT_BODY,
	ID_CHOOSE_VIA_POINT_LANDMARK,

	ID_CHOOSE_INSERTION_POINT_BODY,
	ID_CHOOSE_INSERTION_POINT_LANDMARK,

	ID_GENERATE_MUSCLE,

};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMuscle::lhpOpModifyOpenSimModelCreateMuscle(const wxString &label) :
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

	m_DictionaryToConfigureFileName = "OSIM_CreateThelen2003Muscle_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreateThelen2003Muscle_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreateThelen2003Muscle_Dictionary.txt";

	m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreateThelen2003Muscle.cpp.in";
	m_ComponentToConfigureJointBetweenBodyAndGroundFileName = "OSIM_CreateThelen2003MuscleWithGround.cpp.in";

	m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;

	m_ConfiguredComponentFileName = "OSIM_CreateThelen2003Muscle.cpp";

	m_OriginPointBodyVME = NULL;
	m_ViaPointBodyVME = NULL;

	m_OriginPointBodyVME = NULL;
	m_OriginPointLandmarkVME = NULL;

	m_ViaPointBodyVME = NULL;
	m_ViaPointLandmarkVME = NULL;

	m_InsertionPointBodyVME = NULL;
	m_InsertionPointLandmarkVME = NULL;

}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateMuscle::~lhpOpModifyOpenSimModelCreateMuscle( ) 
	//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateMuscle::Copy()   
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateMuscle(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateMuscle::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateMuscle::OpRun()   
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
void lhpOpModifyOpenSimModelCreateMuscle::CreateGui()
	//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

	
//	const wxString choises[] = {"X", "Y", "Z"};
// 	m_Gui->Combo(ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS, "", &m_RefSysRotationAxis, 3, choises,   "");
//	m_Gui->Enable(ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS, true);

	/* 

	//------------------------------------------------------
	// 
	// Create a new Thelen2003Muscle 
	// 
	//------------------------------------------------------

	// Create a new muscle
	double maxIsometricForce = 1000.0;
	double optimalFiberLength = 0.1;
	double tendonSlackLength = 0.2;
	double pennationAngle = 0.0;
	double activation = 0.0001;
	double deactivation = 1.0;

	// Create new muscle using the Thelen 2003 muscle model
	Thelen2003Muscle *muscle = new Thelen2003Muscle("muscle",maxIsometricForce,optimalFiberLength,tendonSlackLength,pennationAngle);
	muscle->setActivationTimeConstant(activation);
	muscle->setDeactivationTimeConstant(deactivation);

	// Path for muscle
	muscle->addNewPathPoint("O0Lvas_lat", *FemurL, Vec3(0.364003, 0.268098, -0.324833));
	muscle->addNewPathPoint("W1Lvas_lat", *FemurL, Vec3(0.375801, 0.225617, -0.400191));
	muscle->addNewPathPoint("I0Lvas_lat", *TibiaL, Vec3(0.3621, 0.212821, -0.535217));

	// Add the muscle (as force) to the model
	osimModel.addForce(muscle);

	//-----------------END SNIPPET--------------

	*/

	m_Gui->Label("");
	m_Gui->Label("Muscle Origin" ,  true);
	m_Gui->Button(ID_CHOOSE_ORIGIN_POINT_BODY, "Origin Body");
	m_Gui->Enable(ID_CHOOSE_ORIGIN_POINT_BODY, true);
	m_Gui->Button(ID_CHOOSE_ORIGIN_POINT_LANDMARK, "Origin Landmark");
	m_Gui->Enable(ID_CHOOSE_ORIGIN_POINT_LANDMARK, false);
	
	m_Gui->Label("");
	m_Gui->Label("Muscle Via Point", true);
	m_Gui->Button(ID_CHOOSE_VIA_POINT_BODY, "Via Point Body");
	m_Gui->Enable(ID_CHOOSE_VIA_POINT_BODY, false);
	m_Gui->Button(ID_CHOOSE_VIA_POINT_LANDMARK, "Via Point Landmark");
	m_Gui->Enable(ID_CHOOSE_VIA_POINT_LANDMARK, false);
	m_Gui->Label("");

	m_Gui->Label("Muscle Insertion Point", true);
	m_Gui->Button(ID_CHOOSE_INSERTION_POINT_BODY, "Insertion Body");
	m_Gui->Enable(ID_CHOOSE_INSERTION_POINT_BODY, false);
	m_Gui->Button(ID_CHOOSE_INSERTION_POINT_LANDMARK, "Insertion Landmark");
	m_Gui->Enable(ID_CHOOSE_INSERTION_POINT_LANDMARK, false);
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
	m_Gui->Enable(ID_GENERATE_MUSCLE, false);
	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelCreateMuscle::OnEvent(mafEventBase *maf_event)
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch(e->GetId())
		{

		
		case ID_CHOOSE_ORIGIN_POINT_BODY:
			{
				OnChooseOriginPointBody();
			}
			break;


		case ID_CHOOSE_ORIGIN_POINT_LANDMARK:
			{
				OnChooseOriginPointLandmark();
			}
			break;


		case ID_CHOOSE_VIA_POINT_BODY:
			{
				OnChooseViaPointBody();
			}
			break;

		case ID_CHOOSE_VIA_POINT_LANDMARK:
			{
				OnChooseViaPointLandmark();
			}
			break;


		case ID_CHOOSE_INSERTION_POINT_BODY:
			{
				OnChooseInsertionPointBody();
			}
			break;


		case ID_CHOOSE_INSERTION_POINT_LANDMARK:
			{
				OnChooseInsertionPointLandmark();
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

void lhpOpModifyOpenSimModelCreateMuscle::OnChooseOriginPointBody()
{
	mafString title = mafString("Select Origin Point Body:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMESurfaceAcceptForBodyGeneration);
	mafEventMacro(e);
	mafVMESurface *vme = mafVMESurface::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_OriginPointBodyVME = vme;

	m_Gui->Enable(ID_CHOOSE_ORIGIN_POINT_LANDMARK, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMuscle::OnChooseOriginPointLandmark()
{
	mafString title = mafString("Select Origin Landmark:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateMuscle::VMELandmarkAccept);
	mafEventMacro(e);
	mafVMELandmark *vme = mafVMELandmark::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_OriginPointLandmarkVME = vme;

	m_Gui->Enable(ID_CHOOSE_VIA_POINT_BODY, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMuscle::OnChooseViaPointBody()
{
	mafString title = mafString("Select Via Point Body:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMELandmarkAccept);
	mafEventMacro(e);
	mafVMESurface *vme = mafVMESurface::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ViaPointBodyVME = vme;

	m_Gui->Enable(ID_CHOOSE_VIA_POINT_LANDMARK, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMuscle::OnChooseViaPointLandmark()
{
	mafString title = mafString("Select Via Point Landmark:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMELandmarkAccept);
	mafEventMacro(e);
	mafVMELandmark *vme = mafVMELandmark::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ViaPointLandmarkVME = vme;

	m_Gui->Enable(ID_CHOOSE_INSERTION_POINT_BODY, true);
	m_Gui->Update();

}


void lhpOpModifyOpenSimModelCreateMuscle::OnChooseInsertionPointBody()
{
	mafString title = mafString("Select Insertion Point Body:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateMuscle::VMERefSysOrVMESurfaceAccept);
	mafEventMacro(e);
	mafVMESurface *vme = mafVMESurface::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_InsertionPointBodyVME = vme;

	m_Gui->Enable(ID_CHOOSE_INSERTION_POINT_LANDMARK, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMuscle::OnChooseInsertionPointLandmark()
{
	mafString title = mafString("Select Via Point Landmark:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMELandmarkAccept);
	mafEventMacro(e);
	mafVMELandmark *vme = mafVMELandmark::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_InsertionPointLandmarkVME = vme;

	m_Gui->Enable(ID_GENERATE_MUSCLE, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMuscle::OnGenerateMuscle()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateMuscle::WriteNMSBuilderDataToFile()
{
	assert(m_OriginPointBodyVME);
	assert(m_ViaPointBodyVME);

	assert(m_OriginPointBodyVME);
	assert(m_OriginPointLandmarkVME);

	assert(m_ViaPointBodyVME);
	assert(m_ViaPointLandmarkVME);

	wxString originPointBodyName =  m_OriginPointBodyVME->GetName();
	wxString originPointLandmarkName = m_OriginPointLandmarkVME->GetName();
	double originPointPosition[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};
	m_OriginPointLandmarkVME->GetPoint(originPointPosition);

	wxString viaPointBodyName = m_ViaPointBodyVME->GetName();	
	wxString viaPointLandmarkName = m_ViaPointLandmarkVME->GetName();
	double viaPointPosition[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};
	m_ViaPointLandmarkVME->GetPoint(viaPointPosition);

	wxString insertionPointBodyName = m_InsertionPointBodyVME->GetName();
	wxString insertionPointLandmarkName = m_InsertionPointLandmarkVME->GetName();
	double insertionPointPosition[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};
	m_InsertionPointLandmarkVME->GetPoint(insertionPointPosition);

	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream << 
		"[***originPointBodyName_FromNMSB***]" << "," << originPointBodyName.c_str() << std::endl <<
		"[***originPointLandmarkName_FromNMSB***]" << "," << originPointLandmarkName.c_str() << std::endl <<
		"[***originPointPosition0_FromNMSB***]"  << "," << originPointPosition[0] / 1000 << std::endl <<
		"[***originPointPosition1_FromNMSB***]"  << "," << originPointPosition[1] / 1000 << std::endl <<
		"[***originPointPosition2_FromNMSB***]"  << "," << originPointPosition[2] / 1000 << std::endl <<
		
		"[***viaPointBodyName_FromNMSB***]" << "," << viaPointBodyName.c_str() << std::endl <<
		"[***viaPointLandmarkName_FromNMSB***]" << "," << viaPointLandmarkName.c_str() << std::endl <<
		"[***viaPointPosition0_FromNMSB***]"  << "," << viaPointPosition[0] / 1000 << std::endl <<
		"[***viaPointPosition1_FromNMSB***]"  << "," << viaPointPosition[1] / 1000 << std::endl <<
		"[***viaPointPosition2_FromNMSB***]"  << "," << viaPointPosition[2] / 1000 << std::endl <<

		"[***insertionPointBodyName_FromNMSB***]" << "," << insertionPointBodyName.c_str() << std::endl <<
		"[***insertionPointLandmarkName_FromNMSB***]" << "," << insertionPointLandmarkName.c_str() << std::endl <<
		"[***insertionPointPosition0_FromNMSB***]"  << "," << insertionPointPosition[0] / 1000 << std::endl <<
		"[***insertionPointPosition1_FromNMSB***]"  << "," << insertionPointPosition[1] / 1000 << std::endl <<
		"[***insertionPointPosition2_FromNMSB***]"  << "," << insertionPointPosition[2] / 1000 << std::endl;

	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();

}

void lhpOpModifyOpenSimModelCreateMuscle::InitGUITestStuff()
{
	mafNode *root = m_Input->GetRoot();
	assert(root);

	if (m_UseGroundAsParent == false)
	{
		m_OriginPointBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("FemurL"));
		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;
		assert(m_OriginPointBodyVME);
	}
	else
	{
		m_OriginPointBodyVME = NULL;
		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetweenBodyAndGroundFileName ;
	}

	m_ViaPointBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("TibiaL"));
	assert(m_ViaPointBodyVME);

	m_RefSysVME = mafVMERefSys::SafeDownCast(root->FindInTreeByName("ref_sys"));
	assert(m_RefSysVME);

	m_Gui->Enable(ID_CHOOSE_ORIGIN_POINT_BODY, true);

	m_Gui->Enable(ID_CHOOSE_VIA_POINT_BODY, true);
	m_Gui->Enable(ID_GENERATE_MUSCLE, true);

	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateMuscle::SetInputVMEs( 
	mafVMESurface *originPointBodyVME, mafVMELandmark *originPointLandmarkVME, 
	mafVMESurface *viaPointBodyVME, mafVMELandmark *viaPointLandmarkVME, 
	mafVMESurface *insertionPointBodyVME, mafVMELandmark *insertionPointLandmarkVME
	)
{
	m_OriginPointBodyVME = originPointBodyVME;
	m_OriginPointLandmarkVME = originPointLandmarkVME;
	m_ViaPointBodyVME = viaPointBodyVME;
	m_ViaPointLandmarkVME = viaPointLandmarkVME;
	m_InsertionPointBodyVME = insertionPointBodyVME;
	m_InsertionPointLandmarkVME = insertionPointLandmarkVME;
}
