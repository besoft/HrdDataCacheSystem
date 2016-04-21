/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreatePinJoint.cpp,v $
Language:  C++
Date:      $Date: 2012-03-26 09:08:21 $
Version:   $Revision: 1.1.2.11 $
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

#include "lhpOpModifyOpenSimModelCreatePinJoint.h"

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
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreatePinJoint);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateJoint_ID
{
	ID_CHOOSE_JOINT_TYPE = lhpOpModifyOpenSimModel::ID_LAST,
	ID_USE_GROUND_AS_PARENT,
	ID_CHOOSE_BODY_PARENT_VME_SURFACE,
	ID_CHOOSE_BODY_CHILD_VME_SURFACE,
	ID_GENERATE_JOINT,
	ID_CHOOSE_REF_SYS_VME_IN_PARENT,
	ID_CHOOSE_REF_SYS_VME_IN_CHILD,
	ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS,
};

enum {X = 0, Y= 1 , Z = 2};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreatePinJoint::lhpOpModifyOpenSimModelCreatePinJoint(const wxString &label) :
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

	m_DictionaryToConfigureFileName = "OSIM_CreatePinJoint_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreatePinJoint_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreatePinJoint_Dictionary.txt";

	// m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreatePinJoint.cpp.in";
	m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreatePinJointAsCustomJoint.cpp.in";

	// m_ComponentToConfigureJointBetweenBodyAndGroundFileName = "OSIM_CreatePinJointWithGround.cpp.in";
	m_ComponentToConfigureJointBetweenBodyAndGroundFileName = "OSIM_CreatePinJointAsCustomJointWithGround.cpp.in";

	m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;

	m_ConfiguredComponentFileName = "OSIM_CreatePinJoint.cpp";

	m_ParentBodyVME = NULL;
	m_ChildBodyVME = NULL;

	m_RefSysVMEInParent = NULL;
	m_RefSysVMEInChild = NULL;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreatePinJoint::~lhpOpModifyOpenSimModelCreatePinJoint( )
	//----------------------------------------------------------------------------
{
}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreatePinJoint::Copy()
	//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreatePinJoint(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreatePinJoint::Accept(mafNode *node)
	//----------------------------------------------------------------------------
{
	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreatePinJoint::OpRun()
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
void lhpOpModifyOpenSimModelCreatePinJoint::CreateGui()
	//----------------------------------------------------------------------------
{
	m_Gui = new mafGUI(this);
	m_Gui->SetListener(this);

//	const wxString choises[] = {"X", "Y", "Z"};
// 	m_Gui->Combo(ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS, "", &m_RefSysRotationAxis, 3, choises,   "");
//	m_Gui->Enable(ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS, true);

	mafEvent buildHelpGui;
	buildHelpGui.SetSender(this);
	buildHelpGui.SetId(GET_BUILD_HELP_GUI);
	mafEventMacro(buildHelpGui);

	if (buildHelpGui.GetArg() == true)
	{
		m_Gui->Button(ID_HELP, "Help","");	
	}

	m_Gui->Label("");
	m_Gui->Label("Use ground as parent");
	m_Gui->Bool(ID_USE_GROUND_AS_PARENT, "", &m_UseGroundAsParent);
	m_Gui->Enable(ID_USE_GROUND_AS_PARENT, true);
	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_BODY_PARENT_VME_SURFACE, "Parent Body");
	m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);
	m_Gui->Button(ID_CHOOSE_REF_SYS_VME_IN_PARENT, "RefSys in Parent");
	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, false);
	m_Gui->Label("");
	m_Gui->Button(ID_CHOOSE_BODY_CHILD_VME_SURFACE, "Child Body");
	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, false);
	m_Gui->Button(ID_CHOOSE_REF_SYS_VME_IN_CHILD, "RefSys in Child");
	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD, false);
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

