/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateBallJoint.cpp,v $
Language:  C++
Date:      $Date: 2012-03-26 09:08:21 $
Version:   $Revision: 1.1.2.4 $
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

#include "lhpOpModifyOpenSimModelCreateBallJoint.h"

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
#include "mafPlotMath.h"
using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateBallJoint);
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
lhpOpModifyOpenSimModelCreateBallJoint::lhpOpModifyOpenSimModelCreateBallJoint(const wxString &label) :
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

	m_DictionaryToConfigureFileName = "OSIM_CreateBallJoint_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_CreateBallJoint_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_CreateBallJoint_Dictionary.txt";

	// m_ComponentToConfigureJointBetweenBodyAndGroundFileName = "OSIM_CreateBallJointWithGround.cpp.in";
	m_ComponentToConfigureJointBetweenBodyAndGroundFileName = "OSIM_CreateBallJointAsCustomJointWithGround.cpp.in";

	// m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreateBallJoint.cpp.in";
	m_ComponentToConfigureJointBetween2BodiesFileName = "OSIM_CreateBallJointAsCustomJoint.cpp.in";
	
	m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;

	m_ConfiguredComponentFileName = "OSIM_CreateBallJoint.cpp";

	m_UseGroundAsParent = false;

	m_ParentBodyVME = NULL;
	m_ChildBodyVME = NULL;

	m_RefSysVMEInParent = NULL;
	m_RefSysVMEInChild = NULL;
}

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateBallJoint::~lhpOpModifyOpenSimModelCreateBallJoint( ) 
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateBallJoint::Copy()   
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateBallJoint(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateBallJoint::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
 	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateBallJoint::OpRun()   
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
void lhpOpModifyOpenSimModelCreateBallJoint::CreateGui()
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



void lhpOpModifyOpenSimModelCreateBallJoint::OnEvent(mafEventBase *maf_event)
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

void lhpOpModifyOpenSimModelCreateBallJoint::OnChooseParentBodyVMESurface()
{
	mafString title = mafString("Select parent VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateBallJoint::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ParentBodyVME = vme;

	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, true);
	m_Gui->Update();

}

void lhpOpModifyOpenSimModelCreateBallJoint::OnChooseChildBodyVMESurface()
{
	mafString title = mafString("Select child VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateBallJoint::VMEAcceptAsBody);
	mafEventMacro(e);
	mafVME *vme = mafVME::SafeDownCast(e.GetVme());	

	if (vme == NULL)
	{
		return;
	}

	m_ChildBodyVME = vme;

	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD, true);
	m_Gui->Update();

}
bool lhpOpModifyOpenSimModelCreateBallJoint::IsVMESurfaceAndCanBeUsedAsOpenSimBody(mafNode *node) 
{  	

	bool hasInertialTensorComponents = node->GetTagArray()->IsTagPresent("PRINCIPAL_INERTIAL_TENSOR_COMPONENTS");
	bool hasMass = node->GetTagArray()->IsTagPresent("SURFACE_MASS");
	bool isVmeSurface = mafVMESurface::SafeDownCast(node) != NULL;
	return hasInertialTensorComponents && hasMass && isVmeSurface;

}

void lhpOpModifyOpenSimModelCreateBallJoint::OnGenerateJoint()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateBallJoint::WriteNMSBuilderDataToFile()
{
	/** Rotation axes direction cosines */
	double refSysOrientationInParentInRad[3];

	mafMatrix refSysInParentMatrix;
	wxString parentSurfaceVMEName;

	if (m_UseGroundAsParent == false)
	{
		parentSurfaceVMEName = m_ParentBodyVME->GetName();
	}
	else 
	{
		parentSurfaceVMEName = "ground";
	}

	refSysInParentMatrix = m_RefSysVMEInParent->GetMatrixPipe()->GetMatrix();

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

void lhpOpModifyOpenSimModelCreateBallJoint::InitGUITestStuff()
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

	m_ChildBodyVME = mafVMESurface::SafeDownCast(root->FindInTreeByName("TibiaL"));
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

void lhpOpModifyOpenSimModelCreateBallJoint::OnChooseGroundAsParent()
{
	if (m_UseGroundAsParent == true)
	{
		EnableGui(false);
		m_Gui->Enable(ID_USE_GROUND_AS_PARENT, true);
		m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, false);
		m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, true);
		m_Gui->Update();

		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetweenBodyAndGroundFileName ;
	}
	else if (m_UseGroundAsParent == false)
	{
		EnableGui(false);
		m_Gui->Enable(ID_USE_GROUND_AS_PARENT, true);
		m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, true);
		m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, false);
		m_Gui->Update();

		m_ComponentToConfigureFileName = m_ComponentToConfigureJointBetween2BodiesFileName;
	}
}

bool lhpOpModifyOpenSimModelCreateBallJoint::VMERefSysOrVMESurfaceAccept(mafNode *node) 
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

void lhpOpModifyOpenSimModelCreateBallJoint::SetParentBodyVME( mafVME *parent )
{
	assert(lhpOpModifyOpenSimModel::VMEAcceptAsBody(parent));
	m_ParentBodyVME = parent;
}

void lhpOpModifyOpenSimModelCreateBallJoint::SetChildBodyVME( mafVME *child )
{
	assert(lhpOpModifyOpenSimModel::VMEAcceptAsBody(child));
	m_ChildBodyVME = child;
}

void lhpOpModifyOpenSimModelCreateBallJoint::SetRefSysVMEInParent( mafVME *refSys )
{
	assert(lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept(refSys));
	m_RefSysVMEInParent = refSys;
}

void lhpOpModifyOpenSimModelCreateBallJoint::SetRefSysVMEInChild( mafVME *refSys )
{
	assert(lhpOpModifyOpenSimModel::VMERefSysOrVMESurfaceAccept(refSys));
	m_RefSysVMEInChild = refSys;
}

void lhpOpModifyOpenSimModelCreateBallJoint::SetUseGroundAsParent( int useGroundAsParent )
{
	m_UseGroundAsParent = useGroundAsParent;
}

void lhpOpModifyOpenSimModelCreateBallJoint::EnableGui(bool enable)
{
	m_Gui->Enable(ID_USE_GROUND_AS_PARENT, enable);
	m_Gui->Enable(ID_CHOOSE_BODY_PARENT_VME_SURFACE, enable);
	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_PARENT, enable);
	m_Gui->Enable(ID_CHOOSE_BODY_CHILD_VME_SURFACE, enable);
	m_Gui->Enable(ID_CHOOSE_REF_SYS_VME_IN_CHILD, enable);
	m_Gui->Enable(ID_GENERATE_JOINT, enable);
	m_Gui->Update();
}
