/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpOpModifyOpenSimModelCreateBodyFromSurface.cpp,v $
Language:  C++
Date:      $Date: 2012-04-11 16:40:13 $
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

#include "lhpOpModifyOpenSimModelCreateBodyFromSurface.h"

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
#include "mafTagArray.h"
#include "vtkMath.h"
#include "mafTransform.h"

using namespace xercesc;
using namespace std;

#define UNDEFINED_STRING "UNDEFINED"
#define UNDEFINED_DOUBLE -999999999

//----------------------------------------------------------------------------
mafCxxTypeMacro(lhpOpModifyOpenSimModelCreateBodyFromSurface);
//----------------------------------------------------------------------------

enum lhpOpModifyOpenSimModelCreateBody_ID
{
	ID_CHOOSE_BODY_VME = lhpOpModifyOpenSimModel::ID_LAST,
	ID_GENERATE_BODY,
};

//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateBodyFromSurface::lhpOpModifyOpenSimModelCreateBodyFromSurface(const wxString &label) :
lhpOpModifyOpenSimModel(label)
//----------------------------------------------------------------------------
{
	m_OpType	= OPTYPE_OP;
	m_Canundo = false;
	m_Input		= NULL; 

	m_Surface = NULL;
	m_SurfaceVtkAbsFileName = UNDEFINED_STRING;

	m_DictionaryToConfigureFileName = "OSIM_AddBlockToModel_Dictionary.txt.in";
	m_DataFromNMSBuilderFileName = "OSIM_AddBlockToModel_DataFromNMSBuilder.txt";
	m_ConfiguredDictionaryFileName = "OSIM_AddBlockToModel_Dictionary.txt";

	m_ComponentToConfigureFileName = "OSIM_AddBlockToModel.cpp.in";
	m_ConfiguredComponentFileName = "OSIM_AddBlockToModel.cpp";

	#define UNDEFINED_STRING "UNDEFINED"

	m_Ixx = UNDEFINED_DOUBLE;
	m_Iyy = UNDEFINED_DOUBLE;
	m_Izz = UNDEFINED_DOUBLE;
	m_Ixy = UNDEFINED_DOUBLE;
	m_Ixz = UNDEFINED_DOUBLE;
	m_Iyz = UNDEFINED_DOUBLE;
	
	m_Mass = UNDEFINED_DOUBLE;
	
	m_CenterOfMass[0] = UNDEFINED_DOUBLE;
	m_CenterOfMass[1] = UNDEFINED_DOUBLE;
	m_CenterOfMass[2] = UNDEFINED_DOUBLE;


}
//----------------------------------------------------------------------------
lhpOpModifyOpenSimModelCreateBodyFromSurface::~lhpOpModifyOpenSimModelCreateBodyFromSurface( ) 
//----------------------------------------------------------------------------
{

}
//----------------------------------------------------------------------------
mafOp* lhpOpModifyOpenSimModelCreateBodyFromSurface::Copy()   
//----------------------------------------------------------------------------
{
	return (new lhpOpModifyOpenSimModelCreateBodyFromSurface(m_Label));
}

//----------------------------------------------------------------------------
bool lhpOpModifyOpenSimModelCreateBodyFromSurface::Accept(mafNode *node)
//----------------------------------------------------------------------------
{
 	return lhpOpModifyOpenSimModel::Accept(node);
}
//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateBodyFromSurface::OpRun()   
//----------------------------------------------------------------------------
{	

	CreateGui();
	ShowGui();
}

//----------------------------------------------------------------------------
void lhpOpModifyOpenSimModelCreateBodyFromSurface::CreateGui()
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
	m_Gui->Button(ID_CHOOSE_BODY_VME, "Choose body VME");
	m_Gui->Label("");
	m_Gui->Button(ID_GENERATE_BODY, "Generate OpenSim Body");
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

	m_Gui->Enable(ID_GENERATE_BODY, false);

	m_Gui->OkCancel();
}