void lhpOpModifyOpenSimModelCreatePinJoint::OnEvent(mafEventBase *maf_event)
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

		case ID_CHOOSE_REF_SYS_VME_IN_PARENT:
			{
				m_RefSysVMEInParent = ChooseRefSysVME();

				if (m_RefSysVMEInParent != NULL)
				{
					m_Gui->Update();

					m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
					m_Gui->Update();
				}
			}
			break;

		case ID_CHOOSE_REF_SYS_VME_IN_CHILD:
			{
				m_RefSysVMEInChild = ChooseRefSysVME();

				if (m_RefSysVMEInChild != NULL)
				{
					m_Gui->Update();

					m_Gui->Enable(ID_GENERATE_JOINT, true);
					m_Gui->Update();
				}
			}
			break;

		case ID_USE_GROUND_AS_PARENT:
			{
				OnChooseGroundAsParent();
			}
			break;

		case ID_CHOOSE_BODY_PARENT_VME_SURFACE:
			{
				OnChooseParentBodyVMESurface();
			}
			break;

		case ID_CHOOSE_BODY_CHILD_VME_SURFACE:
			{
				OnChooseChildBodyVMESurface();
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

void lhpOpModifyOpenSimModelCreatePinJoint::OnChooseParentBodyVMESurface()
{
	mafString title = mafString("Select parent VME surface:");

	m_ParentBodyVME = ChooseBodyVME(title);

	if (m_ParentBodyVME == NULL)
	{
		return;
	}

	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, true);
	m_Gui->Update();
}

void lhpOpModifyOpenSimModelCreatePinJoint::OnChooseChildBodyVMESurface()
{
	mafString title = mafString("Select child VME surface:");

	m_ChildBodyVME = ChooseBodyVME(title);

	if (m_ChildBodyVME == NULL)
	{
		return;
	}

	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD, true);
	m_Gui->Update();
}

void lhpOpModifyOpenSimModelCreatePinJoint::OnGenerateJoint()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreatePinJoint::WriteNMSBuilderDataToFile()
{
	/** Rotation axes direction cosines */
	double refSysOrientationInParentInRad[3];

	assert(m_RefSysVMEInParent);

	mafMatrix refSysInParentMatrix = m_RefSysVMEInParent->GetMatrixPipe()->GetMatrix();

	double refSysInParentOrientationInDeg[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};

	DiMatrix *refSysInParentDiMatrix = new DiMatrix();

	DiV4d refSysInParentPos;
	DiV4d refSysInParentRot;

	mflMatrixToDi2(refSysInParentMatrix.GetVTKMatrix(), refSysInParentDiMatrix);

	mafTransfDecomposeMatrixEulOrdXYZr(refSysInParentDiMatrix , &refSysInParentRot , &refSysInParentPos );

	refSysOrientationInParentInRad[X] = refSysInParentRot.x;
	refSysOrientationInParentInRad[Y] = refSysInParentRot.y;
	refSysOrientationInParentInRad[Z] = refSysInParentRot.z;

	double refSysPositionInParent[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};

	mafTransform::GetPosition(refSysInParentMatrix, refSysPositionInParent);

	wxString parentSurfaceVMEName;

	if (m_UseGroundAsParent == true)
	{
		parentSurfaceVMEName = "ground";
	}
	else if (m_UseGroundAsParent == false)
	{
		parentSurfaceVMEName = m_ParentBodyVME->GetName();
	}

	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream <<
		"[***ParentSurfaceVMEName_FromNMSB***]" << "," << ReplaceSpaceWithUnderscore(parentSurfaceVMEName.c_str()) << std::endl <<
		"[***ChildSurfaceVMEName_FromNMSB***]"  << "," << ReplaceSpaceWithUnderscore(m_ChildBodyVME->GetName()) << std::endl <<
		"[***ArticularCentreInParent_X_FromNMSB***]" << "," <<  refSysPositionInParent[X] << std::endl <<
		"[***ArticularCentreInParent_Y_FromNMSB***]" << "," <<  refSysPositionInParent[Y] << std::endl <<
		"[***ArticularCentreInParent_Z_FromNMSB***]" << "," <<  refSysPositionInParent[Z] << std::endl <<
		"[***RotationAxisEulerAnglesInRadiansInParent_X_FromNMSB***]" << "," <<  refSysOrientationInParentInRad[X] << std::endl <<
		"[***RotationAxisEulerAnglesInRadiansInParent_Y_FromNMSB***]" << "," <<  refSysOrientationInParentInRad[Y] << std::endl <<
		"[***RotationAxisEulerAnglesInRadiansInParent_Z_FromNMSB***]" << "," <<  refSysOrientationInParentInRad[Z] << std::endl;

	/** Rotation axes direction cosines */
	double refSysOrientationInChildInRad[3];

	assert(m_RefSysVMEInChild);

	mafMatrix refSysInChildMatrix = m_RefSysVMEInChild->GetMatrixPipe()->GetMatrix();

	double refSysInChildOrientationInDeg[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};

	DiMatrix *refSysInChildDiMatrix = new DiMatrix();

	DiV4d refSysInChildPos;
	DiV4d refSysInChildRot;

	mflMatrixToDi2(refSysInChildMatrix.GetVTKMatrix(), refSysInChildDiMatrix);

	mafTransfDecomposeMatrixEulOrdXYZr(refSysInChildDiMatrix , &refSysInChildRot , &refSysInChildPos );

	refSysOrientationInChildInRad[X] = refSysInChildRot.x;
	refSysOrientationInChildInRad[Y] = refSysInChildRot.y;
	refSysOrientationInChildInRad[Z] = refSysInChildRot.z;

	double refSysPositionInChild[3] = {UNDEFINED_DOUBLE , UNDEFINED_DOUBLE , UNDEFINED_DOUBLE};

	mafTransform::GetPosition(refSysInChildMatrix, refSysPositionInChild);

	// write data extracted from NMSBuilder
	stringStream <<
		"[***ArticularCentreInChild_X_FromNMSB***]" << "," <<  refSysPositionInChild[X] << std::endl <<
		"[***ArticularCentreInChild_Y_FromNMSB***]" << "," <<  refSysPositionInChild[Y] << std::endl <<
		"[***ArticularCentreInChild_Z_FromNMSB***]" << "," <<  refSysPositionInChild[Z] << std::endl <<
		"[***RotationAxisEulerAnglesInRadiansInChild_X_FromNMSB***]" << "," <<  refSysOrientationInChildInRad[X] << std::endl <<
		"[***RotationAxisEulerAnglesInRadiansInChild_Y_FromNMSB***]" << "," <<  refSysOrientationInChildInRad[Y] << std::endl <<
		"[***RotationAxisEulerAnglesInRadiansInChild_Z_FromNMSB***]" << "," <<  refSysOrientationInChildInRad[Z] << std::endl;

	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();
}