void lhpOpModifyOpenSimModelCreateBodyFromSurface::OnEvent(mafEventBase *maf_event)
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
	
		case ID_CHOOSE_BODY_VME:
			{
				OnChooseBodyVME();
			}
			break;

		case ID_GENERATE_BODY:
			{
				OnGenerateBody();
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

void lhpOpModifyOpenSimModelCreateBodyFromSurface::OnChooseBodyVME()
{
	mafString title = mafString("Select a VME surface:");
	mafEvent e(this,VME_CHOOSE);
	e.SetString(&title);
	e.SetArg((long)&lhpOpModifyOpenSimModelCreateBodyFromSurface::VMESurfaceAcceptForBodyGeneration);
	mafEventMacro(e);
	mafVME *vme = (mafVME *)e.GetVme();	
	
	if (vme == NULL)
	{
		return;
	}

	m_Gui->Enable(ID_GENERATE_BODY, true);

	OnSetInputVME(vme);
}

void lhpOpModifyOpenSimModelCreateBodyFromSurface::OnGenerateBody()
{
	GenerateOpenSimComponentAPIAndOpenItInNotepadPP();
}

void lhpOpModifyOpenSimModelCreateBodyFromSurface::WriteNMSBuilderDataToFile()
{
	assert(m_SurfaceVtkAbsFileName != UNDEFINED_STRING);
	
	// write data extracted from NMSBuilder
	ostringstream stringStream;
	stringStream << 
		"[***VTPAbsFileName_FromNMSB***]" << "," << m_SurfaceVTPFileName.c_str() << std::endl <<
		"[***blockName_FromNMSB***]"  << "," << m_Surface->GetName() << std::endl <<
		"[***blockNameWithUnderscoreInsteadOfSpaces_FromNMSB***]"  << "," << ReplaceSpaceWithUnderscore(m_Surface->GetName()).c_str() << std::endl <<
	    "[***blockMass_FromNMSB***]" <<"," <<  m_Mass << std::endl <<
		"[***blockInertia_XX_FromNMSB***]" <<"," <<  m_Ixx << std::endl <<
		"[***blockInertia_YY_FromNMSB***]" << "," <<  m_Iyy << std::endl <<
		"[***blockInertia_ZZ_FromNMSB***]" <<"," <<  m_Izz << std::endl <<
		"[***blockInertia_XY_FromNMSB***]" <<"," <<  m_Ixy << std::endl <<
		"[***blockInertia_XZ_FromNMSB***]" << "," <<  m_Ixz << std::endl <<
		"[***blockInertia_YZ_FromNMSB***]" <<"," <<  m_Iyz << std::endl <<
		"[***blockMassCenter_X_FromNMSB***]" << "," <<  m_CenterOfMass[0] << std::endl <<
		"[***blockMassCenter_Y_FromNMSB***]" << "," <<  m_CenterOfMass[1] << std::endl <<
		"[***blockMassCenter_Z_FromNMSB***]" << "," <<  m_CenterOfMass[2] << std::endl;
	    
	ofstream myFile;
	myFile.open(GetOpenSimAPITemplatesDir() + m_DataFromNMSBuilderFileName.c_str());
	myFile << stringStream.str().c_str();
	myFile.close();
}

void lhpOpModifyOpenSimModelCreateBodyFromSurface::ComputeCenterOfMass(vtkPoints* points, double center[3])
{
	assert(points);
	vtkIdType n = points->GetNumberOfPoints();
	// Initialize the center to zero
	center[0] = 0.0;
	center[1] = 0.0;
	center[2] = 0.0;

	assert("pre: no points" && n > 0);

	// No weights
	for(vtkIdType i = 0; i < n; i++)
	{
		double point[3];
		points->GetPoint(i, point);

		mafTransform::AddVectors(center, point, center);
	}

	mafTransform::MultiplyVectorByScalar(1.0/n , center , center);
	
}

void lhpOpModifyOpenSimModelCreateBodyFromSurface::OnSetInputVME( mafVME * vme )
{
	assert(VMESurfaceAcceptForBodyGeneration(vme));

 /* Inertial tensor
  | Ixx  Iyx  Izx |
  | Iyx  Iyy  Izy | 
  | Izx  Izy  Izz | */
  // [Ixx  Iyx  Izx ,  Iyx  Iyy  Izy , Izx  Izy  Izz]

	// get inertial attributes
	m_Ixx = vme->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(0);
	m_Iyy = vme->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(4);
	m_Izz = vme->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(8);
	m_Ixy = vme->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(1);
	m_Ixz = vme->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(2);
	m_Iyz = vme->GetTagArray()->GetTag("INERTIAL_TENSOR_COMPONENTS")->GetComponentAsDouble(5);

	m_Mass = vme->GetTagArray()->GetTag("SURFACE_MASS")->GetValueAsDouble();

	assert(vme);
	m_Surface = mafVMESurface::SafeDownCast(vme);

	m_Surface->GetOutput()->GetVTKData()->Modified();
	m_Surface->GetOutput()->GetVTKData()->Update();

	m_CenterOfMass[0] = m_Surface->GetTagArray()->GetTag("LOCAL_CENTER_OF_MASS_COMPONENTS")->GetComponentAsDouble(0);
	m_CenterOfMass[1] = m_Surface->GetTagArray()->GetTag("LOCAL_CENTER_OF_MASS_COMPONENTS")->GetComponentAsDouble(1);
	m_CenterOfMass[2] = m_Surface->GetTagArray()->GetTag("LOCAL_CENTER_OF_MASS_COMPONENTS")->GetComponentAsDouble(2);

	// get center of mass for current geometry
	// ComputeCenterOfMass(m_Surface->GetSurfaceOutput()->GetSurfaceData()->GetPoints(), m_CenterOfMass);

	m_SurfaceVtkAbsFileName = GetVTKFileNameAbsPath(m_Surface);
	assert(wxFileExists(m_SurfaceVtkAbsFileName));

	m_SurfaceVTPFileName = m_Surface->GetName() + wxString(".vtp");

	wxString outputFileName = GetOutputModelDirectory() + m_SurfaceVTPFileName;

	medOpExporterVTKXML *exporter=new medOpExporterVTKXML();
	exporter->SetInput(vme);
	exporter->TestModeOn();
	exporter->SetFileName(outputFileName.c_str());
	exporter->ExportAsBynaryOn();
	exporter->ExportVTK();

	assert(wxFileExists(outputFileName));

	vtkDEL(exporter);
}

void lhpOpModifyOpenSimModelCreateBodyFromSurface::SetInputVMEForComponentAPIGeneration(mafNode *vme)
{
	assert(mafVME::SafeDownCast(vme));

	OnSetInputVME(mafVME::SafeDownCast(vme));
}