void lhpOpModifyOpenSimModelCreatePinJoint::InitGUITestStuff()
{
	mafNode *root = m_Input->GetRoot();
	assert(root);

	if (m_UseGroundAsParent == false)
	{
		m_ParentBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("FemurL"));
		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;
		assert(m_ParentBodyVME);
	}
	else
	{
		m_ParentBodyVME = NULL;
		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetweenBodyAndGroundFileName ;
	}

	m_ChildBodyVME = mafVME::SafeDownCast(root->FindInTreeByName("TibiaL"));
	assert(m_ChildBodyVME);

	m_RefSysVMEInParent = mafVMERefSys::SafeDownCast(root->FindInTreeByName("ref_sys"));
	assert(m_RefSysVMEInParent);

	m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);

	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, true);
	m_Gui->Enable(ID_GENERATE_JOINT, true);

	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, true);
	m_Gui->Enable(ID_CHOOSE_ROTATION_AXIS_FROM_VME_REF_SYS, true);

	m_Gui->Update();
}

void lhpOpModifyOpenSimModelCreatePinJoint::OnChooseGroundAsParent()
{
	if (m_UseGroundAsParent == true)
	{
		m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, false);
		m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, true);
		m_Gui->Update();

		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetweenBodyAndGroundFileName ;
	}
	else if (m_UseGroundAsParent == false)
	{
		m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);
		m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, false);
		m_Gui->Update();

		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;
	}
}

void lhpOpModifyOpenSimModelCreatePinJoint::GetOrientationXYZ( const mafMatrix &in_matrix,double orientation[3] )
{
}

void lhpOpModifyOpenSimModelCreatePinJoint::SetParentBodyVME( mafVME *parent )
{
	assert(lhpOpModifyOpenSimModel::VMEAcceptAsBody(parent));
	m_ParentBodyVME = parent;
}

void lhpOpModifyOpenSimModelCreatePinJoint::SetChildBodyVME( mafVME *child )
{
	assert(lhpOpModifyOpenSimModel::VMEAcceptAsBody(child));
	m_ChildBodyVME = child;
}

void lhpOpModifyOpenSimModelCreatePinJoint::SetRefSysVMEInParent( mafVME *refSys )
{
	assert(lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept(refSys));
	m_RefSysVMEInParent = refSys;
}

void lhpOpModifyOpenSimModelCreatePinJoint::SetRefSysVMEInChild( mafVME *refSys )
{
	assert(lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept(refSys));
	m_RefSysVMEInChild = refSys;
}

void lhpOpModifyOpenSimModelCreatePinJoint::SetUseGroundAsParent( int useGroundAsParent )
{
	m_UseGroundAsParent = useGroundAsParent;
}

mafVME *lhpOpModifyOpenSimModelCreatePinJoint::ChooseBodyVME( mafString title )
{
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModel::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());

	return vme;
}